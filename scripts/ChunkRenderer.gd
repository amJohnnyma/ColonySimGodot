class_name ChunkRenderer
extends MultiMeshInstance2D

var world: World
var chunk_coord := Vector2i(0, 0)

func setup(p_world: World, coord: Vector2i) -> void:
	world = p_world
	chunk_coord = coord

	multimesh = MultiMesh.new()
	multimesh.transform_format = MultiMesh.TRANSFORM_2D
	multimesh.use_colors = true

	var quad := QuadMesh.new()
	quad.size = Vector2(1, 1)
	quad.center_offset = Vector3(0.5, -0.5, 1)
	multimesh.mesh = quad

	var cs := world.get_chunk_size()
	multimesh.instance_count = cs * cs

	rebuild()

func rebuild() -> void:
	var cs := world.get_chunk_size()
	var colors: Array[Color] = []
	var raw = world.get_chunk_colors(chunk_coord)
	colors.assign(raw)

	for i in multimesh.instance_count:
		var lx := i % cs
		var ly := i / cs
		var wx := chunk_coord.x * cs + lx
		var wy := chunk_coord.y * cs + ly

		var col: Color = Color(0.8, 0.6, 1.0) if i >= colors.size() else colors[i]

		multimesh.set_instance_transform_2d(i, Transform2D(0, Vector2(wx, wy)))
		multimesh.set_instance_color(i, col)
