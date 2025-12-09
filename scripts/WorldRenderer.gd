# WorldRenderer.gd — SCREEN-SPACE CULLING + BUFFER (perfect pop-in free)
extends Node2D

@export var world: World
@export var chunk_renderer_scene: PackedScene
@export var entity_sprite_scene: PackedScene

@export var render_buffer_chunks: int = 2   # ← 1 or 2 extra chunks around screen
@export var simulation_distance: int = 6    # ← background sim (unchanged)

@onready var entity_container: Node2D = $EntityContainer

var chunks: Dictionary = {}                    # Vector2i → ChunkRenderer
var entity_sprites: Array[Sprite2D] = []
const ENTITY_POOL_SIZE: int = 20000
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
	
	var cam := get_viewport().get_camera_2d()
	if not cam:
		return
	
	var viewport_rect := get_viewport_rect()
	var cam_pos := cam.global_position
	var cam_zoom := cam.zoom
	
	# Convert screen corners to world space
	var screen_top_left := cam_pos + (viewport_rect.position - viewport_rect.size * 0.5) / cam_zoom
	var screen_bottom_right := cam_pos + (viewport_rect.end - viewport_rect.size * 0.5) / cam_zoom
	
	# Add buffer in world units
	var chunk_size := float(world.get_chunk_size())
	var buffer_world := render_buffer_chunks * chunk_size
	
	var min_world := Vector2(screen_top_left.x - buffer_world, screen_top_left.y - buffer_world)
	var max_world := Vector2(screen_bottom_right.x + buffer_world, screen_bottom_right.y + buffer_world)
	
	# Convert to chunk coordinates
	var min_chunk := world.world_pos_to_chunk(min_world)
	var max_chunk := world.world_pos_to_chunk(max_world)
	
	var needed_chunks: Dictionary = {}
	
	# Load only chunks inside the screen + buffer
	for cy in range(min_chunk.y, max_chunk.y + 1):
		for cx in range(min_chunk.x, max_chunk.x + 1):
			var c := Vector2i(cx, cy)
			if not world.is_valid_chunk(c):
				continue
			needed_chunks[c] = true
			
			if not chunks.has(c):
				var renderer := chunk_renderer_scene.instantiate() as ChunkRenderer
				add_child(renderer)
				renderer.setup(world, c)
				chunks[c] = renderer
	
	# Unload chunks outside view + buffer
	for c in chunks.keys():
		if not needed_chunks.has(c):
			chunks[c].queue_free()
			chunks.erase(c)
	
	# Draw visible entities
	pool_index = 0
	for c in needed_chunks.keys():
		var entity_count := world.get_chunk_entity_count(c)
		for i in entity_count:
			if pool_index >= entity_sprites.size():
				break
			var sprite := entity_sprites[pool_index]
			sprite.global_position = world.get_entity_position(c, i)
			sprite.visible = true
			pool_index += 1
	
	for i in range(pool_index, entity_sprites.size()):
		entity_sprites[i].visible = false
	
	# Background simulation (unchanged)
	world.update(cam.global_position, simulation_distance, delta)
	
	# Debug
	if Engine.get_frames_drawn() % 60 == 0:
		print("Visible chunks: %d | Entities: %d | FPS: %.1f" % [
			needed_chunks.size(), pool_index,
			Performance.get_monitor(Performance.TIME_FPS)
		])
