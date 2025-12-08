extends Node2D

@export var world: World
@export var chunk_renderer_scene: PackedScene
@export var render_distance := 8

var chunks := {}

func _process(_delta: float) -> void:
	if !world || !chunk_renderer_scene:
		return

	var cam := get_viewport().get_camera_2d()
	if !cam:
		return

	var origin: Vector2i = world.world_pos_to_chunk(cam.global_position)

	var needed := {}

	for dy in range(-render_distance, render_distance + 1):
		for dx in range(-render_distance, render_distance + 1):
			var c: Vector2i = origin + Vector2i(dx, dy)
			if !world.is_valid_chunk(c):
				continue

			needed[c] = true

			if !chunks.has(c):
				var renderer := chunk_renderer_scene.instantiate() as ChunkRenderer
				add_child(renderer)
				renderer.setup(world, c)
				chunks[c] = renderer
				#dssprint("Loaded chunk ", c)

	for c: Vector2i in chunks.keys():
		if !needed.has(c):
			chunks[c].queue_free()
			chunks.erase(c)

	world.update(cam.global_position, render_distance + 2, _delta)
