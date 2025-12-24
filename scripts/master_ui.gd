extends Control

# Signals for button presses
signal main_button_pressed(button_id: int, panel_id: int)
# Signal for panel state changes
signal panel_toggled(is_visible: bool, panel_id : int)

#build menu signals
signal build_category_button_pressed(button_id : int)

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
				fill_grid_with_placeholders(build_grid_options, 10, "Panel", button_id)
				build_category_button_pressed.emit(button_id)
			pass
		2:
			if currentShownGrid == button_id:
				clear_grid(build_grid_options)
				currentShownGrid = -1
			else:
				fill_grid_with_placeholders(build_grid_options, 20, "Label", button_id)
				build_category_button_pressed.emit(button_id)
			pass
		3:
			if currentShownGrid == button_id:
				clear_grid(build_grid_options)
				currentShownGrid = -1
			else:
				fill_grid_with_placeholders(build_grid_options, 30, "Button", button_id)
				build_category_button_pressed.emit(button_id)
			pass
		4:
			if currentShownGrid == button_id:
				clear_grid(build_grid_options)
				currentShownGrid = -1
			else:
				fill_grid_with_placeholders(build_grid_options, 40, "TextureRect", button_id)
				build_category_button_pressed.emit(button_id)
			pass


''''''

# Function to fill the grid with placeholders
func fill_grid_with_placeholders(grid: GridContainer, count: int = 20, placeholder_type: String = "Panel", button_id : int = -1):
	# First clear any existing children (in case you refill)
	clear_grid(grid)
	build_scroll_container.visible = true
	currentShownGrid = button_id
	
	
	for i in range(count):
		var placeholder: Control
		
		match placeholder_type:
			"Panel":
				placeholder = Panel.new()
				placeholder.custom_minimum_size = Vector2(100, 100)  # Adjust size as needed
				placeholder.modulate = Color(0.3, 0.3, 0.3, 0.8)   # Gray semi-transparent
			"Label":
				placeholder = Label.new()
				placeholder.text = "Item %d" % (i + 1)
				placeholder.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
				placeholder.vertical_alignment = VERTICAL_ALIGNMENT_CENTER
				placeholder.custom_minimum_size = Vector2(100, 100)
			"Button":
				placeholder = Button.new()
				placeholder.text = "Btn %d" % (i + 1)
				placeholder.custom_minimum_size = Vector2(100, 100)
			"TextureRect":
				placeholder = TextureRect.new()
				placeholder.texture = preload("res://icon.svg")  # Use Godot icon or your placeholder image
				placeholder.expand_mode = TextureRect.EXPAND_FIT_WIDTH_PROPORTIONAL
				placeholder.stretch_mode = TextureRect.STRETCH_KEEP_ASPECT_CENTERED
				placeholder.custom_minimum_size = Vector2(100, 100)
			_:
				placeholder = Panel.new()  # fallback
		
		# Optional: give it a name for debugging
		placeholder.name = "Placeholder_%d" % i
		
		# Add to grid
		grid.add_child(placeholder)


func clear_grid(grid: GridContainer):
	for child in grid.get_children():
		child.queue_free()  # Safely removes and frees the node
	build_scroll_container.visible = false
