# WorldRenderer.gd — ULTRA-OPTIMIZED (BATCH API)
extends Node2D

@export var world: World
@export var chunk_renderer_scene: PackedScene
@export var entity_sprite_scene: PackedScene

@export var max_render_distance_chunks: int = 1
@export var render_buffer_chunks: int = 1
@export var simulation_distance: int = 2

@onready var entity_container: Node2D = $EntityContainer

var chunks: Dictionary = {}
var entity_sprites: Array[Sprite2D] = []
const ENTITY_POOL_SIZE: int = 5000 # drawn at once

func _ready() -> void:
	# Pre-allocate entity sprite pool
	entity_sprites.resize(ENTITY_POOL_SIZE)
	for i in ENTITY_POOL_SIZE:
		var s: Sprite2D = entity_sprite_scene.instantiate() as Sprite2D
		s.visible = false
		entity_container.add_child(s)
		entity_sprites[i] = s

func _process(delta: float) -> void:
	if not world or not chunk_renderer_scene:
		return
	
	var cam := get_viewport().get_camera_2d() as Camera2D
	if not cam:
		return
	
	var viewport_rect: Rect2 = get_viewport_rect()
	var cam_pos: Vector2 = cam.global_position
	var cam_zoom: Vector2 = cam.zoom
	
	# === 1. Calculate world-space bounds ===
	var viewport_half: Vector2 = viewport_rect.size * 0.5
	var screen_top_left: Vector2 = cam_pos - viewport_half / cam_zoom
	var screen_bottom_right: Vector2 = cam_pos + viewport_half / cam_zoom
	
	var chunk_size: float = float(world.get_chunk_size())
	var buffer_world: float = render_buffer_chunks * chunk_size
	
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
	var entity_positions: PackedVector2Array = world.get_visible_entities(
		visible_chunks,
		world_min,
		world_max,
		ENTITY_POOL_SIZE
	)
	
	# === 5. Update sprite pool (simple array assignment) ===
	var entity_count: int = entity_positions.size()
	
	# Show visible entities
	for i in entity_count:
		entity_sprites[i].global_position = entity_positions[i]
		entity_sprites[i].visible = true
	
	# Hide unused sprites
	for i in range(entity_count, ENTITY_POOL_SIZE):
		entity_sprites[i].visible = false
	
	# === 6. Background simulation (C++ handles this) ===
	world.update(cam_pos, simulation_distance, delta)
	
	# === 7. Debug (every second) ===
	if Engine.get_frames_drawn() % 60 == 0:
		print("Chunks: %d | Entities: %d | FPS: %.1f" % [
			visible_chunks.size(),
			entity_count,
			Performance.get_monitor(Performance.TIME_FPS)
		])
