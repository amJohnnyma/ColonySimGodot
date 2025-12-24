extends Control


# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	$TextureRect/MarginContainer/MenuButtons/ContinueButton.pressed.connect(_on_continue_pressed)
	$TextureRect/MarginContainer/MenuButtons/NewButton.pressed.connect(_on_new_pressed)
	$TextureRect/MarginContainer/MenuButtons/SettingsButton.pressed.connect(_on_settings_pressed)
	$TextureRect/MarginContainer/MenuButtons/ExitButton.pressed.connect(_on_exit_pressed)



func _on_continue_pressed():
	get_tree().change_scene_to_file("res://scenes/MainGame.tscn")
	
func _on_new_pressed():
	pass
	
func _on_settings_pressed():
	pass
	
func _on_exit_pressed():
	get_tree().quit()
