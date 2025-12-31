# WorldRenderer.gd — ULTRA-OPTIMIZED (Multiple Spritesheets)
extends Node2D

@export var world: World
@export var chunk_renderer_scene: PackedScene
@export var entity_sprite_scene: PackedScene
@export var max_render_distance_chunks: int = 1
@export var render_buffer_chunks: int = 1
@export var simulation_distance: int = 60

@onready var entity_container: Node2D = $EntityContainer
@onready var MainNode = get_parent()

var chunks: Dictionary = {}
var sprite_pool: Array[Sprite2D] = []  # Renamed: this is our pool of Sprite2D objects
var cam_pos: Vector2
const ENTITY_POOL_SIZE: int = 20000
var cs: float

func _ready() -> void:
	cs = GameSettings.chunk_size

	# Pre-allocate sprite pool
	sprite_pool.resize(ENTITY_POOL_SIZE)
	for i in ENTITY_POOL_SIZE:
		var s: Sprite2D = entity_sprite_scene.instantiate() as Sprite2D
		s.visible = false
		s.region_enabled = true
		s.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
		
		# Set safe defaults
		s.texture = SpriteAtlas.default_texture
		s.region_rect = SpriteAtlas.default_region
		s.scale = SpriteAtlas.default_scale
		s.offset = SpriteAtlas.default_offset
		
		entity_container.add_child(s)
		sprite_pool[i] = s


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

	# === 2. Get visible chunks ===
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

	for c in chunks.keys():
		if not needed_chunks.has(c):
			chunks[c].queue_free()
			chunks.erase(c)

	# === 4. Get visible entities from C++ ===
	var visible_data: Dictionary = world.get_visible_entities(
		visible_chunks,
		world_min,
		world_max,
		ENTITY_POOL_SIZE
	)

	var positions: PackedVector2Array = visible_data["positions"]
	var sheet_ids: PackedInt32Array = visible_data["types"]                    # Spritesheet ID
	var variant_ids: PackedInt32Array = visible_data["entity_sprites"]        # Sprite variant on that sheet
	var entity_count: int = visible_data["count"]
	var entity_width: PackedInt32Array = visible_data["entity_width"]
	var entity_height: PackedInt32Array = visible_data["entity_height"]

	# === 5. Update sprite pool ===
	for i in entity_count:
		
		var width: int = entity_width[i]                     # e.g., 3 for a 3-wide building
		var height: int = entity_height[i]
		var sprite: Sprite2D = sprite_pool[i]
		sprite.global_position = positions[i]

		var sheet_id: int = sheet_ids[i]
		var variant_id: int = variant_ids[i]

		# Texture — only update if changed
		var new_texture: Texture2D = SpriteAtlas.get_texture(sheet_id)
		if sprite.texture != new_texture:
			sprite.texture = new_texture

		# Region
		var new_region: Rect2 = SpriteAtlas.get_region(sheet_id, variant_id)
		if sprite.region_rect != new_region:
			sprite.region_rect = new_region

		# Scale
		var new_scale: Vector2 = SpriteAtlas.get_scale(sheet_id)
		if sprite.scale != new_scale:
			sprite.scale = new_scale

		# Offset
		var new_offset: Vector2 = SpriteAtlas.get_offset(sheet_id, variant_id)
		if sprite.offset != new_offset:
			sprite.offset = new_offset
			
		sprite.scale.x *= width
		sprite.scale.y *= height

		sprite.visible = true

	# Hide excess sprites
	for i in range(entity_count, ENTITY_POOL_SIZE):
		sprite_pool[i].visible = false

	# === DEBUG: Per-chunk counts ===
	var visible_per_chunk: Dictionary = {}
	for i in entity_count:
		var world_pos: Vector2 = positions[i]
		var chunk: Vector2i = (world_pos / cs).floor()
		visible_per_chunk[chunk] = visible_per_chunk.get(chunk, 0) + 1

	for chunk_coord in visible_chunks:
		var renderer: ChunkRenderer = chunks[chunk_coord]
		var visible_here: int = visible_per_chunk.get(chunk_coord, 0)
		var total_here: int = world.get_loaded_entity_count_in_chunk(chunk_coord) if world.has_method("get_loaded_entity_count_in_chunk") \
							  else visible_here
		renderer.set_debug_text(total_here, visible_here)


func _physics_process(delta: float) -> void:
	world.update(cam_pos, simulation_distance, delta)


func _unhandled_input(event):
	if event is InputEventKey and event.pressed and event.keycode == KEY_F3:
		for r in chunks.values():
			if r.debug_label:
				r.debug_label.visible = !r.debug_label.visible
