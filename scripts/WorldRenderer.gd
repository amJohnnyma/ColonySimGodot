# WorldRenderer.gd — MAX DISTANCE + TRUE ENTITY CULLING (FINAL)
extends Node2D

@export var world: World
@export var chunk_renderer_scene: PackedScene
@export var entity_sprite_scene: PackedScene

@export var max_render_distance_chunks: int = 2      # ← YOU control max chunks rendered
@export var render_buffer_chunks: int = 2           # ← extra chunks around screen
@export var simulation_distance: int = 12           # ← background simulation

@onready var entity_container: Node2D = $EntityContainer

var chunks: Dictionary = {}
var entity_sprites: Array[Sprite2D] = []
const ENTITY_POOL_SIZE: int = 25000
var pool_index: int = 0

func _ready() -> void:
	for i in ENTITY_POOL_SIZE:
		var s: Sprite2D = entity_sprite_scene.instantiate() as Sprite2D
		s.visible = false
		entity_container.add_child(s)
		entity_sprites.append(s)

func _process(delta: float) -> void:
	if not world or not chunk_renderer_scene or not is_instance_valid(get_viewport()):
		return
	
	var cam := get_viewport().get_camera_2d() as Camera2D
	if not cam:
		return
	
	var viewport_rect: Rect2 = get_viewport_rect()
	var cam_pos: Vector2 = cam.global_position
	var cam_zoom: Vector2 = cam.zoom
	
	# === 1. Screen-space bounds (with buffer) ===
	var screen_top_left: Vector2 = cam_pos + (viewport_rect.position - viewport_rect.size * 0.5) / cam_zoom
	var screen_bottom_right: Vector2 = cam_pos + (viewport_rect.end - viewport_rect.size * 0.5) / cam_zoom
	
	var chunk_size: float = float(world.get_chunk_size())
	var buffer_world: float = render_buffer_chunks * chunk_size
	
	var world_min: Vector2 = Vector2(screen_top_left.x - buffer_world, screen_top_left.y - buffer_world)
	var world_max: Vector2 = Vector2(screen_bottom_right.x + buffer_world, screen_bottom_right.y + buffer_world)
	
	var min_chunk: Vector2i = world.world_pos_to_chunk(world_min)
	var max_chunk: Vector2i = world.world_pos_to_chunk(world_max)
	
	# === 2. Clamp to max_render_distance ===
	var origin_chunk: Vector2i = world.world_pos_to_chunk(cam_pos)
	var clamped_min := Vector2i(
		max(origin_chunk.x - max_render_distance_chunks, min_chunk.x),
		max(origin_chunk.y - max_render_distance_chunks, min_chunk.y)
	)
	var clamped_max := Vector2i(
		min(origin_chunk.x + max_render_distance_chunks, max_chunk.x),
		min(origin_chunk.y + max_render_distance_chunks, max_chunk.y)
	)
	
	var needed_chunks: Dictionary = {}
	
	# === 3. Load only visible + buffer + clamped ===
	for cy in range(clamped_min.y, clamped_max.y + 1):
		for cx in range(clamped_min.x, clamped_max.x + 1):
			var c: Vector2i = Vector2i(cx, cy)
			if not world.is_valid_chunk(c):
				continue
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
	
	# === 4. ENTITY RENDERING — ONLY IF ON SCREEN (strict culling) ===
	pool_index = 0
	var screen_margin: float = 100.0  # pixels off-screen still allowed
	
	for c in needed_chunks.keys():
		var entity_count: int = world.get_chunk_entity_count(c)
		for i in entity_count:
			if pool_index >= entity_sprites.size():
				break
				
			var pos: Vector2 = world.get_entity_position(c, i)
			var screen_pos: Vector2 = cam.get_screen_center_position() + (pos - cam_pos) * cam_zoom
			
			# TRUE off-screen culling
			if screen_pos.x < -screen_margin or screen_pos.x > viewport_rect.size.x + screen_margin or \
			   screen_pos.y < -screen_margin or screen_pos.y > viewport_rect.size.y + screen_margin:
				continue  # Skip — not visible
				
			var sprite: Sprite2D = entity_sprites[pool_index]
			sprite.global_position = pos
			sprite.visible = true
			pool_index += 1
	
	# Hide unused
	for i in range(pool_index, entity_sprites.size()):
		entity_sprites[i].visible = false
	
	# === 5. Background simulation ===
	world.update(cam.global_position, simulation_distance, delta)
	
	# Debug
	if Engine.get_frames_drawn() % 60 == 0:
		print("Chunks: %d | Entities ON-SCREEN: %d | FPS: %.1f" % [
			needed_chunks.size(), pool_index,
			Performance.get_monitor(Performance.TIME_FPS)
		])
