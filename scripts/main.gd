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
	print(OS.get_user_data_dir())
	scale = Vector2(world_scale, world_scale)
	Engine.max_fps = 0
	DisplayServer.window_set_vsync_mode(DisplayServer.VSYNC_DISABLED)
	
	$World.init(GameSettings.max_world_tiles, GameSettings.max_world_tiles, GameSettings.chunk_size)
	var world_width = $World.get_world_width_tiles()
	var world_height = $World.get_world_height_tiles()
	var half_width = world_width / 2.0
	var half_height = world_height / 2.0
	cam.target_position = Vector2(half_width, half_height)
	
	ui.building_selected.connect(_on_building_selected)
	ui.update_place_ghost.connect(_update_place_ghost)
	
	update_paused_icon()


	# ===================================================================
	# TEMPORARY: Large-scale procedural generation for testing sprites
	# ===================================================================
	# Comment out or delete this entire block when no longer needed.
	const NUM_COLONISTS : int = 15
	
	const NUM_SPRITES_COLONIST : int = 15
	const NUM_SPRITES_BUILDING : int = 135
	const NUM_SPRITES_ITEMS : int = 27 * 26
	
	const NUM_BUILDINGS : int = 1
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
	var colonist_count := 0
	var jobTypeCount = 0

	# Calculate positions for each corner
	var top_left = Vector2i(0, 0)
	var top_right = Vector2i(world_width - 15, 0)
	var bottom_left = Vector2i(0, world_height - 15)
	var bottom_right = Vector2i(world_width - 15, world_height - 15)

	# Target in the center
	var target = Vector2i(half_width, half_height)

	var corners = [
		{"pos": top_left, "offset": Vector2i(1, 0)},
		{"pos": top_right, "offset": Vector2i(-1, 0)},
		{"pos": bottom_left, "offset": Vector2i(1, 0)},
		{"pos": bottom_right, "offset": Vector2i(-1, 0)}
	]

	for corner in corners:
		var pos = corner["pos"]
		var offset = corner["offset"]

		for i in range(NUM_COLONISTS):
			var sprite_idx := i
			$World.create_entity("colonist", pos, 1, sprite_idx)

			# Make target slightly different for each colonist to spread them out
			var colonist_target = target + Vector2i(sprite_idx % 10, (sprite_idx / 10) % 10)

			var jobType = 1 if jobTypeCount % 2 == 0 else 2
			$World.create_temp_job(colonist_target, pos, colonist_count, jobType)

			pos += offset
			colonist_count += 1
			jobTypeCount += 1

	print("Colonists placed: ", colonist_count, " (", NUM_COLONISTS, " per corner)")

	# === Place colonists in all corner cells ===
	colonist_count = 0
	jobTypeCount = 0
	const CORNER_SIZE := 15

	# Corners (top-left, top-right, bottom-left, bottom-right)
	corners = [
		Vector2i(0, 0),
		Vector2i(world_width - CORNER_SIZE, 0),
		Vector2i(0, world_height - CORNER_SIZE),
		Vector2i(world_width - CORNER_SIZE, world_height - CORNER_SIZE)
	]

	# Center target
	target = Vector2i(half_width, half_height)

	for corner in corners:
		for y in range(CORNER_SIZE):
			for x in range(CORNER_SIZE):
				var pos : Vector2i = corner + Vector2i(x, y)

				var sprite_idx := colonist_count % NUM_COLONISTS
				$World.create_entity("building", pos, 2, 8)

				# Spread targets slightly
				var colonist_target = target + Vector2i(
					sprite_idx % 10,
					(sprite_idx / 10) % 10
				)

				var jobType := 1 if jobTypeCount % 2 == 0 else 2
				$World.create_temp_job(colonist_target, pos, colonist_count, jobType)

				colonist_count += 1
				jobTypeCount += 1

	# ────────────────────────────────────────────────
	# Assuming these exist / are defined earlier:
	# var world_width  : int
	# var world_height : int
	# var half_width   := world_width  / 2
	# var half_height  := world_height / 2
	# const GameSettings.chunk_size : int = 16 (example)
	# const NUM_COLONISTS : int = ... (number of colonist sprites)
	# ────────────────────────────────────────────────

	var max_chunk_x :int= (world_width  / GameSettings.chunk_size) - 1
	var max_chunk_y :int= (world_height / GameSettings.chunk_size) - 1

	# We now pick only **four corner chunks** (one per corner)
	var corner_chunks := [
		Vector2i(0, 0),                    # top-left
		Vector2i(max_chunk_x, 0),          # top-right
		Vector2i(0, max_chunk_y),          # bottom-left
		Vector2i(max_chunk_x, max_chunk_y) # bottom-right
	]

	const BUILDING_IDS := [24,26,28,32,33,34,35,36,37,40,41,42,43,44]

	colonist_count = 0
	jobTypeCount   = 0

	# Optional: different random seed per corner so they look different
	rng = RandomNumberGenerator.new()

	for corner_idx in range(corner_chunks.size()):
		rng.seed = corner_idx + 1000  # simple way to get different feel per corner

		var chunk :Vector2i= corner_chunks[corner_idx]

		# Clamp just to be 100% safe
		chunk.x = clamp(chunk.x, 0, max_chunk_x)
		chunk.y = clamp(chunk.y, 0, max_chunk_y)

		var origin := chunk * GameSettings.chunk_size

		for y in range(GameSettings.chunk_size):
			for x in range(GameSettings.chunk_size):
				var pos := origin + Vector2i(x, y)

				# Skip if somehow outside world (edge case protection)
				if pos.x >= world_width or pos.y >= world_height:
					continue

				# Decide randomly whether to place colonist or building
				# Adjust 0.15 to control colonist density (15% colonists here)
				if rng.randf() < 2:
					# ─── Colonist ───────────────────────────────────────
					var sprite_idx := colonist_count % NUM_COLONISTS

					$World.create_entity("colonist", pos, 1, sprite_idx)

					# Spread target around center
					var offset_x := (sprite_idx % 13) - 6
					var offset_y := ((sprite_idx / 13) % 9) - 4
					var target_pos := Vector2i(
					half_width  + offset_x,
					half_height + offset_y
					)
					target_pos.x = clamp(target_pos.x, 0, world_width - 1)
					target_pos.y = clamp(target_pos.y, 0, world_height - 1)

					var job_type := 1 if jobTypeCount % 2 == 0 else 2

					$World.create_temp_job(target_pos, pos, colonist_count, job_type)

					colonist_count += 1
					jobTypeCount   += 1

				else:
				# ─── Building ───────────────────────────────────────
					var building_id = BUILDING_IDS[rng.randi() % BUILDING_IDS.size()]
					$World.create_entity("building", pos, 2, building_id)



	# ===================================================================
	# END OF TEMPORARY GENERATION
	# ===================================================================

func _on_building_selected(sheet_id: int, variant_id: int) -> void:
	# unselect it now
	if sheet_id == selectedSprite[0] and variant_id == selectedSprite[1]:
		selectedSprite[0] = 0
		selectedSprite[1] = 0
		selectedSprite[2] = 0
		#print("Main received deselection → sheet: %d  variant: %d" % [sheet_id, variant_id])
	else:
		selectedSprite[0] = sheet_id
		selectedSprite[1] = variant_id
		selectedSprite[2] = 1
		#print("Main received selection → sheet: %d  variant: %d" % [sheet_id, variant_id])

func _update_place_ghost(sprite : AtlasTexture, c_scale : Vector2, offset : Vector2) -> void:
	if selectedSprite[0] == 0:
		selectedSprite[2] = 0
	$GameSystems/GridHighlight.update_selected_sprite_ghost(sprite, c_scale, offset, selectedSprite[2])



func _unhandled_input(event):
	if event is InputEventKey and event.keycode == KEY_P and event.pressed:
		print("Pause toggled")
		GameSettings.paused = !GameSettings.paused
		update_paused_icon()
	
	if event is InputEventKey and event.keycode == KEY_F7 and event.pressed:
		$World.toggle_track_entity_movement_per_second()

	if event is InputEventMouseButton and event.button_index == MOUSE_BUTTON_LEFT and event.pressed:
		if $UI/MasterControl/SelectionOverlay.is_active:
			return
		if selectedSprite[2]:
			#print("Placing sprite")
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
			elif selectedSprite[0] == 4:
				type = "harvestable"
			$World.create_entity(type, new_tile, selectedSprite[0], selectedSprite[1])
		else:
			var camera := get_viewport().get_camera_2d()
			if camera == null:
				print("No Camera2D found!")
				return
			
			var world_click_pos := camera.get_global_mouse_position()
			world_click_pos.y += 1
			var result = $World.get_entities_at_world_pos(world_click_pos)
			var count = result.get("count", 0)
			
			
			if count == 0:
				#print("No entities found at ", world_click_pos)
				return
			
			#print("Found %d entit(y/ies) at %s:" % [count, world_click_pos])

					
			# Show hide correct UI
			$UI/MasterControl/GameUI.visible=false
			$UI/MasterControl/EntityClickPopup.visible=true
			
			selectedEntity.entities_selected(result)


func create_entity_job(pos : Vector2i, entityPos : Vector2i, id : int, jobType : int):
	# For making an entity job -> Temporary
	$World.create_temp_job(pos, entityPos, id, jobType)


func update_paused_icon():
	if(GameSettings.paused):
		$UI/MasterControl/GameUI/PausePanel/Label.text = ">"
		print("Paused set to || is >")
	else:
		$UI/MasterControl/GameUI/PausePanel/Label.text = "||"
		print("Paused set to > is ||")
