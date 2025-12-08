# Main.gd  ← attach to the root Node2D
extends Node2D

func _ready():
	$World.init(2_000_000, 2_000_000, 16)
	print("World initialized – 2 million × 2 million tiles")
