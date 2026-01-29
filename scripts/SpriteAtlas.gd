# SpriteAtlas.gd
extends Node


# Regions: sheet_id → { variant_id → Rect2 }
@export var sheet_regions: Dictionary = {}
@export var sheet_entity_sizes: Dictionary = {}

func _ready():
	load_sheet(1, "res://EntityData/sprite_sheet_1_colonist.json")
	load_sheet(2, "res://EntityData/sprite_sheet_2_building.json")
	load_sheet(3, "res://EntityData/sprite_sheet_3_item.json")


func load_sheet(sheet_id: int, path: String):
	var data = load_json(path)
	sheet_regions[sheet_id] = {}
	sheet_entity_sizes[sheet_id] = {}
	
	for sprite_id in data.keys():
		var sprite_id_int = int(sprite_id)
		var rect_data = data[sprite_id]
		
		
		sheet_regions[sheet_id][sprite_id_int] = Rect2(
			rect_data["x"], rect_data["y"],
			rect_data["w"], rect_data["h"]
		)
		
		sheet_entity_sizes[sheet_id][sprite_id_int] = Vector2i(
			rect_data["size_x"], rect_data["size_y"]
		)

func load_json(path: String) -> Variant:
	var file = FileAccess.open(path, FileAccess.READ)
	if file == null:
		push_error("Could not open: " + path)
		return {}
	
	var json_string = file.get_as_text()
	file.close()
	
	var json = JSON.new()
	if json.parse(json_string) != OK:
		push_error("Parse error in " + path)
		return {}
	
	return json.data

# Preloaded spritesheets — index = sheet_id from C++
@export var spritesheets: Array[Texture2D] = [
	null,  # index 0 reserved (or use as default/missing)
	preload("res://sprites/ninja-sheet.png"),      # 1 → Players
	preload("res://sprites/Castle2.png"),    # 2 → Buildings
	preload("res://sprites/items-sheet.png"),        # 3 → Items / Environment
#	preload("res://sprites/creature-sheet.png"),    # 4 → Creatures, etc.
]



# Per-sheet scale and offset (can vary per category)
@export var sheet_scales: Dictionary = {
	1: Vector2(0.015, 0.015),
	2: Vector2(0.03, 0.03),
	3: Vector2(0.06, 0.06),
	4: Vector2(0.04, 0.04),
}

@export var sheet_offsets: Dictionary = {
	1: Vector2(32, -32),
	2: Vector2(16, -16),
	3: Vector2(8, -8),
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
	return Vector2(size_px.x / 2.0, -size_px.y / 2.0)

func get_entity_size(sheet_id:int, entity_id:int) -> Vector2i:
	var sheet_dict = sheet_entity_sizes.get(sheet_id)
	if sheet_dict != null:
		return sheet_dict.get(entity_id, Vector2i(1,1))
	return Vector2i(1,1)
