@tool  # So it works in editor too — great for debugging!
extends Node2D

@export var line_color: Color = Color.RED
@export var line_width: float = 0.1
@export var highlight_offset_y: float = 0.01  # Tiny lift to avoid z-fight with tiles

var current_tile: Vector2 = Vector2(-999, -999)  # Invalid start


func _process(_delta):
	update_highlight()

func update_highlight():
	var cam: Camera2D = get_viewport().get_camera_2d()
	if not cam:
		return
		
	var tile_size = GameSettings.tile_size
	
	var mouse_pos_screen: Vector2 = get_viewport().get_mouse_position()
	var mouse_pos_world: Vector2 = cam.global_position + (mouse_pos_screen - get_viewport_rect().size / 2) / cam.zoom
	
	# Convert world position → tile coordinate
	var tile_x: int = floor(mouse_pos_world.x / tile_size)
	var tile_y: int = floor(mouse_pos_world.y / tile_size)
	var new_tile := Vector2(tile_x, tile_y)
	
	if new_tile != current_tile:
		current_tile = new_tile
		queue_redraw()  # Only redraw when mouse moves to new tile

func _draw():
	if current_tile.x == -999:
		return
	var tile_size : float = GameSettings.tile_size

	# Compute the four corners of the tile (in local space of this Node2D)
	var origin := current_tile * tile_size
	var size_vec := Vector2(tile_size, tile_size)
	
	var p1 := origin
	var p2 := origin + Vector2(size_vec.x, 0)
	var p3 := origin + size_vec
	var p4 := origin + Vector2(0, size_vec.y)
	
	# Slight Y offset to draw above tiles
	p1.y += highlight_offset_y
	p2.y += highlight_offset_y
	p3.y += highlight_offset_y
	p4.y += highlight_offset_y
	
	# Draw the four red lines
	draw_line(p1, p2, line_color, line_width)
	draw_line(p2, p3, line_color, line_width)
	draw_line(p3, p4, line_color, line_width)
	draw_line(p4, p1, line_color, line_width)
	
	# Optional: draw cross for extra clarity
	# draw_line(p1 + size_vec/2, p3 - size_vec/2, line_color, line_width)
