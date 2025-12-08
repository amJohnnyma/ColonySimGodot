extends Camera2D

@export var move_speed := 400.0
@export var zoom_speed := 3.0         # how quickly zoom interpolates
@export var zoom_step := 0.1          # each scroll/key press changes zoom by this
@export var min_zoom := 0.2
@export var max_zoom := 4.0

var target_zoom: float = 1.0          # store zoom as a single number!


func _ready():
	target_zoom = zoom.x              # assume zoom.x == zoom.y


func _process(delta: float) -> void:
	handle_movement(delta)
	handle_zoom(delta)


func handle_movement(delta: float) -> void:
	var dir := Vector2.ZERO

	if Input.is_key_pressed(KEY_A) or Input.is_key_pressed(KEY_LEFT):
		dir.x -= 1
	if Input.is_key_pressed(KEY_D) or Input.is_key_pressed(KEY_RIGHT):
		dir.x += 1
	if Input.is_key_pressed(KEY_W) or Input.is_key_pressed(KEY_UP):
		dir.y -= 1
	if Input.is_key_pressed(KEY_S) or Input.is_key_pressed(KEY_DOWN):
		dir.y += 1

	if dir != Vector2.ZERO:
		dir = dir.normalized()
		global_position += dir * move_speed * delta * zoom.x  # scale by current zoom


func handle_zoom(delta: float) -> void:
	# Zoom IN
	if Input.is_key_pressed(KEY_E):          # or Up, or Mouse wheel up
		target_zoom -= zoom_step

	# Zoom OUT
	if Input.is_key_pressed(KEY_Q):          # or Down, or Mouse wheel down
		target_zoom += zoom_step

	# Clamp zoom
	target_zoom = clamp(target_zoom, min_zoom, max_zoom)

	# Smooth interpolation
	zoom = zoom.lerp(Vector2(target_zoom, target_zoom), 1.0 - pow(0.001, delta))
