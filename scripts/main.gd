# Main.gd
extends Node2D

@export var world_scale: float = 1.0

@onready var button_selector: Control = $BuildUI/BuildingInterface  # Adjust path
@onready var cam: Camera2D = $Camera2D

var current_build_type: SpriteAtlas.EntityType = -1  # -1 = nothing selected

func _ready() -> void:
	scale = Vector2(world_scale, world_scale)
	Engine.max_fps = 0
	DisplayServer.window_set_vsync_mode(DisplayServer.VSYNC_DISABLED)
	
	$World.init(GameSettings.max_world_tiles, GameSettings.max_world_tiles, GameSettings.chunk_size)
	
	var half_width = $World.get_world_width_tiles() / 2.0
	var half_height = $World.get_world_height_tiles() / 2.0
	cam.target_position = Vector2(half_width, half_height)
	
	# Connect selection
	button_selector.entity_type_selected.connect(_on_entity_type_selected)

func _on_entity_type_selected(type: SpriteAtlas.EntityType) -> void:
	current_build_type = type
	print("Ready to place: ", SpriteAtlas.EntityType.keys()[type])

func _input(event: InputEvent) -> void:

	if current_build_type == -1:
		print("No build type selected (click UI button first!)")
		return  

	if event is InputEventMouseButton and event.button_index == MOUSE_BUTTON_LEFT and event.pressed:
		print(">>> LEFT CLICK DETECTED!")

		var tile_size = GameSettings.tile_size
		var mouse_pos_screen: Vector2 = event.position
		var mouse_pos_world: Vector2 = cam.global_position + (mouse_pos_screen - get_viewport_rect().size / 2) / cam.zoom

		var tile_x: int = floor(mouse_pos_world.x / tile_size)
		var tile_y: int = floor(mouse_pos_world.y / tile_size) + 1
		var tile_pos := Vector2i(tile_x, tile_y)

		print(">>> PLACING ", SpriteAtlas.EntityType.keys()[current_build_type], " at ", tile_pos)

		$World.place_building_in_chunk(tile_pos, current_build_type)
