# SpriteSheetTester.gd — COMPLETE, WORKING, NO ERRORS
@tool
extends Node2D

@export var sprite_texture: Texture2D
@export var test_region: Rect2i = Rect2i(0, 0, 64, 64)
@export var border_color: Color = Color.YELLOW
@export var border_width: float = 3.0
@export var sheet_scale: Vector2 = Vector2.ONE

@onready var sheet_sprite: Sprite2D = $SheetSprite
@onready var ui_layer: CanvasLayer = $UI

var ui: Control = null
var info_label: Label = null  # Direct reference to avoid get_node errors

func _ready() -> void:
	if sheet_sprite == null:
		push_error("SpriteSheetTester: Missing child Sprite2D named 'SheetSprite'!")
		return
	if ui_layer == null:
		push_error("SpriteSheetTester: Missing child CanvasLayer named 'UI'!")
		return

	if sprite_texture:
		sheet_sprite.texture = sprite_texture

	sheet_sprite.centered = false
	sheet_sprite.region_enabled = true
	sheet_sprite.scale = sheet_scale

	update_region()
	setup_ui()  # Now safely creates UI and info_label


func setup_ui() -> void:
	# Clean up any old UI
	if ui:
		ui.queue_free()
		ui = null
		info_label = null

	ui = Control.new()
	ui.anchor_left = 0.02
	ui.anchor_top = 0.02
	ui.anchor_right = 0.02
	ui.anchor_bottom = 0.98
	ui.offset_right = 320
	ui_layer.add_child(ui)

	var vbox = VBoxContainer.new()
	vbox.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	vbox.add_theme_constant_override("separation", 12)
	ui.add_child(vbox)

	# Title
	var title = Label.new()
	title.text = "Sprite Sheet Tester"
	title.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	title.add_theme_font_size_override("font_size", 22)
	vbox.add_child(title)

	# Scale section
	var scale_label = Label.new()
	scale_label.text = "Sheet Scale"
	vbox.add_child(scale_label)

	var scale_hbox = HBoxContainer.new()
	vbox.add_child(scale_hbox)

	var zoom_in = Button.new()
	zoom_in.text = "Zoom In"
	zoom_in.pressed.connect(_on_scale_up)
	scale_hbox.add_child(zoom_in)

	var zoom_out = Button.new()
	zoom_out.text = "Zoom Out"
	zoom_out.pressed.connect(_on_scale_down)
	scale_hbox.add_child(zoom_out)

	var reset_btn = Button.new()
	reset_btn.text = "Reset Scale"
	reset_btn.pressed.connect(_on_reset_scale)
	vbox.add_child(reset_btn)

	# Region section
	var region_label = Label.new()
	region_label.text = "Test Region (X,Y,W,H)"
	vbox.add_child(region_label)

	# Position controls
	var pos_hbox = HBoxContainer.new()
	vbox.add_child(pos_hbox)

	pos_hbox.add_child(_make_button("←", func():
		test_region.position.x = max(0, test_region.position.x - 1)
		update_region()
	))
	pos_hbox.add_child(_make_button("→", func():
		test_region.position.x += 1
		update_region()
	))
	pos_hbox.add_child(_make_button("↑", func():
		test_region.position.y = max(0, test_region.position.y - 1)
		update_region()
	))
	pos_hbox.add_child(_make_button("↓", func():
		test_region.position.y += 1
		update_region()
	))

	# Size controls
	var size_hbox = HBoxContainer.new()
	vbox.add_child(size_hbox)

	size_hbox.add_child(_make_button("-W", func():
		test_region.size.x = max(1, test_region.size.x - 1)
		update_region()
	))
	size_hbox.add_child(_make_button("+W", func():
		test_region.size.x += 1
		update_region()
	))
	size_hbox.add_child(_make_button("-H", func():
		test_region.size.y = max(1, test_region.size.y - 1)
		update_region()
	))
	size_hbox.add_child(_make_button("+H", func():
		test_region.size.y += 1
		update_region()
	))

	# Info label — saved as reference
	info_label = Label.new()
	info_label.add_theme_font_size_override("font_size", 14)
	vbox.add_child(info_label)

	update_info()


# Helper function to create buttons cleanly
func _make_button(text: String, on_pressed: Callable) -> Button:
	var btn = Button.new()
	btn.text = text
	btn.pressed.connect(on_pressed)
	return btn


func update_region() -> void:
	sheet_sprite.region_rect = Rect2(test_region.position, test_region.size)
	queue_redraw()
	update_info()


func update_info() -> void:
	if info_label == null:
		return
	info_label.text = """
Region: %s
Position: (%d, %d)
Size: %d × %d
Scale: %.2f × %.2f

Copy & paste:
Rect2(%d, %d, %d, %d)
""" % [
		test_region,
		test_region.position.x, test_region.position.y,
		test_region.size.x, test_region.size.y,
		sheet_scale.x, sheet_scale.y,
		test_region.position.x, test_region.position.y,
		test_region.size.x, test_region.size.y
	]


func _on_scale_up() -> void:
	sheet_scale *= 1.25
	sheet_sprite.scale = sheet_scale
	update_info()

func _on_scale_down() -> void:
	sheet_scale *= 0.8
	sheet_sprite.scale = sheet_scale
	update_info()

func _on_reset_scale() -> void:
	sheet_scale = Vector2.ONE
	sheet_sprite.scale = sheet_scale
	update_info()


func _draw() -> void:
	if test_region.size == Vector2i.ZERO:
		return
	var scaled_pos = Vector2(test_region.position) * sheet_scale
	var scaled_size = Vector2(test_region.size) * sheet_scale
	var rect = Rect2(scaled_pos, scaled_size)
	draw_rect(rect, border_color, false, border_width)


func toggle_visible() -> void:
	visible = !visible
	if visible:
		update_region()
