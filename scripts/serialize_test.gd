extends Node

func _ready():
	var world = World.new()
	var passed = world.test_serialization()
	
	if passed:
		print("✓ ALL TESTS PASSED!")
		get_tree().quit(0)
	else:
		printerr("✗ TESTS FAILED")
		get_tree().quit(1)
