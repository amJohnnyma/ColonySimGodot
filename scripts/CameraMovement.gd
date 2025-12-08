# CameraMovement.gd
extends Camera2D

@export var speed := 400.0   # pixels per second at zoom 1.0

func _process(delta: float) -> void:
	var input_vec := Vector2.ZERO
	
	if Input.is_key_pressed(KEY_A) or Input.is_key_pressed(KEY_LEFT):
		input_vec.x -= 1
	if Input.is_key_pressed(KEY_D) or Input.is_key_pressed(KEY_RIGHT):
		input_vec.x += 1
	if Input.is_key_pressed(KEY_W) or Input.is_key_pressed(KEY_UP):
		input_vec.y -= 1
	if Input.is_key_pressed(KEY_S) or Input.is_key_pressed(KEY_DOWN):
		input_vec.y += 1
	
	if input_vec != Vector2.ZERO:
		input_vec = input_vec.normalized()
		global_position += input_vec * speed * delta * zoom.x   # scale by zoom!
