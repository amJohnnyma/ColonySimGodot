extends Camera2D

@export var move_speed_base: float = 400.0    # Base speed at zoom=1.0
@export var zoom_speed: float = 0.15          # Zoom multiplier per wheel tick
@export var zoom_smoothness: float = 8.0      # How fast zoom interpolates (higher = snappier)
@export var move_smoothness: float = 10.0     # How fast movement interpolates (higher = snappier)
@export var min_zoom: float = 0.05            # Maximum zoom out
@export var max_zoom: float = 8.0             # Maximum zoom in

var target_zoom: float = 1.0
var target_position: Vector2 = Vector2.ZERO

func _ready() -> void:
	target_zoom = zoom.x

func _process(delta: float) -> void:
	handle_movement(delta)
	handle_zoom(delta)
	
	# Smooth interpolation
	global_position = global_position.lerp(target_position, move_smoothness * delta)
	zoom = zoom.lerp(Vector2(target_zoom, target_zoom), zoom_smoothness * delta)

func handle_movement(delta: float) -> void:
	var input_dir := Vector2.ZERO
	
	# WASD + Arrow keys
	if Input.is_key_pressed(KEY_A) or Input.is_key_pressed(KEY_LEFT):
		input_dir.x -= 1
	if Input.is_key_pressed(KEY_D) or Input.is_key_pressed(KEY_RIGHT):
		input_dir.x += 1
	if Input.is_key_pressed(KEY_W) or Input.is_key_pressed(KEY_UP):
		input_dir.y -= 1
	if Input.is_key_pressed(KEY_S) or Input.is_key_pressed(KEY_DOWN):
		input_dir.y += 1
	
	if input_dir != Vector2.ZERO:
		input_dir = input_dir.normalized()
		
		# SPEED SCALES INVERSELY WITH ZOOM (far away = faster, close = slower)
		var speed_multiplier = 1.0 / zoom.x  # Zoom out = faster movement
		var current_speed = move_speed_base * speed_multiplier
		
		# Apply movement to target position
		target_position += input_dir * current_speed * delta

func handle_zoom(delta: float) -> void:

	
	# KEYBOARD ZOOM (Q/E or +/-)
	if Input.is_key_pressed(KEY_Q) or Input.is_key_pressed(KEY_MINUS):
		target_zoom *= (1.0 + zoom_speed * delta * 10.0)  # Continuous zoom out
	if Input.is_key_pressed(KEY_E) or Input.is_key_pressed(KEY_EQUAL):
		target_zoom *= (1.0 - zoom_speed * delta * 10.0)  # Continuous zoom in
	
	# CLAMP ZOOM LIMITS
	target_zoom = clamp(target_zoom, min_zoom, max_zoom)

# MOUSE WHEEL INPUT (add these to your Input Map)
# Project Settings → Input Map → Add "zoom_in" and "zoom_out"
# Mouse wheel up → zoom_in
# Mouse wheel down → zoom_out
