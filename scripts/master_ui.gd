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
				fill_grid_with_placeholders(build_grid_options, 40, button_id)
				build_category_button_pressed.emit(button_id)
			pass
		2:
			if currentShownGrid == button_id:
				clear_grid(build_grid_options)
				currentShownGrid = -1
			else:
				fill_grid_with_placeholders(build_grid_options, 40, button_id)
				build_category_button_pressed.emit(button_id)
			pass
		3:
			if currentShownGrid == button_id:
				clear_grid(build_grid_options)
				currentShownGrid = -1
			else:
				fill_grid_with_placeholders(build_grid_options, 40, button_id)
				build_category_button_pressed.emit(button_id)
			pass
		4:
			if currentShownGrid == button_id:
				clear_grid(build_grid_options)
				currentShownGrid = -1
			else:
				fill_grid_with_placeholders(build_grid_options, 40, button_id)
				build_category_button_pressed.emit(button_id)
			pass


''''''

# Function to fill the grid with placeholders
func fill_grid_with_placeholders(grid: GridContainer, count: int = 20,  button_id : int = -1):
	# First clear any existing children (in case you refill)
	clear_grid(grid)
	build_scroll_container.visible = true
	currentShownGrid = button_id
	
	
	for i in range(count):
		var placeholder: Control

		placeholder = Panel.new()
		placeholder.custom_minimum_size = Vector2(50, 50)  # Adjust size as needed
		#placeholder.size = Vector2(50,50)
		placeholder.modulate = Color(0.3, 0.3, 0.3, 0.8)   # Gray semi-transparent
		
		# Create the button
		var button = Button.new()
		button.text = "Click Me"  # Optional: set button text
		button.custom_minimum_size = Vector2(50, 50)  # Adjust size as needed
		#button.size = Vector2(50,50)

		# Create a custom StyleBoxFlat for the normal state (background)
		var normal_style = StyleBoxFlat.new()
		normal_style.bg_color = Color(0.2, 0.6, 0.8, 1)  # Example: blue-ish background
		normal_style.corner_radius_top_left = 10
		normal_style.corner_radius_top_right = 10
		normal_style.corner_radius_bottom_right = 10
		normal_style.corner_radius_bottom_left = 10
		normal_style.border_color = Color(0.1, 0.4, 0.6, 1)

		# Optional: hover style for better feedback
		var hover_style = StyleBoxFlat.new()
		hover_style.bg_color = Color(0.3, 0.7, 0.9, 1)  # Lighter on hover

		# Apply the styles
		button.add_theme_stylebox_override("normal", normal_style)
		button.add_theme_stylebox_override("hover", hover_style)

		# Optional: pressed and focus styles (copy and adjust as needed)
		button.add_theme_stylebox_override("pressed", normal_style.duplicate())
		button.add_theme_stylebox_override("focus", normal_style.duplicate())
		

		# Add the button to the placeholder panel (or wherever you need it)
		placeholder.add_child(button)

		# Optional: give it a name for debugging
		placeholder.name = "Placeholder_%d" % i
		
		# Add to grid
		grid.add_child(placeholder)


func clear_grid(grid: GridContainer):
	for child in grid.get_children():
		child.queue_free()  # Safely removes and frees the node
	build_scroll_container.visible = false
