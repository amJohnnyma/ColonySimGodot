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
	
	# Temporary to see all sprites
	# void create_entity(const String &type, const Vector2i &coord,const int &entity_type, const int &entity_sprite);

# Put this in your Main.gd or any control/test script



	const NUM_SPRITES_COLONIST : int = 15
	const NUM_SPRITES_BUILDING : int = 135
	const NUM_SPRITES_ITEMS : int = 27 * 26
	var max_world_tile = GameSettings.max_world_tiles
	var CHUNK_SIZE : int = GameSettings.chunk_size
	var world_center_chunk : Vector2i = Vector2i((max_world_tile/2) / CHUNK_SIZE,(max_world_tile/2) / CHUNK_SIZE)
	for chunk_x in range(2):  # 0,1
		for chunk_y in range(2):  # 0,1
			for local_y in range(CHUNK_SIZE):
				for local_x in range(CHUNK_SIZE):
					# World tile coord
					var world_coord : Vector2i = Vector2i(
						(chunk_x + world_center_chunk.x) * CHUNK_SIZE + local_x,
						(chunk_y + world_center_chunk.y) * CHUNK_SIZE + local_y
					)
					
					# Cycle through sprite indices (0-14)
					var sprite_idx : int = (local_x + local_y) % NUM_SPRITES_COLONIST
					
					# Create entity
					$World.create_entity("colonist", world_coord, 1, sprite_idx)
'''
	for chunk_x in range(2):  # 0,1
		for chunk_y in range(2):  # 0,1
			for sprite_idx in range(NUM_SPRITES_COLONIST):
	# Local coord: horizontal row
				var local_x : int = sprite_idx  # 0-14
				var local_y : int = 0            # Top row

				# World tile coord
				var world_coord : Vector2i = Vector2i(
					(chunk_x + world_center_chunk.x) * CHUNK_SIZE + local_x,
					(chunk_y + world_center_chunk.y) * CHUNK_SIZE + local_y
				)

	# Create entity (sprite_idx = 0-14 for full sheet)
				$World.create_entity("colonist", world_coord, 1, sprite_idx)
				

	var count : int = 0
	$World.create_entity("building", Vector2i((0 + world_center_chunk.x) * CHUNK_SIZE + 0, (0 + world_center_chunk.y) * CHUNK_SIZE + 0), 1, 0)
	$World.create_entity("building", Vector2i((0 + world_center_chunk.x) * CHUNK_SIZE + 4, (0 + world_center_chunk.y) * CHUNK_SIZE + 0), 1, 1)
	$World.create_entity("building", Vector2i((0 + world_center_chunk.x) * CHUNK_SIZE + 8, (0 + world_center_chunk.y) * CHUNK_SIZE + 0), 1, 2)
	$World.create_entity("building", Vector2i((0 + world_center_chunk.x) * CHUNK_SIZE + 10, (0 + world_center_chunk.y) * CHUNK_SIZE + 0), 1, 5)
	$World.create_entity("building", Vector2i((0 + world_center_chunk.x) * CHUNK_SIZE + 17, (0 + world_center_chunk.y) * CHUNK_SIZE + 0), 1, 4)
	var item_count_total : int = 0
	for chunk_x in range(2):  # 0,1
		for chunk_y in range(2):  # 0,1
			count = 0
			for sprite_idx in range(NUM_SPRITES_ITEMS):
	# Local coord: horizontal row
				var local_x : int = sprite_idx % 27  # 0-14
				var local_y : int = 1 + floor(count/27)            # Top row

				# World tile coord
				var world_coord : Vector2i = Vector2i(
					(chunk_x + world_center_chunk.x) * CHUNK_SIZE + local_x,
					(chunk_y + world_center_chunk.y) * CHUNK_SIZE + local_y
				)
				count+=1

	# Create entity (sprite_idx = 0-14 for full sheet)
				item_count_total+=1
				$World.create_entity("item", world_coord, 1, sprite_idx)
				
	print("Items made: ", item_count_total)
	for chunk_x in range(2):  # 0,1
		for chunk_y in range(2):  # 0,1
			count = 0
			for sprite_idx in range(NUM_SPRITES_BUILDING):
	# Local coord: horizontal row
				var local_x : int = sprite_idx%16
				var local_y : int = 1 + floor(count/16)         # Top row

				# World tile coord
				var world_coord : Vector2i = Vector2i(
					(chunk_x + world_center_chunk.x) * CHUNK_SIZE + local_x,
					(chunk_y + world_center_chunk.y) * CHUNK_SIZE + local_y
				)
				count+=1
	# Create entity (sprite_idx = 0-14 for full sheet)
				$World.create_entity("building", world_coord, 1, sprite_idx)
'''

func _input(event):
	if event is InputEventMouseButton and event.button_index == MOUSE_BUTTON_LEFT and event.pressed:
		# Get the world position under the mouse click
		var camera = get_viewport().get_camera_2d()
		if camera == null:
			print("No Camera2D found!")
			return
		
		var world_click_pos = camera.get_global_mouse_position()
		# Alternative if no camera: get_viewport().get_mouse_position() then project it
		
		# Call your World function (adjust the path if World is not a sibling)
		var world = $World  # Change this to match your scene tree, e.g. get_node("/root/World")
		# Or if World is an autoload: var world = World
		
		var result = world.get_entities_at_world_pos(world_click_pos)
		
		var count = result.get("count", 0)
		if count == 0:
			print("No entities found at ", world_click_pos)
			return
		
		print("Found %d entit(y/ies) at %s:" % [count, world_click_pos])
		
		var ids = result["entity_ids"]
		var types = result["types"]
		var sprites = result["entity_sprites"]
		
		for i in count:
			print("  - ID: ", ids[i],
				  " | Type: ", types[i],
				  " | Sprite: ", sprites[i])
