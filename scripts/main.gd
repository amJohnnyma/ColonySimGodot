# Main.gd
extends Node2D

@export var world_scale: float = 1.0
@onready var cam: Camera2D = $Camera2D
@onready var ui: Control = $UI/MasterControl
@onready var selectedEntity : Control = $UI/MasterControl/EntityClickPopup

@export var selectedSprite : Array = [0,0, 0]

# Helper function - now correctly placed at script level
func random_pos(rng) -> Vector2i:
	var max_world_tile : int = GameSettings.max_world_tiles
	var min_coord : int = 0
	var max_coord : int = max_world_tile - 1
	
	var center = max_world_tile / 2.0
	var sigma = max_world_tile / 6.0
	var x = round(rng.randfn(center, sigma))
	var y = round(rng.randfn(center, sigma))
	x = clamp(x, min_coord, max_coord)
	y = clamp(y, min_coord, max_coord)
	return Vector2i(x, y)

func _ready() -> void:
	scale = Vector2(world_scale, world_scale)
	Engine.max_fps = 0
	DisplayServer.window_set_vsync_mode(DisplayServer.VSYNC_DISABLED)
	
	$World.init(GameSettings.max_world_tiles, GameSettings.max_world_tiles, GameSettings.chunk_size)
	
	var half_width = $World.get_world_width_tiles() / 2.0
	var half_height = $World.get_world_height_tiles() / 2.0
	cam.target_position = Vector2(half_width, half_height)
	
	ui.building_selected.connect(_on_building_selected)
	ui.update_place_ghost.connect(_update_place_ghost)


'''
	# ===================================================================
	# TEMPORARY: Large-scale procedural generation for testing sprites
	# ===================================================================
	# Comment out or delete this entire block when no longer needed.
	
	const NUM_SPRITES_COLONIST : int = 15
	const NUM_SPRITES_BUILDING : int = 135
	const NUM_SPRITES_ITEMS : int = 27 * 26
	
	const NUM_BUILDINGS : int = 1
	const NUM_COLONISTS : int = 4
	const ITEMS_PER_BUILDING_MIN : int = 1
	const ITEMS_PER_BUILDING_MAX : int = 2
	
	const MIN_BUILDING_DIST : int = 11      # ~10 tile clear radius around buildings
	const ITEM_MAX_DIST : int = 5           # Items placed within this radius
	
	const MAX_TRIES : int = 10_000_000
	
	var max_world_tile : int = GameSettings.max_world_tiles
	var min_coord : int = 0
	var max_coord : int = max_world_tile - 1
	
	var rng := RandomNumberGenerator.new()
	rng.randomize()
	
	var occupied := {}                       # Dictionary[Vector2i, bool]
	var building_positions : Array[Vector2i] = []
	
	# === Place buildings ===
	var tries := 0
	while building_positions.size() < NUM_BUILDINGS and tries < MAX_TRIES:
		tries += 1
		var pos := random_pos(rng)  # Now valid - calls the member function
		
		if occupied.has(pos):
			continue
		
		var too_close := false
		for other in building_positions:
			if (pos - other).length() < MIN_BUILDING_DIST:
				too_close = true
				break
		if too_close:
			continue
		
		var sprite_idx := rng.randi_range(0, NUM_SPRITES_BUILDING - 1)
		$World.create_entity("building", pos, 1, sprite_idx)
		
		occupied[pos] = true
		building_positions.append(pos)
	
	print("Buildings placed: ", building_positions.size())
	
	# === Place items near buildings ===
	var item_count_total := 0
	for bpos in building_positions:
		var num_items := rng.randi_range(ITEMS_PER_BUILDING_MIN, ITEMS_PER_BUILDING_MAX)
		for _i in num_items:
			var item_tries := 0
			var placed := false
			while item_tries < 100 and not placed:
				item_tries += 1
				var dx := rng.randi_range(-ITEM_MAX_DIST, ITEM_MAX_DIST)
				var dy := rng.randi_range(-ITEM_MAX_DIST, ITEM_MAX_DIST)
				if dx == 0 and dy == 0:
					continue
				if sqrt(dx * dx + dy * dy) > ITEM_MAX_DIST:
					continue
				
				var ipos := bpos + Vector2i(dx, dy)
				if ipos.x < min_coord or ipos.x > max_coord or ipos.y < min_coord or ipos.y > max_coord:
					continue
				if occupied.has(ipos):
					continue
				
				var sprite_idx := rng.randi_range(0, NUM_SPRITES_ITEMS - 1)
				$World.create_entity("item", ipos, 1, sprite_idx)
				
				occupied[ipos] = true
				item_count_total += 1
				placed = true
	
	print("Items made: ", item_count_total)
	
	# === Place colonists ===
	tries = 0
	var colonist_count := 0
	while colonist_count < NUM_COLONISTS and tries < MAX_TRIES:
		tries += 1
		var pos := random_pos(rng)
		
		if occupied.has(pos):
			continue
		
		var sprite_idx := rng.randi_range(0, NUM_SPRITES_COLONIST - 1)
		$World.create_entity("colonist", pos, 1, sprite_idx)
		
		occupied[pos] = true
		colonist_count += 1
	
	print("Colonists placed: ", colonist_count)
	
	# ===================================================================
	# END OF TEMPORARY GENERATION
	# ===================================================================
'''
func _on_building_selected(sheet_id: int, variant_id: int) -> void:
	# unselect it now
	if sheet_id == selectedSprite[0] and variant_id == selectedSprite[1]:
		selectedSprite[0] = 0
		selectedSprite[1] = 0
		selectedSprite[2] = 0
		print("Main received deselection → sheet: %d  variant: %d" % [sheet_id, variant_id])
	else:
		selectedSprite[0] = sheet_id
		selectedSprite[1] = variant_id
		selectedSprite[2] = 1
		print("Main received selection → sheet: %d  variant: %d" % [sheet_id, variant_id])

func _update_place_ghost(sprite : AtlasTexture, c_scale : Vector2, offset : Vector2) -> void:
	if selectedSprite[0] == 0:
		selectedSprite[2] = 0
	$GameSystems/GridHighlight.update_selected_sprite_ghost(sprite, c_scale, offset, selectedSprite[2])



func _unhandled_input(event):
	if event is InputEventKey and event.keycode == KEY_P and event.pressed:
		GameSettings.paused = !GameSettings.paused
	if event is InputEventMouseButton and event.button_index == MOUSE_BUTTON_LEFT and event.pressed:
		if selectedSprite[2]:
			print("Placing sprite")
			var camera := get_viewport().get_camera_2d()
			if camera == null:
				print("No Camera2D found!")
				return
			
			var world_click_pos := camera.get_global_mouse_position()
			var tile_x = floori(world_click_pos.x / GameSettings.tile_size)
			var tile_y = floori(world_click_pos.y / GameSettings.tile_size)
			var new_tile = Vector2i(tile_x, tile_y + 1)
			print("Tile place: ", new_tile)
			var type = "building"
			if selectedSprite[0] == 1:
				type = "colonist"
			elif selectedSprite[0] == 2:
				type = "building"
			elif selectedSprite[0] == 3:
				type = "item"
			$World.create_entity(type, new_tile, selectedSprite[0], selectedSprite[1])
		else:
			var camera := get_viewport().get_camera_2d()
			if camera == null:
				print("No Camera2D found!")
				return
			
			var world_click_pos := camera.get_global_mouse_position()
			
			var result = $World.get_entities_at_world_pos(world_click_pos)
			var count = result.get("count", 0)
			world_click_pos.y += 1
			
			if count == 0:
				print("No entities found at ", world_click_pos)
				return
			
			print("Found %d entit(y/ies) at %s:" % [count, world_click_pos])
			var ids = result["entity_ids"]
			var types = result["types"]
			var sprites = result["entity_sprites"]
			var pos_x = result["x_pos"]
			var pos_y = result["y_pos"]
			
			var pos : Array[Vector2i] 
			
			for i in count:
				print(" - ID: ", ids[i],
					  " | Type: ", types[i],
					  " | Sprite: ", sprites[i])
				pos.push_back(Vector2i(pos_x[i], pos_y[i]))
					
			# Show hide correct UI
			$UI/MasterControl/GameUI.visible=false
			$UI/MasterControl/EntityClickPopup.visible=true
			
			selectedEntity.entities_selected(ids, types, sprites, pos)


func create_entity_job(pos : Vector2i, entityPos : Vector2i):
	# For making an entity job -> Temporary
	$World.create_temp_job(pos, entityPos)
