class_name Sprite_Atlas
extends Node2D

# currently 15 x 6
var entity_atlas: Texture2D = preload("res://sprites/sprite-sheet.png")


# Define your entity types (match your C++ enum!)
enum EntityType { PLAYER_FRONT = 0, PLAYER_BACK = 1, PLAYER_CLIMB_FRONT = 2, PLAYER_CLIMB_BACK = 3, BUSH = 4, TILE=5 }

@export var type_to_region: Dictionary = { # x = 16px, y = 32px
	EntityType.PLAYER_FRONT: Rect2(0,   0, 16, 32), # 0,0
	EntityType.PLAYER_BACK: Rect2(48,  32, 16, 32), # 3, 1
	EntityType.PLAYER_CLIMB_FRONT:  Rect2(32,  96, 16, 32), # 2, 3
	EntityType.PLAYER_CLIMB_BACK:   Rect2(80,  96, 16, 32), # 5 , 3
	EntityType.BUSH:   Rect2(64,  160, 16, 32), # 4, 5
	EntityType.TILE:    Rect2(64,  0, 16, 32), # 4,0
}

@export var type_to_scale: Dictionary = {
	EntityType.PLAYER_FRONT: Vector2(0.03, 0.03),
	EntityType.PLAYER_BACK: Vector2(0.03, 0.03),
	EntityType.PLAYER_CLIMB_FRONT: Vector2(0.03, 0.03),
	EntityType.PLAYER_CLIMB_BACK: Vector2(0.03, 0.03),
	EntityType.BUSH: Vector2(0.03, 0.03),
	EntityType.TILE: Vector2(0.03, 0.03),
}

@export var type_to_offset: Dictionary = {
	EntityType.PLAYER_FRONT: Vector2(16,-16),
	EntityType.PLAYER_BACK: Vector2(16, -16),
	EntityType.PLAYER_CLIMB_FRONT: Vector2(16, -16),
	EntityType.PLAYER_CLIMB_BACK: Vector2(16, -16),
	EntityType.BUSH: Vector2(16, -16),
	EntityType.TILE: Vector2(16, -16),
}

@export var default_scale: Vector2 = Vector2(0.03, 0.03)
@export var default_region: Rect2 = Rect2(0, 0, 16, 32)
@export var default_offset: Vector2 = Vector2(16, -16)
