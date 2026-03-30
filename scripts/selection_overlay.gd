# SelectionOverlay.gd
extends Control

signal area_confirmed(rects: Array)  # Array of Rect2i, one per drag
signal area_cancelled

@export var camera: Camera2D  # assign in inspector

var _dragging := false
var _start_world := Vector2.ZERO
var _current_world := Vector2.ZERO
var _committed_rects: Array[Rect2i] = []  # all completed drags
var _active_rect: Rect2i               # the one currently being drawn

var is_active := false

func activate() -> void:
	is_active = true
	_committed_rects.clear()
	_dragging = false
	visible = true
	mouse_filter = Control.MOUSE_FILTER_STOP

func deactivate() -> void:
	is_active = false
	visible = false
	mouse_filter = Control.MOUSE_FILTER_IGNORE  # clicks pass through again
	_dragging = false
	_committed_rects.clear()

func _gui_input(event: InputEvent) -> void:
	if event is InputEventMouseButton and event.button_index == MOUSE_BUTTON_LEFT:
		if event.pressed:
			_dragging = true
			_start_world = camera.get_viewport().get_camera_2d().get_global_mouse_position()
			_current_world = _start_world
		else:
			if _dragging:
				_dragging = false
				var rect := _world_to_tile_rect(_start_world, _current_world)
				_committed_rects.append(rect)
				queue_redraw()

	elif event is InputEventMouseMotion and _dragging:
		_current_world = camera.get_global_mouse_position()
		queue_redraw()

func _draw() -> void:
	var tile_size := GameSettings.tile_size

	# Draw all committed rects
	for rect in _committed_rects:
		var px_pos  := Vector2(rect.position) * tile_size - camera.get_screen_center_position() + get_viewport_rect().size / 2
		var px_size := Vector2(rect.size) * tile_size
		var px_rect := Rect2(px_pos, px_size)
		draw_rect(px_rect, Color(0.2, 0.8, 0.2, 0.25), true)
		draw_rect(px_rect, Color(0.2, 0.8, 0.2, 1.0),  false)

	# Draw the in-progress drag
	if _dragging:
		_active_rect = _world_to_tile_rect(_start_world, _current_world)
		var px_pos  := Vector2(_active_rect.position) * tile_size - camera.get_screen_center_position() + get_viewport_rect().size / 2
		var px_size := Vector2(_active_rect.size) * tile_size
		var px_rect := Rect2(px_pos, px_size)
		draw_rect(px_rect, Color(0.2, 0.6, 1.0, 0.25), true)   # blue while dragging
		draw_rect(px_rect, Color(0.2, 0.6, 1.0, 1.0),  false)

func _world_to_tile_rect(a: Vector2, b: Vector2) -> Rect2i:
	var tile_size := GameSettings.tile_size
	var ta := Vector2i(floori(a.x / tile_size), floori(a.y / tile_size))
	var tb := Vector2i(floori(b.x / tile_size), floori(b.y / tile_size))
	var top_left     := Vector2i(min(ta.x, tb.x), min(ta.y, tb.y))
	var bottom_right := Vector2i(max(ta.x, tb.x), max(ta.y, tb.y))
	return Rect2i(top_left, bottom_right - top_left + Vector2i(1, 1))

func confirm() -> void:
	area_confirmed.emit(_committed_rects.duplicate())
	deactivate()

func cancel() -> void:
	area_cancelled.emit()
	deactivate()
