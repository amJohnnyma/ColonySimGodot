# Main.gd  ← attach to the root Node2D
extends Node2D

func _ready():
	Engine.max_fps = 0  # unlimited
	DisplayServer.window_set_vsync_mode(DisplayServer.VSYNC_DISABLED)
	$World.init(2_000_000, 2_000_000, 16)
	print("World initialized – 2 million × 2 million tiles")    
	# Start camera in EXACT middle of world
	var half_width = $World.get_world_width_tiles() / 2.0
	var half_height = $World.get_world_height_tiles() / 2.0
	$Camera2D.global_position = Vector2(half_width, half_height)

	print("Camera started at world center: ", $Camera2D.global_position)
