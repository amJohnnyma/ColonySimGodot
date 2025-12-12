class_name ChunkRenderer
extends MultiMeshInstance2D

var world: World
var chunk_coord := Vector2i(0, 0)
var debug_label: Label
var entity_count_total := 0

func setup(p_world: World, coord: Vector2i) -> void:
	world = p_world
	chunk_coord = coord

	multimesh = MultiMesh.new()
	multimesh.transform_format = MultiMesh.TRANSFORM_2D
	multimesh.use_colors = true
	position = chunk_coord * world.get_chunk_size()

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

		multimesh.set_instance_transform_2d(i, Transform2D(0, Vector2(lx, ly)))
		multimesh.set_instance_color(i, col)
		#multimesh.set_instance_transform_2d(i, Transform2D(0, Vector2(lx, ly)))

# === NEW: Called every frame (or less) from WorldRenderer ===
func set_debug_text(total: int, visible: int) -> void:
# Create the label ONCE the first time we're called
	if not debug_label:
		debug_label = Label.new()
		debug_label.set_anchors_and_offsets_preset(Control.PRESET_CENTER)
		debug_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
		debug_label.vertical_alignment   = VERTICAL_ALIGNMENT_CENTER
		# Crisp tiny text in one line
		debug_label.add_theme_font_size_override("font_size", 8)
		debug_label.set("custom_fonts/font", null)  # disables blurry distance-field font

		debug_label.add_theme_constant_override("outline_size", 2)
		debug_label.add_theme_color_override("font_outline_color", Color8(0,0,0))
		debug_label.add_theme_color_override("font_color", Color.YELLOW)
		debug_label.z_index = 1000
		add_child(debug_label)
		
		# Perfectly center in chunk
		var cs := world.get_chunk_size()
		debug_label.position = Vector2(cs * 0.5, cs * 0.5)
		#debug_label.anchors_preset = Control.PRESET_CENTER
		#debug_label.position = Vector2(cs, cs)

	# Update text
	debug_label.text = "%d (%d)" % [total, visible]
	debug_label.visible = (total > 0 or visible > 0)
