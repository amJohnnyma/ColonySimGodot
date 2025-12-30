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
	2: {  # Auto-generated sheet
		0: Rect2(  0,   0, 64, 64),
		1: Rect2( 64,   0, 64, 64),
		2: Rect2(128,   0, 64, 64),
		3: Rect2(192,   0, 64, 64),
		4: Rect2(256,   0, 64, 64),
		5: Rect2(320,   0, 64, 64),
		6: Rect2(384,   0, 64, 64),
		7: Rect2(448,   0, 64, 64),
		8: Rect2(  0,  64, 64, 64),
		9: Rect2( 64,  64, 64, 64),
		10: Rect2(128,  64, 64, 64),
		11: Rect2(192,  64, 64, 64),
		12: Rect2(256,  64, 64, 64),
		13: Rect2(320,  64, 64, 64),
		14: Rect2(384,  64, 64, 64),
		15: Rect2(448,  64, 64, 64),
		16: Rect2(  0, 128, 64, 64),
		17: Rect2( 64, 128, 64, 64),
		18: Rect2(128, 128, 64, 64),
		19: Rect2(192, 128, 64, 64),
		20: Rect2(256, 128, 64, 64),
		21: Rect2(320, 128, 64, 64),
		22: Rect2(384, 128, 64, 64),
		23: Rect2(448, 128, 64, 64),
		24: Rect2(  0, 192, 64, 64),
		25: Rect2( 64, 192, 64, 64),
		26: Rect2(128, 192, 64, 64),
		27: Rect2(192, 192, 64, 64),
		28: Rect2(256, 192, 64, 64),
		29: Rect2(320, 192, 64, 64),
		30: Rect2(384, 192, 64, 64),
		31: Rect2(448, 192, 64, 64),
		32: Rect2(  0, 256, 64, 64),
		33: Rect2( 64, 256, 64, 64),
		34: Rect2(128, 256, 64, 64),
		35: Rect2(192, 256, 64, 64),
		36: Rect2(256, 256, 64, 64),
		37: Rect2(320, 256, 64, 64),
		38: Rect2(384, 256, 64, 64),
		39: Rect2(448, 256, 64, 64),
		40: Rect2(  0, 320, 64, 64),
		41: Rect2( 64, 320, 64, 64),
		42: Rect2(128, 320, 64, 64),
		43: Rect2(192, 320, 64, 64),
		44: Rect2(256, 320, 64, 64),
		45: Rect2(320, 320, 64, 64),
		46: Rect2(384, 320, 64, 64),
		47: Rect2(448, 320, 64, 64),
		48: Rect2(  0, 384, 64, 64),
		49: Rect2( 64, 384, 64, 64),
		50: Rect2(128, 384, 64, 64),
		51: Rect2(192, 384, 64, 64),
		52: Rect2(256, 384, 64, 64),
		53: Rect2(320, 384, 64, 64),
		54: Rect2(384, 384, 64, 64),
		55: Rect2(448, 384, 64, 64),
		56: Rect2(  0, 448, 64, 64),
		57: Rect2( 64, 448, 64, 64),
		58: Rect2(128, 448, 64, 64),
		59: Rect2(192, 448, 64, 64),
		60: Rect2(256, 448, 64, 64),
		61: Rect2(320, 448, 64, 64),
		62: Rect2(384, 448, 64, 64),
		63: Rect2(448, 448, 64, 64)
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
	2: Vector2(0.015, 0.015),
	3: Vector2(0.03, 0.03),
	4: Vector2(0.04, 0.04),
}

@export var sheet_offsets: Dictionary = {
	1: Vector2(32, -32),
	2: Vector2(32, -32),
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

func get_offset(sheet_id: int) -> Vector2:
	return sheet_offsets.get(sheet_id, default_offset)
