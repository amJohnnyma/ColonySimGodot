# SpriteAtlas.gd
extends Node

# Preloaded spritesheets — index = sheet_id from C++
@export var spritesheets: Array[Texture2D] = [
	null,  # index 0 reserved (or use as default/missing)
	preload("res://sprites/ninja-sheet.png"),      # 1 → Players
	preload("res://sprites/Castle2.png"),    # 2 → Buildings
#	preload("res://sprites/item-sheet.png"),        # 3 → Items / Environment
#	preload("res://sprites/creature-sheet.png"),    # 4 → Creatures, etc.
]

# Regions: sheet_id → { variant_id → Rect2 }
@export var sheet_regions: Dictionary = {
	1: {  # Player sheet
		0: Rect2(0,   0,   64, 64),  
		1: Rect2(64,  0,   64, 64),
		2: Rect2(128,  0,   64, 64),
		3: Rect2(192,  0,   64, 64),
		4: Rect2(256,  0,   64, 64),
		5: Rect2(0,  64,   64, 64),
		6: Rect2(64,  64,   64, 64),
		7: Rect2(128,  64,   64, 64),
		8: Rect2(192,  64,   64, 64),
		9: Rect2(256,  64,   64, 64),
		10: Rect2(0,  128,   64, 64),
		11: Rect2(64,  128,   64, 64),
		12: Rect2(128,  128,   64, 64),
		13: Rect2(192,  128,   64, 64),
		14: Rect2(256,  128,   64, 64)
	},
2: { # Auto-generated sheet
  0: Rect2(  0, 416,  96,  96), # 0 13 (top left)
  1: Rect2( 96, 416,  96,  96), # 3 13 (top left)
  2: Rect2(192, 416,  32,  96), # 6 13 (top left)
  3: Rect2(224, 448,  64,  64), # 7 14 (top left)
  4: Rect2(288, 384,  64,  64), # 9 12 (top left)
  5: Rect2(  0,   0, 192, 192), # 0 0 (top left)
  6: Rect2(192,   0, 128, 224), # 6 0 (top left)
  7: Rect2(320,   0, 192, 192), # 10 0 (top left)
  8: Rect2(  0, 192, 32, 32), # 0 6 (top left)
  9: Rect2( 32, 192, 32, 32), # 1 6 (top left)
 10: Rect2( 64, 192, 32, 32), # 2 6 (top left)
 11: Rect2( 96, 192, 32, 32), # 3 6 (top left)
 12: Rect2(128, 192, 32, 32), # 4 6 (top left)
 13: Rect2(160, 192, 32, 32), # 5 6 (top left)
 14: Rect2(320, 192, 32, 32), # 10 6 (top left)
 15: Rect2(352, 192, 32, 32), # 11 6 (top left)
 16: Rect2(384, 192, 32, 32), # 12 6 (top left)
 17: Rect2(416, 192, 32, 32), # 13 6 (top left)
 18: Rect2(448, 192, 32, 32), # 14 6 (top left)
 19: Rect2(480, 192, 32, 32), # 15 6 (top left)
 20: Rect2(  0, 224, 32, 32), # 0 7 (top left)
 21: Rect2( 32, 224, 32, 32), # 1 7 (top left)
 22: Rect2( 64, 224, 32, 32), # 2 7 (top left)
 23: Rect2( 96, 224, 32, 32), # 3 7 (top left)
 24: Rect2(128, 224, 32, 32), # 4 7 (top left)
 25: Rect2(160, 224, 32, 32), # 5 7 (top left)
 26: Rect2(192, 224, 32, 32), # 6 7 (top left)
 27: Rect2(224, 224, 32, 32), # 7 7 (top left)
 28: Rect2(256, 224, 32, 32), # 8 7 (top left)
 29: Rect2(288, 224, 32, 32), # 9 7 (top left)
 30: Rect2(320, 224, 32, 32), # 10 7 (top left)
 31: Rect2(352, 224, 32, 32), # 11 7 (top left)
 32: Rect2(384, 224, 32, 32), # 12 7 (top left)
 33: Rect2(416, 224, 32, 32), # 13 7 (top left)
 34: Rect2(448, 224, 32, 32), # 14 7 (top left)
 35: Rect2(480, 224, 32, 32), # 15 7 (top left)
 36: Rect2(  0, 256, 32, 32), # 0 8 (top left)
 37: Rect2( 32, 256, 32, 32), # 1 8 (top left)
 38: Rect2( 64, 256, 32, 32), # 2 8 (top left)
 39: Rect2( 96, 256, 32, 32), # 3 8 (top left)
 40: Rect2(128, 256, 32, 32), # 4 8 (top left)
 41: Rect2(160, 256, 32, 32), # 5 8 (top left)
 42: Rect2(192, 256, 32, 32), # 6 8 (top left)
 43: Rect2(224, 256, 32, 32), # 7 8 (top left)
 44: Rect2(256, 256, 32, 32), # 8 8 (top left)
 45: Rect2(288, 256, 32, 32), # 9 8 (top left)
 46: Rect2(320, 256, 32, 32), # 10 8 (top left)
 47: Rect2(352, 256, 32, 32), # 11 8 (top left)
 48: Rect2(384, 256, 32, 32), # 12 8 (top left)
 49: Rect2(416, 256, 32, 32), # 13 8 (top left)
 50: Rect2(448, 256, 32, 32), # 14 8 (top left)
 51: Rect2(480, 256, 32, 32), # 15 8 (top left)
 52: Rect2(  0, 288, 32, 32), # 0 9 (top left)
 53: Rect2( 32, 288, 32, 32), # 1 9 (top left)
 54: Rect2( 64, 288, 32, 32), # 2 9 (top left)
 55: Rect2( 96, 288, 32, 32), # 3 9 (top left)
 56: Rect2(128, 288, 32, 32), # 4 9 (top left)
 57: Rect2(160, 288, 32, 32), # 5 9 (top left)
 58: Rect2(192, 288, 32, 32), # 6 9 (top left)
 59: Rect2(224, 288, 32, 32), # 7 9 (top left)
 60: Rect2(256, 288, 32, 32), # 8 9 (top left)
 61: Rect2(288, 288, 32, 32), # 9 9 (top left)
 62: Rect2(320, 288, 32, 32), # 10 9 (top left)
 63: Rect2(352, 288, 32, 32), # 11 9 (top left)
 64: Rect2(384, 288, 32, 32), # 12 9 (top left)
 65: Rect2(416, 288, 32, 32), # 13 9 (top left)
 66: Rect2(448, 288, 32, 32), # 14 9 (top left)
 67: Rect2(480, 288, 32, 32), # 15 9 (top left)
 68: Rect2(  0, 320, 32, 32), # 0 10 (top left)
 69: Rect2( 32, 320, 32, 32), # 1 10 (top left)
 70: Rect2( 64, 320, 32, 32), # 2 10 (top left)
 71: Rect2( 96, 320, 32, 32), # 3 10 (top left)
 72: Rect2(128, 320, 32, 32), # 4 10 (top left)
 73: Rect2(160, 320, 32, 32), # 5 10 (top left)
 74: Rect2(192, 320, 32, 32), # 6 10 (top left)
 75: Rect2(224, 320, 32, 32), # 7 10 (top left)
 76: Rect2(256, 320, 32, 32), # 8 10 (top left)
 77: Rect2(288, 320, 32, 32), # 9 10 (top left)
 78: Rect2(320, 320, 32, 32), # 10 10 (top left)
 79: Rect2(352, 320, 32, 32), # 11 10 (top left)
 80: Rect2(384, 320, 32, 32), # 12 10 (top left)
 81: Rect2(416, 320, 32, 32), # 13 10 (top left)
 82: Rect2(448, 320, 32, 32), # 14 10 (top left)
 83: Rect2(480, 320, 32, 32), # 15 10 (top left)
 84: Rect2(  0, 352, 32, 32), # 0 11 (top left)
 85: Rect2( 32, 352, 32, 32), # 1 11 (top left)
 86: Rect2( 64, 352, 32, 32), # 2 11 (top left)
 87: Rect2( 96, 352, 32, 32), # 3 11 (top left)
 88: Rect2(128, 352, 32, 32), # 4 11 (top left)
 89: Rect2(160, 352, 32, 32), # 5 11 (top left)
 90: Rect2(192, 352, 32, 32), # 6 11 (top left)
 91: Rect2(224, 352, 32, 32), # 7 11 (top left)
 92: Rect2(256, 352, 32, 32), # 8 11 (top left)
 93: Rect2(288, 352, 32, 32), # 9 11 (top left)
 94: Rect2(320, 352, 32, 32), # 10 11 (top left)
 95: Rect2(352, 352, 32, 32), # 11 11 (top left)
 96: Rect2(384, 352, 32, 32), # 12 11 (top left)
 97: Rect2(416, 352, 32, 32), # 13 11 (top left)
 98: Rect2(448, 352, 32, 32), # 14 11 (top left)
 99: Rect2(480, 352, 32, 32), # 15 11 (top left)
 100: Rect2(  0, 384, 32, 32), # 0 12 (top left)
 101: Rect2( 32, 384, 32, 32), # 1 12 (top left)
 102: Rect2( 64, 384, 32, 32), # 2 12 (top left)
 103: Rect2( 96, 384, 32, 32), # 3 12 (top left)
 104: Rect2(128, 384, 32, 32), # 4 12 (top left)
 105: Rect2(160, 384, 32, 32), # 5 12 (top left)
 106: Rect2(192, 384, 32, 32), # 6 12 (top left)
 107: Rect2(224, 384, 32, 32), # 7 12 (top left)
 108: Rect2(256, 384, 32, 32), # 8 12 (top left)
 109: Rect2(352, 384, 32, 32), # 11 12 (top left)
 110: Rect2(384, 384, 32, 32), # 12 12 (top left)
 111: Rect2(416, 384, 32, 32), # 13 12 (top left)
 112: Rect2(448, 384, 32, 32), # 14 12 (top left)
 113: Rect2(480, 384, 32, 32), # 15 12 (top left)
 114: Rect2(224, 416, 32, 32), # 7 13 (top left)
 115: Rect2(256, 416, 32, 32), # 8 13 (top left)
 116: Rect2(352, 416, 32, 32), # 11 13 (top left)
 117: Rect2(384, 416, 32, 32), # 12 13 (top left)
 118: Rect2(416, 416, 32, 32), # 13 13 (top left)
 119: Rect2(448, 416, 32, 32), # 14 13 (top left)
 120: Rect2(480, 416, 32, 32), # 15 13 (top left)
 121: Rect2(288, 448, 32, 32), # 9 14 (top left)
 122: Rect2(320, 448, 32, 32), # 10 14 (top left)
 123: Rect2(352, 448, 32, 32), # 11 14 (top left)
 124: Rect2(384, 448, 32, 32), # 12 14 (top left)
 125: Rect2(416, 448, 32, 32), # 13 14 (top left)
 126: Rect2(448, 448, 32, 32), # 14 14 (top left)
 127: Rect2(480, 448, 32, 32), # 15 14 (top left)
 128: Rect2(288, 480, 32, 32), # 9 15 (top left)
 129: Rect2(320, 480, 32, 32), # 10 15 (top left)
 130: Rect2(352, 480, 32, 32), # 11 15 (top left)
 131: Rect2(384, 480, 32, 32), # 12 15 (top left)
 132: Rect2(416, 480, 32, 32), # 13 15 (top left)
 133: Rect2(448, 480, 32, 32), # 14 15 (top left)
 134: Rect2(480, 480, 32, 32) # 15 15 (top left)
},

	3: {  # Items
		0: Rect2(64, 160, 16, 32),  # bush
		1: Rect2(64, 0, 16, 32),    # tile / ground decoration
	},
	# Add more sheets here...
}

# Per-sheet scale and offset (can vary per category)
@export var sheet_scales: Dictionary = {
	1: Vector2(0.015, 0.015),
	2: Vector2(0.03, 0.03),
	3: Vector2(0.03, 0.03),
	4: Vector2(0.04, 0.04),
}

@export var sheet_offsets: Dictionary = {
	1: Vector2(32, -32),
	2: Vector2(16, -16),
	3: Vector2(0, 0),
	4: Vector2(0, 0),
}

# Fallbacks
@export var default_texture: Texture2D = preload("res://sprites/sprite-sheet.png")  # Optional missing texture
@export var default_region: Rect2 = Rect2(0, 0, 16, 16)
@export var default_scale: Vector2 = Vector2(0.03, 0.03)
@export var default_offset: Vector2 = Vector2(8, -8)


# Fast getter functions
func get_texture(sheet_id: int) -> Texture2D:
	if sheet_id > 0 && sheet_id < spritesheets.size():
		return spritesheets[sheet_id]
	print("Default texture")
	return default_texture

func get_region(sheet_id: int, variant_id: int) -> Rect2:
	var sheet_dict = sheet_regions.get(sheet_id)
	if sheet_dict != null:
		return sheet_dict.get(variant_id, default_region)
	return default_region

func get_scale(sheet_id: int) -> Vector2:
	return sheet_scales.get(sheet_id, default_scale)

func get_offset(sheet_id: int, variant_id: int) -> Vector2:
	var region: Rect2 = get_region(sheet_id, variant_id)
	var size_px: Vector2 = region.size  # e.g. Vector2(96, 160) for a 3×5 at 32px/tile

	# Centered pivot (most common for Godot sprites)
	return Vector2(size_px.x / 2.0, size_px.y / 2.0)
