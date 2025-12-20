extends Control

@export var button_1: BaseButton
@export var button_2: BaseButton
@export var button_3: BaseButton
@export var button_4: BaseButton

@export var type_1: SpriteAtlas.EntityType = SpriteAtlas.EntityType.BUSH
@export var type_2: SpriteAtlas.EntityType = SpriteAtlas.EntityType.TILE
@export var type_3: SpriteAtlas.EntityType = SpriteAtlas.EntityType.PLAYER_FRONT
@export var type_4: SpriteAtlas.EntityType = SpriteAtlas.EntityType.PLAYER_CLIMB_BACK

var selected_button: BaseButton = null
var selected_type: SpriteAtlas.EntityType = -1

# Only emit selection, NOT placement
signal entity_type_selected(type: SpriteAtlas.EntityType)

func _ready() -> void:
	if not button_1 or not button_2 or not button_3 or not button_4:
		push_error("One or more buttons not assigned!")
		return
	
	button_1.pressed.connect(_on_button_pressed.bind(button_1, type_1))
	button_2.pressed.connect(_on_button_pressed.bind(button_2, type_2))
	button_3.pressed.connect(_on_button_pressed.bind(button_3, type_3))
	button_4.pressed.connect(_on_button_pressed.bind(button_4, type_4))

func _on_button_pressed(button: BaseButton, entity_type: SpriteAtlas.EntityType) -> void:
	if selected_button and selected_button != button:
		_reset_visual(selected_button)
	
	selected_button = button
	selected_type = entity_type
	_highlight_visual(button)
	
	entity_type_selected.emit(entity_type)
	print("Selected for placement: ", SpriteAtlas.EntityType.keys()[entity_type])

func _highlight_visual(btn: BaseButton) -> void:
	btn.modulate = Color(1.4, 1.4, 1.0)

func _reset_visual(btn: BaseButton) -> void:
	btn.modulate = Color.WHITE

func deselect_all() -> void:
	if selected_button:
		_reset_visual(selected_button)
		selected_button = null
		selected_type = -1
