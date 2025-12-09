# Main.gd — FIXED ORDER (scale FIRST, then camera)
extends Node2D

@export var world_scale: float = 8.0   # ← 8 = perfect size

func _ready():
	# 1. Scale FIRST
	scale = Vector2(world_scale, world_scale)
	print("World scaled to ", world_scale, "×")
	
	# 2. Unlock FPS
	Engine.max_fps = 0
	DisplayServer.window_set_vsync_mode(DisplayServer.VSYNC_DISABLED)
	
	# 3. Init world
	$World.init(10_000, 10_000, 128)
	
	# 4. Camera position AFTER scale
	var half_width = $World.get_world_width_tiles() / 2.0
	var half_height = $World.get_world_height_tiles() / 2.0
	$Camera2D.target_position = Vector2(half_width, half_height)
	
	print("Camera at center: ", $Camera2D.global_position)
