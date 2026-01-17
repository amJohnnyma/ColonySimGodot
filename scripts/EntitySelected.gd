extends Control

signal create_job_panel()
signal create_job()

@export var main: Node

var selectedEntityPos : Vector2i
var selectedEntityID : int

func _ready() -> void:
	$MarginContainer/Panel/EntityJobPanel/JoblistButtonMargin/Button.pressed.connect(_create_job_panel)
	$MarginContainer/Panel/EntityJobPanel/JobCreatePanel/VBoxContainer/Button.pressed.connect(_create_job)


func entities_selected(ids : Array, types : Array, sprites : Array, epos : Array[Vector2i]) -> void:
	# Hide what mustnt be seen
	$MarginContainer/Panel/EntityJobPanel/JoblistButtonMargin/Button.visible = true
	$MarginContainer/Panel/EntityJobPanel/JobCreatePanel.visible = false
	GameSettings.paused = true
	
	# Populate the UI with the first entity for now
	# NBNBNBNB Check the type to know which tabs to show
	var id = ids[0]
	var type = types[0]
	var sprite = sprites[0]
	var pos = epos[0]
	selectedEntityPos = pos
	selectedEntityID = id
	
	# Make a sprite the Icon
	var full_tex = SpriteAtlas.get_texture(type)
	var region  = SpriteAtlas.get_region(type, sprite)


	var atlas_tex = AtlasTexture.new()
	atlas_tex.atlas = full_tex
	atlas_tex.region = region
	
	$MarginContainer/Panel/EntityJobPanel/IconMargin/Icon/TextureRect.texture = atlas_tex

	var tempLabelString = ""
	tempLabelString += str(id) + "\n" + str(pos)
	# Show the ID and pos in the box below
	$MarginContainer/Panel/EntityJobPanel/JoblistMargin/Joblist/Label.text = tempLabelString


func _create_job_panel():
	print("Create job")
	# Show the UI for the job
	$MarginContainer/Panel/EntityJobPanel/JoblistButtonMargin/Button.visible = false
	$MarginContainer/Panel/EntityJobPanel/JobCreatePanel.visible = true

	create_job_panel.emit()

func _create_job():
	
	var pos_x = int($MarginContainer/Panel/EntityJobPanel/JobCreatePanel/VBoxContainer/HBoxContainer/TextEdit.text)
	var pos_y = int($MarginContainer/Panel/EntityJobPanel/JobCreatePanel/VBoxContainer/HBoxContainer2/TextEdit.text)

	print(pos_x, pos_y)
	
	main.create_entity_job(Vector2i(pos_x, pos_y), selectedEntityPos, selectedEntityID)
	create_job.emit()
