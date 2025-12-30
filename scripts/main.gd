# Main.gd
extends Node2D

@export var world_scale: float = 1.0

@onready var cam: Camera2D = $Camera2D


func _ready() -> void:
	scale = Vector2(world_scale, world_scale)
	Engine.max_fps = 0
	DisplayServer.window_set_vsync_mode(DisplayServer.VSYNC_DISABLED)
	
	$World.init(GameSettings.max_world_tiles, GameSettings.max_world_tiles, GameSettings.chunk_size)
	
	var half_width = $World.get_world_width_tiles() / 2.0
	var half_height = $World.get_world_height_tiles() / 2.0
	cam.target_position = Vector2(half_width, half_height)
	
	# Generate stuff in this area for starter stuff
	
