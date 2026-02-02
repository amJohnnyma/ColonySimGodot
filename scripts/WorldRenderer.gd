extends Node2D

@export var world: World
@export var chunk_renderer_scene: PackedScene
@export var entity_sprite_scene: PackedScene

var max_render_distance_chunks: int = GameSettings.max_render_distance_chunks
var render_buffer_chunks: int = GameSettings.render_buffer_chunks
var simulation_distance: int = GameSettings.simulation_distance

@onready var entity_container: Node2D = $EntityContainer

var chunks: Dictionary = {}  # Vector2i → ChunkRenderer
var sprite_pool: Array[Sprite2D] = []
var cam_pos: Vector2

const ENTITY_POOL_SIZE: int = 20000

var cs: float

# Slowdown system
var slowdown_factor: float = 1.0
var fps_history: Array[float] = []
var simulation_task_id: int = -1          # -1 = no task running
var last_sim_delta_accum: float = 0.0     # accumulate real delta when sim is busy
@onready var debug_label: Label = get_tree().root.get_node("Main_Game/UI/MasterControl/GameUI/DebugPanel/Label")
var sim_ticks_this_second: int = 0
var last_second_time: float = 0.0
var displayed_fps: float = 0.0
var displayed_sim_rate: float = 0.0

# Per-sprite cache to avoid redundant atlas lookups
var sprite_last_sheet: PackedInt32Array
var sprite_last_variant: PackedInt32Array

func _ready() -> void:
	cs = GameSettings.chunk_size

	sprite_pool.resize(ENTITY_POOL_SIZE)
	sprite_last_sheet.resize(ENTITY_POOL_SIZE)
	sprite_last_variant.resize(ENTITY_POOL_SIZE)

	for i in ENTITY_POOL_SIZE:
		var s: Sprite2D = entity_sprite_scene.instantiate() as Sprite2D
		s.visible = false
		s.region_enabled = true
		s.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST

		# Safe defaults + disable expensive/unused features
		s.texture = SpriteAtlas.default_texture
		s.region_rect = SpriteAtlas.default_region
		s.scale = SpriteAtlas.default_scale
		s.offset = SpriteAtlas.default_offset

		s.light_mask = 0
		s.modulate = Color.WHITE
		s.self_modulate = Color.WHITE
		s.use_parent_material = true
		s.z_index = 0
		s.top_level = true
		s.clip_children = CanvasItem.CLIP_CHILDREN_DISABLED

		entity_container.add_child(s)
		sprite_pool[i] = s

		# Initialize cache
		sprite_last_sheet[i] = -1
		sprite_last_variant[i] = -1
		
		
	if not debug_label:
		push_warning("DebugLabel not found! FPS counter disabled.")
	else:
		debug_label.text = "FPS: -- | Sim: -- Hz"


func _process(delta: float) -> void:
	if not world or not chunk_renderer_scene:
		return
	var cam := get_viewport().get_camera_2d() as Camera2D
	if not cam:
		return

	var viewport_rect: Rect2 = get_viewport_rect()
	cam_pos = cam.global_position
	var cam_zoom: Vector2 = cam.zoom

	# 1. Calculate visible area (zoom-aware) + fixed world buffer
	var viewport_half := viewport_rect.size * 0.5
	var screen_top_left := cam_pos - viewport_half / cam_zoom
	var screen_bottom_right := cam_pos + viewport_half / cam_zoom

	const MIN_WORLD_BUFFER_TILES: float = 512.0
	var min_buffer := MIN_WORLD_BUFFER_TILES

	var world_min := Vector2(
		min(screen_top_left.x, cam_pos.x - min_buffer),
		min(screen_top_left.y, cam_pos.y - min_buffer)
	)
	var world_max := Vector2(
		max(screen_bottom_right.x, cam_pos.x + min_buffer),
		max(screen_bottom_right.y, cam_pos.y + min_buffer)
	)

	var buffer_world := render_buffer_chunks * cs
	world_min -= Vector2(buffer_world, buffer_world)
	world_max += Vector2(buffer_world, buffer_world)

	# 2. Get visible chunks
	var visible_chunks: Array[Vector2i] = world.get_visible_chunks(
		cam_pos, world_min, world_max, max_render_distance_chunks
	)

	# 3. Manage chunk renderers
	var needed_chunks: Dictionary = {}
	for c in visible_chunks:
		needed_chunks[c] = true
		if not chunks.has(c):
			var renderer: ChunkRenderer = chunk_renderer_scene.instantiate() as ChunkRenderer
			add_child(renderer)
			renderer.setup(world, c)
			chunks[c] = renderer

	for c in chunks.keys():
		if not needed_chunks.has(c):
			chunks[c].queue_free()
			chunks.erase(c)

	# 4. Get visible entities from C++
	var visible_data: Dictionary = world.get_visible_entities(
		visible_chunks, world_min, world_max, ENTITY_POOL_SIZE
	)

	var positions: PackedVector2Array = visible_data["positions"]
	var sheet_ids: PackedInt32Array   = visible_data["types"]
	var variant_ids: PackedInt32Array = visible_data["entity_sprites"]
	var entity_count: int = visible_data["count"]

	# 5. Update sprite pool — with aggressive change detection
	for i in entity_count:
		var sprite := sprite_pool[i]

		# Position — most frequent change
		var new_pos := positions[i]
		if sprite.global_position != new_pos:
			sprite.global_position = new_pos

		var sheet_id := sheet_ids[i]
		var variant_id := variant_ids[i]

		# Skip atlas lookups if sheet + variant unchanged
		if sheet_id == sprite_last_sheet[i] and variant_id == sprite_last_variant[i]:
			# Still need to make sure it's visible
			if not sprite.visible:
				sprite.visible = true
			continue

		# Cache miss → update properties
		sprite_last_sheet[i] = sheet_id
		sprite_last_variant[i] = variant_id

		# Texture
		var new_texture := SpriteAtlas.get_texture(sheet_id)
		if sprite.texture != new_texture:
			sprite.texture = new_texture

		# Region
		var new_region := SpriteAtlas.get_region(sheet_id, variant_id)
		if sprite.region_rect != new_region:
			sprite.region_rect = new_region

		# Scale (per sheet)
		var new_scale := SpriteAtlas.get_scale(sheet_id)
		if sprite.scale != new_scale:
			sprite.scale = new_scale

		# Offset (per variant)
		var new_offset := SpriteAtlas.get_offset(sheet_id, variant_id)
		if sprite.offset != new_offset:
			sprite.offset = new_offset

		# Ensure visible
		if not sprite.visible:
			sprite.visible = true

	# Hide unused sprites
	for i in range(entity_count, ENTITY_POOL_SIZE):
		var s := sprite_pool[i]
		if s.visible:
			s.visible = false
			# Optional: reset cache when hiding (helps if pool re-uses change often)
			sprite_last_sheet[i] = -1
			sprite_last_variant[i] = -1

	# === DEBUG ===
	var visible_per_chunk: Dictionary = {}
	for i in entity_count:
		var chunk := (positions[i] / cs).floor()
		visible_per_chunk[chunk] = visible_per_chunk.get(chunk, 0) + 1

	for chunk_coord in visible_chunks:
		var renderer: ChunkRenderer = chunks.get(chunk_coord)
		if not renderer: continue
		var visible_here :int= visible_per_chunk.get(chunk_coord, 0)
		var total_here :int= world.get_loaded_entity_count_in_chunk(chunk_coord) \
			if world.has_method("get_loaded_entity_count_in_chunk") else visible_here
		renderer.set_debug_text(total_here, visible_here)


func _update_slowdown() -> void:
	fps_history.append(Engine.get_frames_per_second())
	if fps_history.size() > GameSettings.HISTORY_FRAMES:
		fps_history.pop_front()

	var avg_fps := 0.0
	for fps in fps_history:
		avg_fps += fps
	avg_fps /= fps_history.size()

	slowdown_factor = clamp(avg_fps / float(GameSettings.TARGET_FPS),
							GameSettings.MIN_SLOWDOWN, 1.0)


func _get_slowed_delta(delta: float) -> float:
	return delta * slowdown_factor


func _physics_process(delta: float) -> void:
	_update_slowdown()

	# Accumulate time (fixed timestep style, but non-blocking)
	last_sim_delta_accum += _get_slowed_delta(delta)

	# Only start a new sim step if:
	# - no sim is currently running
	# - we have enough accumulated time for at least one tick
	if simulation_task_id == -1 and last_sim_delta_accum >= GameSettings.sim_rate_target:
		# Cap the delta we send (prevents huge simulation jumps after long stalls)
		var sim_delta_to_send = min(last_sim_delta_accum, GameSettings.max_sim_delta_per_tick)

		# Launch background task
		simulation_task_id = WorkerThreadPool.add_task(
			func():
				world.update(cam_pos, max_render_distance_chunks, sim_delta_to_send, GameSettings.paused),
			true  # high_priority = true → gets picked up faster
		)

		# Consume the time we just scheduled
		last_sim_delta_accum -= sim_delta_to_send

	# Optional: if sim is taking too long and accum grows large, you can skip frames or clamp
	if last_sim_delta_accum > 0.5:  # e.g. >0.5s backlog → drop excess to avoid spiral
		last_sim_delta_accum = 0.5

	# Optional: check if previous task finished (non-blocking poll)
	if simulation_task_id != -1:
		if WorkerThreadPool.is_task_completed(simulation_task_id):
			WorkerThreadPool.wait_for_task_completion(simulation_task_id)  # cleans up
			simulation_task_id = -1

	# If you want to know when a sim tick finished (e.g. for debug/UI):
	# if simulation_task_id == -1 and previous was running → emit signal "sim_tick_completed"
	# Update FPS & sim rate display
	var current_time = Time.get_ticks_msec() / 1000.0

	# FPS (smoothed a bit for readability)
	displayed_fps = lerp(displayed_fps, Engine.get_frames_per_second(), 0.1)

	# Simulation tick rate (how many world.update calls per second)
	if current_time - last_second_time >= 1.0:
		displayed_sim_rate = sim_ticks_this_second
		sim_ticks_this_second = 0
		last_second_time = current_time

	# Only increment when a simulation tick actually ran
	# (put this right after you launch or complete a WorkerThreadPool task)
	# Example — in the if block where you add_task:
	if simulation_task_id == -1 and last_sim_delta_accum >= GameSettings.sim_rate_target:
		# ... launch task ...
		sim_ticks_this_second += 1   # ← count this as a scheduled tick

	# If using blocking version (fallback):
	# world.update(...)  → put sim_ticks_this_second += 1 right after the call

	# Finally, update the label text
	if debug_label:
		debug_label.text = "FPS: %d | VisibleSim: %.1f Hz | Backlog: %.2fs" % [
			int(displayed_fps),
			displayed_sim_rate,
			last_sim_delta_accum
		]

func _unhandled_input(event: InputEvent) -> void:
	if event is InputEventKey and event.pressed and event.keycode == KEY_F3:
		for r in chunks.values():
			if r.debug_label:
				r.debug_label.visible = !r.debug_label.visible
		if debug_label:
			debug_label.visible = !debug_label.visible
