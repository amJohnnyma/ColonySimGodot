extends Control

# Signals for button presses
signal main_button_pressed(button_id: int, panel_id: int)
# Signal for panel state changes
signal panel_toggled(is_visible: bool, panel_id : int)

#build menu signals
signal build_category_button_pressed(button_id : int)

signal building_selected(sheet_id: int, variant_id: int)
signal update_place_ghost(atlas_texture: AtlasTexture, scale : Vector2, offset : Vector2)

# References to UI elements
@onready var main_action_container = $MainActionContainer
@onready var expanded_panel_build = $BuildExpandedPanel
@onready var expanded_panel_job = $JobExpandedPanel
@onready var expanded_panel_diplomacy = $DiplomacyExpandedPanel

# Build specific UI elements
@onready var build_grid_options = $BuildExpandedPanel/ScrollContainer/GridContainer
@onready var build_scroll_container = $BuildExpandedPanel/ScrollContainer
var currentShownGrid : int = -1

func _ready():
	# Set mouse filter for MasterUI
	mouse_filter = MOUSE_FILTER_IGNORE
	
	# Connect button signals
	$MainActionContainer/BuildExpandButton.pressed.connect(_on_main_button_pressed.bind(1,1))
	$MainActionContainer/JobExpandButton.pressed.connect(_on_main_button_pressed.bind(2,2))
	$MainActionContainer/DiplomacyExpandButton.pressed.connect(_on_main_button_pressed.bind(3,3))
	
	$BuildExpandedPanel/CloseBuildExpandedButton.pressed.connect(_on_close_panel.bind(1))
	$JobExpandedPanel/CloseJobExpandedButton.pressed.connect(_on_close_panel.bind(2))
	$DiplomacyExpandedPanel/CloseDiplomacyExpandedButton.pressed.connect(_on_close_panel.bind(3))
	
	#build buttons
	$BuildExpandedPanel/BuildCategories/Cat1Button.pressed.connect(_on_build_category_button_pressed.bind(1))
	$BuildExpandedPanel/BuildCategories/Cat2Button.pressed.connect(_on_build_category_button_pressed.bind(2))
	$BuildExpandedPanel/BuildCategories/Cat3Button.pressed.connect(_on_build_category_button_pressed.bind(3))
	$BuildExpandedPanel/BuildCategories/Cat4Button.pressed.connect(_on_build_category_button_pressed.bind(4))


	
	# Initially hide the panel
	expanded_panel_build.visible = false
	expanded_panel_job.visible = false
	expanded_panel_diplomacy.visible = false

func _unhandled_input(event: InputEvent) -> void:
	if event.is_action_pressed("toggle_spritetester"):  # Bind to F9 or whatever
		$SpriteSheetTester.toggle_visible()

func _on_main_button_pressed(button_id: int, panel_id : int):
	match button_id:
		1:
			# Hide buttons
			main_action_container.visible = false
			# Show panel
			expanded_panel_build.visible = true
			# Clear the grid
			clear_grid(build_grid_options)
			# Emit signals
			main_button_pressed.emit(button_id)
			panel_toggled.emit(true, panel_id)
			pass
		2:
			main_action_container.visible = false
			# Show panel
			expanded_panel_job.visible = true
			# Emit signals
			main_button_pressed.emit(button_id,panel_id)
			panel_toggled.emit(true)
			pass
		3:
			main_action_container.visible = false
			# Show panel
			expanded_panel_diplomacy.visible = true
			# Emit signals
			main_button_pressed.emit(button_id,panel_id)
			panel_toggled.emit(true)
			pass

func _on_close_panel(panel_id: int):
	match panel_id:
		1:
			# Hide buttons
			main_action_container.visible = true
			# Show panel
			expanded_panel_build.visible = false
			clear_grid(build_grid_options)
			# Emit signals
			panel_toggled.emit(false, panel_id)
			pass
		2:
			main_action_container.visible = true
			# Show panel
			expanded_panel_job.visible = false
			# Emit signals
			panel_toggled.emit(false, panel_id)
			pass
		3:
		# Action for Button3
			main_action_container.visible = true
			# Show panel
			expanded_panel_diplomacy.visible = false
			# Emit signals
			panel_toggled.emit(false, panel_id)
			pass

func _on_build_category_button_pressed(button_id: int):
	match button_id:
		1:
			if currentShownGrid == button_id:
				clear_grid(build_grid_options)
				currentShownGrid = -1
			else:
				fill_grid_with_sprites(build_grid_options, button_id)
				build_category_button_pressed.emit(button_id)
			pass
		2:
			if currentShownGrid == button_id:
				clear_grid(build_grid_options)
				currentShownGrid = -1
			else:
				fill_grid_with_sprites(build_grid_options, button_id)
				build_category_button_pressed.emit(button_id)
			pass
		3:
			if currentShownGrid == button_id:
				clear_grid(build_grid_options)
				currentShownGrid = -1
			else:
				fill_grid_with_sprites(build_grid_options, button_id)
				build_category_button_pressed.emit(button_id)
			pass
		4:
			if currentShownGrid == button_id:
				clear_grid(build_grid_options)
				currentShownGrid = -1
			else:
				fill_grid_with_sprites(build_grid_options, button_id)
				build_category_button_pressed.emit(button_id)
			pass



func fill_grid_with_sprites(grid: GridContainer, button_id: int = -1) -> void:
	clear_grid(grid)
	build_scroll_container.visible = true
	currentShownGrid = button_id

	# Protect against invalid button_id / sheet
	if button_id <= 0 or button_id > SpriteAtlas.sheet_regions.size():
		print("Invalid sheet id:", button_id)
		return

	var sheet_id := button_id
	# If your sheets are already 0-based in the atlas → remove the -1

	# How many variants/sprites exist on this sheet
	var variant_count: int = SpriteAtlas.sheet_regions[sheet_id].size()
	if variant_count <= 0:
		print("Sheet", sheet_id, "has no regions defined")
		return

	var full_sheet_texture: Texture2D = SpriteAtlas.get_texture(sheet_id)

	for i in variant_count:
		var variant_id: int = i
		var region: Rect2 = SpriteAtlas.get_region(sheet_id, variant_id)

		if region.size == Vector2.ZERO:
			continue  # skip invalid/empty regions



		var sprite := TextureRect.new()
		sprite.modulate = Color(1, 1, 1, 1)         
		sprite.self_modulate = Color(1, 1, 1, 1)
		sprite.name = "Sprite_%d_var%d" % [sheet_id, variant_id]
		
		
		var atlas_tex := AtlasTexture.new()
		atlas_tex.atlas  = full_sheet_texture
		atlas_tex.region = region

		sprite.texture = atlas_tex

		# UI-friendly display settings
		sprite.stretch_mode = TextureRect.STRETCH_KEEP_ASPECT_CENTERED
		sprite.expand_mode  = TextureRect.EXPAND_FIT_WIDTH_PROPORTIONAL
		# Good alternatives: EXPAND_KEEP_SIZE, EXPAND_FIT_HEIGHT_PROPORTIONAL

		sprite.custom_minimum_size = Vector2(64, 64)  # ← tune this to your GridContainer cell size

		# Make clickable
		sprite.mouse_filter = Control.MOUSE_FILTER_STOP
		sprite.mouse_default_cursor_shape = Control.CURSOR_POINTING_HAND
		sprite.gui_input.connect(_on_sprite_gui_input.bind(i, sheet_id, variant_id))
		

		grid.add_child(sprite)

	# Optional: force layout refresh
	grid.queue_sort()


func _on_sprite_gui_input(event: InputEvent, index: int, sheet_id: int, variant_id : int) -> void:
	if event is InputEventMouseButton and event.button_index == MOUSE_BUTTON_LEFT and event.pressed:
		print("Clicked sprite %d → sheet %d variant %d" % [index, sheet_id, variant_id])
		building_selected.emit(sheet_id, variant_id)
		var full_tex = SpriteAtlas.get_texture(sheet_id)
		var region  = SpriteAtlas.get_region(sheet_id, variant_id)
		var scale_ghost = SpriteAtlas.get_scale(sheet_id)
		var offset = SpriteAtlas.get_offset(sheet_id, variant_id)

		var ghost_atlas = AtlasTexture.new()
		ghost_atlas.atlas = full_tex
		ghost_atlas.region = region
		update_place_ghost.emit(ghost_atlas, scale_ghost, offset)


		
func clear_grid(grid: GridContainer):
	for child in grid.get_children():
		child.queue_free()  # Safely removes and frees the node
		update_place_ghost.emit(null, Vector2.ZERO ,Vector2.ZERO)
	build_scroll_container.visible = false
