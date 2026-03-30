extends CanvasLayer

signal confirmed
signal cancelled

func activate() -> void:
	visible = true

func deactivate() -> void:
	visible = false

func _on_confirm_pressed() -> void:
	confirmed.emit()

func _on_cancel_pressed() -> void:
	cancelled.emit()
