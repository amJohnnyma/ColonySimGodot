@tool
extends Node2D

@export var line_color: Color = Color(1, 0, 0, 0.8)      # semi-transparent red
@export var line_width: float = 2.0
@export var highlight_offset_y: float = 0.5

@onready var ghost_sprite: Sprite2D = $Ghost

var current_tile: Vector2i = Vector2i(-999, -999)

var tile_size = GameSettings.tile_size



func _ready() -> void:
	if Engine.is_editor_hint():
		# Optional: make ghost visible in editor for testing
		# ghost_sprite.visible = true
		pass


func _process(_delta: float) -> void:
	update_highlight()
	
	if ghost_sprite == null:
		return
	# Follow mouse in world space
	if ghost_sprite.visible:
		var cam = get_viewport().get_camera_2d()
		if cam:
			var mouse_screen = get_viewport().get_mouse_position()
			var mouse_world = cam.get_canvas_transform().affine_inverse() * mouse_screen
			var tile_x = floori(mouse_world.x / tile_size)
			var tile_y = floori(mouse_world.y / tile_size)
			var new_tile = Vector2i(tile_x, tile_y + 1)
			ghost_sprite.global_position = new_tile


func update_highlight() -> void:
	var cam = get_viewport().get_camera_2d()
	if not cam:
		return
	
	var mouse_screen = get_viewport().get_mouse_position()
	var mouse_world = cam.get_canvas_transform().affine_inverse() * mouse_screen
	
	var tile_x = floori(mouse_world.x / tile_size)
	var tile_y = floori(mouse_world.y / tile_size)
	var new_tile = Vector2i(tile_x, tile_y)
	
	if new_tile != current_tile:
		current_tile = new_tile
		queue_redraw()


func _draw() -> void:
	if current_tile.x == -999:
		return
	
	var origin = Vector2(current_tile) * tile_size
	
	# Draw tile border slightly lifted
	var p1 = origin + Vector2(0, highlight_offset_y)
	var p2 = origin + Vector2(tile_size, highlight_offset_y)
	var p3 = origin + Vector2(tile_size, tile_size + highlight_offset_y)
	var p4 = origin + Vector2(0, tile_size + highlight_offset_y)
	
	draw_line(p1, p2, line_color, line_width)
	draw_line(p2, p3, line_color, line_width)
	draw_line(p3, p4, line_color, line_width)
	draw_line(p4, p1, line_color, line_width)


func update_selected_sprite_ghost(
	atlas_tex: AtlasTexture,
	custom_scale: Vector2 = Vector2.ONE,
	custom_offset: Vector2 = Vector2.ZERO,
	visible : bool = true
) -> void:
	if !visible:
		ghost_sprite.visible = false
		return
	ghost_sprite.texture = atlas_tex
	ghost_sprite.scale   = custom_scale
	ghost_sprite.offset  = custom_offset          # this controls pivot/alignment
	ghost_sprite.modulate.a = 0.6                 # ghost transparency
	ghost_sprite.visible = true


func clear_ghost() -> void:
	ghost_sprite.visible = false
	ghost_sprite.texture = null
