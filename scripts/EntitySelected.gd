extends Control

signal create_job_panel()
signal create_job()

@export var main: Node

# Selected entity data
var selected_entity_pos: Vector2i
var selected_entity_id: int
var selected_job_type: int

# Main UI references
@onready var tab_container: TabContainer = $MarginContainer/Panel/TabContainer

# Common elements (visible on all tabs)
@onready var icon_texture: TextureRect = $MarginContainer/Panel/IconMargin/Icon/TextureRect
@onready var joblist_button: Button = $MarginContainer/Panel/TabContainer/EntityJobPanel/JoblistButtonMargin/Button
@onready var job_create_panel: Control = $MarginContainer/Panel/TabContainer/EntityJobPanel/JobCreatePanel

# References to panels inside the TabContainer
@onready var info_panel: Control = tab_container.get_node("EntityJobPanel/JoblistMargin/Joblist")
@onready var inventory_panel: Control = tab_container.get_node("InventoryPanel")
@onready var jobs_panel: Control = tab_container.get_node("EntityJobPanel")

@onready var menu_popup: PopupMenu = job_create_panel.get_node("VBoxContainer/MenuButton").get_popup()
@onready var selected_job_label: Label = job_create_panel.get_node("VBoxContainer/SelectedJobLabel")

func _ready() -> void:
	print_tree_pretty()
	# Connect button signals
	joblist_button.pressed.connect(_create_job_panel)
	job_create_panel.get_node("VBoxContainer/Button").pressed.connect(_create_job)
	
	# Set default tab
	tab_container.current_tab = 0


func entities_selected(result: Dictionary) -> void:
	if result.get("count", 0) == 0:
		push_warning("entities_selected called with no entity data")
		return
	
	GameSettings.paused = true
	
	# Extract entity data
	var entity_id: int = result["entity_ids"][0]
	var entity_type: int = result["types"][0]
	var entity_sprite: int = result["entity_sprites"][0]
	
	selected_entity_pos = Vector2i(result["x_pos"][0], result["y_pos"][0])
	selected_entity_id = entity_id
	
	# Update common UI
	_update_entity_icon(entity_type, entity_sprite)
	_update_info_panel(result, entity_id)          # Info panel gets the text
	_setup_action_menu(entity_type)
	
	# Reset UI state
	joblist_button.visible = false
	job_create_panel.visible = true
	
	# Switch to first tab (Info)
	tab_container.current_tab = 0


func _update_entity_icon(entity_type: int, sprite_id: int) -> void:
	var full_tex = SpriteAtlas.get_texture(entity_type)
	var region = SpriteAtlas.get_region(entity_type, sprite_id)
	var atlas_tex = AtlasTexture.new()
	atlas_tex.atlas = full_tex
	atlas_tex.region = region
	icon_texture.texture = atlas_tex


func _update_info_panel(result: Dictionary, entity_id: int) -> void:
	var text = str(entity_id) + "\n" + str(selected_entity_pos) + "\n\nInventory:\n"
	
	var inv_types = result.get("inv_types", PackedInt32Array())
	var inv_ids = result.get("inv_ids", PackedInt32Array())
	var inv_counts = result.get("inv_counts", PackedInt32Array())
	
	if inv_types.size() > 0:
		for i in inv_types.size():
			text += "• Type %d, ID %d: %d\n" % [inv_types[i], inv_ids[i], inv_counts[i]]
	else:
		text += "(Empty)\n"
	
	# Update the label inside InfoPanel
	if info_panel.has_node("Label"):
		info_panel.get_node("Label").text = text


func _setup_action_menu(entity_type: int) -> void:
	menu_popup.clear()
	if not menu_popup.id_pressed.is_connected(_on_popup_id_pressed):
		menu_popup.id_pressed.connect(_on_popup_id_pressed)
	
	match entity_type:
		1: # Colonist
			menu_popup.add_item("Move To Position", 1)
			menu_popup.add_item("Harvest", 2)
		2:
			menu_popup.add_item("Option C", 1)
			menu_popup.add_item("Option D", 2)
		4: # Harvestable
			menu_popup.add_item("Harvest", 2)
		_:
			menu_popup.add_item("Inspect", 0)




func _on_popup_id_pressed(id: int) -> void:
	selected_job_type = id
	# Map ID to a human-readable name
	var job_names = {
		0: "Inspect",
		1: "Move To Position",
		2: "Harvest"
	}
	var display_name = job_names.get(id, "Unknown (%d)" % id)
	selected_job_label.text = "Selected: " + display_name
	print("Selected job type: ", id)

func _create_job_panel() -> void:
	joblist_button.visible = false
	job_create_panel.visible = true
	create_job_panel.emit()


func _create_job() -> void:
	var x_text = job_create_panel.get_node("VBoxContainer/HBoxContainer/TextEdit").text
	var y_text = job_create_panel.get_node("VBoxContainer/HBoxContainer2/TextEdit").text
	
	var target_pos = Vector2i(int(x_text), int(y_text))
	
	main.create_entity_job(target_pos, selected_entity_pos, selected_entity_id, selected_job_type)
	create_job.emit()
