extends Control

func _ready() -> void:
	$Panel/BackButton.pressed.connect(_on_back_pressed)
	$Panel/VBoxContainer/HBoxContainer/CreateButton.pressed.connect(_on_create_pressed)

func _on_back_pressed():
	get_tree().change_scene_to_file("res://scenes/main_menu.tscn")


func _on_create_pressed():
	pass
