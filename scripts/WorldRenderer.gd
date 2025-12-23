# WorldRenderer.gd — ULTRA-OPTIMIZED (BATCH API)
extends Node2D

@export var world: World
@export var chunk_renderer_scene: PackedScene
@export var entity_sprite_scene: PackedScene



@export var max_render_distance_chunks: int = 1
@export var render_buffer_chunks: int = 1
@export var simulation_distance: int = 8

@onready var entity_container: Node2D = $EntityContainer
@onready var MainNode = get_parent()
#@onready var MainNode: Node2D = get_node("res://scenes/Main.tscn")

var chunks: Dictionary = {}
var entity_sprites: Array[Sprite2D] = []
var cam_pos : Vector2
const ENTITY_POOL_SIZE: int = 5000 # drawn at once
var cs : float

func _ready() -> void:
	cs = GameSettings.chunk_size
	# Pre-allocate entity sprite pool
	entity_sprites.resize(ENTITY_POOL_SIZE)
	for i in ENTITY_POOL_SIZE:
		var s: Sprite2D = entity_sprite_scene.instantiate() as Sprite2D
		s.visible = false
		entity_container.add_child(s)
		entity_sprites[i] = s
		
	for sprite in entity_sprites:
		sprite.texture = SpriteAtlas.entity_atlas
		sprite.region_enabled = true
		sprite.region_rect = SpriteAtlas.default_region  # or hide somehow
		sprite.offset = SpriteAtlas.default_offset

		sprite.scale = SpriteAtlas.default_scale  
		sprite.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST

func _process(delta: float) -> void:
	if not world or not chunk_renderer_scene:
		return
	
	var cam := get_viewport().get_camera_2d() as Camera2D
	if not cam:
		return
	
	var viewport_rect: Rect2 = get_viewport_rect()
	cam_pos = cam.global_position
	var cam_zoom: Vector2 = cam.zoom
	
	# === 1. Calculate world-space bounds ===
	var viewport_half: Vector2 = viewport_rect.size * 0.5
	var screen_top_left: Vector2 = cam_pos - viewport_half / cam_zoom
	var screen_bottom_right: Vector2 = cam_pos + viewport_half / cam_zoom
	
	var buffer_world: float = render_buffer_chunks * cs
	
	var world_min: Vector2 = screen_top_left - Vector2(buffer_world, buffer_world)
	var world_max: Vector2 = screen_bottom_right + Vector2(buffer_world, buffer_world)
	
	# === 2. Get visible chunks from C++ (clamped + culled in one call) ===
	var visible_chunks: Array[Vector2i] = world.get_visible_chunks(
		cam_pos, 
		world_min, 
		world_max, 
		max_render_distance_chunks
	)
	
	# === 3. Manage chunk renderers ===
	var needed_chunks: Dictionary = {}
	for c in visible_chunks:
		needed_chunks[c] = true
		if not chunks.has(c):
			var renderer: ChunkRenderer = chunk_renderer_scene.instantiate() as ChunkRenderer
			add_child(renderer)
			renderer.setup(world, c)
			chunks[c] = renderer
	
	# Unload far chunks
	for c in chunks.keys():
		if not needed_chunks.has(c):
			chunks[c].queue_free()
			chunks.erase(c)
	
	# === 4. Get all visible entity positions from C++ (batch operation) ===
	var visible_data: Dictionary = world.get_visible_entities(
		visible_chunks,
		world_min,
		world_max,
		ENTITY_POOL_SIZE
	)

	# Extract the actual data
	var positions: PackedVector2Array = visible_data["positions"]
	var entity_count: int = visible_data["count"]
	var entity_type: PackedInt32Array = visible_data["types"]

	# === 5. Update sprite pool (simple array assignment) ===
	for i in entity_count:
		var sprite: Sprite2D = entity_sprites[i]
		sprite.global_position = positions[i]
		

		var type: int = entity_type[i]
		var sprite_region: Rect2 = SpriteAtlas.type_to_region.get(type, SpriteAtlas.default_region)
		var sprite_scale: Vector2 = SpriteAtlas.type_to_scale.get(type, SpriteAtlas.default_scale)  
		var sprite_offset_vec: Vector2 = SpriteAtlas.type_to_offset.get(type, SpriteAtlas.default_offset)  
		# Only update texture/region if it changed (optional micro-optimization)
		if sprite.region_rect != sprite_region:
			sprite.region_rect = sprite_region
		if sprite.scale != sprite_scale:
			sprite.scale = sprite_scale
		if sprite.offset != sprite_offset_vec: 
			sprite.offset = sprite_offset_vec

		sprite.visible = true

	# Hide unused sprites
	for i in range(entity_count, ENTITY_POOL_SIZE):
		entity_sprites[i].visible = false

	# === TEMPORARY DEBUG: Count visible entities per chunk (GDScript only) ===
	var visible_per_chunk: Dictionary = {} # Vector2i -> count
	for i in entity_count:
		var world_pos: Vector2 = positions[i]
		var chunk: Vector2i = (world_pos / cs).floor()
		visible_per_chunk[chunk] = visible_per_chunk.get(chunk, 0) + 1

	# === Update all chunk debug labels ===
	for chunk_coord in visible_chunks:
		var renderer: ChunkRenderer = chunks[chunk_coord]
		var visible_here : int = visible_per_chunk.get(chunk_coord, 0)
		
		# You probably already have a way to get total entities per chunk?
		# If not, use this is the fastest temporary hack:
		var total_here : int = world.get_loaded_entity_count_in_chunk(chunk_coord) if world.has_method("get_loaded_entity_count_in_chunk") \
						else visible_here  # fallback: assume all loaded are visible (good enough for debug)

		renderer.set_debug_text(total_here, visible_here)

	
	# === 7. Debug (every second) ===
	'''
	if Engine.get_frames_drawn() % 60 == 0:
		print("Chunks: %d | Entities: %d | FPS: %.1f" % [
			visible_chunks.size(),
			entity_count,
			Performance.get_monitor(Performance.TIME_FPS)
		]) '''

func _physics_process(delta: float) -> void:
		# === 6. Background simulation (C++ handles this) ===
	world.update(cam_pos, simulation_distance, delta) # split into update render # update physics
	
func _unhandled_input(event):
	if event is InputEventKey and event.pressed and event.keycode == KEY_F3:
		for r in chunks.values():
			if r.debug_label:
				r.debug_label.visible = !r.debug_label.visible
