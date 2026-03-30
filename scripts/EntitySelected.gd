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

@onready var inventory_grid: GridContainer = tab_container.get_node("InventoryPanel/Inventory")
# Store inventory data for use across tabs
var _inv_types: PackedInt32Array
var _inv_ids: PackedInt32Array
var _inv_counts: PackedInt32Array

@onready var menu_popup: PopupMenu = job_create_panel.get_node("VBoxContainer/MenuButton").get_popup()
@onready var selected_job_label: Label = job_create_panel.get_node("VBoxContainer/SelectedJobLabel")

@export var selection_overlay: Control 
@export var selection_hud: CanvasLayer
func _ready() -> void:
	
	# Connect button signals
	joblist_button.pressed.connect(_create_job_panel)
	job_create_panel.get_node("VBoxContainer/Button").pressed.connect(_create_job)
	
	job_create_panel.get_node("VBoxContainer/SelectAreaButton").pressed.connect(_activate_area_selection)

	selection_overlay.area_confirmed.connect(_on_area_confirmed)
	selection_overlay.area_cancelled.connect(_on_area_cancelled)
	selection_hud.confirmed.connect(selection_overlay.confirm)
	selection_hud.cancelled.connect(selection_overlay.cancel)
	
	# Set default tab
	tab_container.current_tab = 0

func _activate_area_selection() -> void:
	pass
	#visible = false
	#selection_overlay.visible = true
	#selection_hud.visible = true
	#selection_overlay.activate()
	#selection_hud.activate()

func _on_area_confirmed(rects: Array) -> void:
	selection_hud.deactivate()
	selection_hud.visible = false
	# Each rect is independent — one job per tile per rect
	for rect in rects:
		for x in rect.size.x:
			for y in rect.size.y:
				var tile_pos : Vector2i
				tile_pos = rect.position + Vector2i(x, y)
				main.create_entity_job(tile_pos, selected_entity_pos, selected_entity_id, selected_job_type)
	create_job.emit()

func _on_area_cancelled() -> void:
	selection_hud.deactivate()
	visible = true  # reopen the popup
	
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
	
	# Cache inventory for grid population
	_inv_types  = result.get("inv_types",PackedInt32Array())
	_inv_ids    = result.get("inv_ids",PackedInt32Array())
	_inv_counts = result.get("inv_counts",PackedInt32Array())
	
	# Update common UI
	_update_info_panel(entity_id)
	_fill_inventory_grid()   # populate immediately so it's ready when tab is switched
	_update_entity_icon(entity_type, entity_sprite)
	_setup_action_menu(entity_type)
	
	# Reset UI state
	joblist_button.visible = false
	job_create_panel.visible = true
	
	# Switch to first tab (Info)
	tab_container.current_tab = 0

func _fill_inventory_grid() -> void:
	for child in inventory_grid.get_children():
		child.queue_free()

	if _inv_types.size() == 0:
		var empty_label := Label.new()
		empty_label.text = "Empty"
		inventory_grid.add_child(empty_label)
		return

	# 3 strips x 10 rows = 30 slots, 6 columns (icon+count per strip)
	const STRIPS    := 3
	const ROWS      := 10
	const COLS      := STRIPS * 2   # 6
	const TOTAL_SLOTS := STRIPS * ROWS  # 30

	inventory_grid.columns = COLS

	# Pre-fill all 30 slots as empty so the grid always has a fixed shape
	var cells: Array = []
	for i in TOTAL_SLOTS:
		cells.append(null)  # placeholder

	# Map inventory items into slots
	# Slot index fills column-strip order: strip 0 rows 0-9, strip 1 rows 0-9, strip 2 rows 0-9
	for i in _inv_types.size():
		if i >= TOTAL_SLOTS:
			break
		# Which strip and row this item lives in
		var strip : int = i / ROWS
		var row   : int = i % ROWS
		var slot  : int = row * STRIPS + strip   # maps to grid insertion order
		cells[slot] = i  # store the inventory index

	# Now build the grid in row-major order (GridContainer fills left→right, top→bottom)
	for slot in TOTAL_SLOTS:
		var inv_idx = cells[slot]

		# --- Icon cell ---
		var icon_cell := PanelContainer.new()
		icon_cell.custom_minimum_size = Vector2(40, 40)

		if inv_idx != null:
			var item_type  : int = _inv_types[inv_idx]
			var variant_id : int = _inv_ids[inv_idx]
			var region     : Rect2 = SpriteAtlas.get_region(item_type, variant_id)

			if region.size != Vector2.ZERO:
				var atlas_tex        := AtlasTexture.new()
				atlas_tex.atlas       = SpriteAtlas.get_texture(item_type)
				atlas_tex.region      = region

				var sprite := TextureRect.new()
				sprite.texture      = atlas_tex
				sprite.stretch_mode = TextureRect.STRETCH_KEEP_ASPECT_CENTERED
				sprite.expand_mode  = TextureRect.EXPAND_FIT_WIDTH_PROPORTIONAL
				sprite.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
				icon_cell.add_child(sprite)

			icon_cell.tooltip_text = "Type %d | ID %d" % [item_type, variant_id]

		inventory_grid.add_child(icon_cell)

		# --- Count cell ---
		var count_cell := Label.new()
		count_cell.custom_minimum_size = Vector2(24, 40)
		count_cell.vertical_alignment  = VERTICAL_ALIGNMENT_CENTER

		if inv_idx != null:
			count_cell.text = "×%d" % _inv_counts[inv_idx]
		else:
			count_cell.text = ""

		inventory_grid.add_child(count_cell)

	inventory_grid.queue_sort()


func _update_entity_icon(entity_type: int, sprite_id: int) -> void:
	var full_tex = SpriteAtlas.get_texture(entity_type)
	var region = SpriteAtlas.get_region(entity_type, sprite_id)
	var atlas_tex = AtlasTexture.new()
	atlas_tex.atlas = full_tex
	atlas_tex.region = region
	icon_texture.texture = atlas_tex


func _update_info_panel(entity_id: int) -> void:
	var text = str(entity_id) + "\n" + str(selected_entity_pos)
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
	#print("Selected job type: ", id)

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
