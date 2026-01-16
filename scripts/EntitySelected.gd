extends Control



# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	pass # Replace with function body.


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	pass


func entities_selected(ids : Array, types : Array, sprites : Array, epos : Array[Vector2i]) -> void:
	# Populate the UI with the first entity for now
	var id = ids[0]
	var type = types[0]
	var sprite = sprites[0]
	var pos = epos[0]
	
	# Make a sprite the Icon
	var full_tex = SpriteAtlas.get_texture(type)
	var region  = SpriteAtlas.get_region(type, sprite)
	var c_scale = SpriteAtlas.get_scale(type)
	var c_offset = SpriteAtlas.get_offset(type, sprite)

	var atlas_tex = AtlasTexture.new()
	atlas_tex.atlas = full_tex
	atlas_tex.region = region
	
	$MarginContainer/Panel/EntityJobPanel/IconMargin/Icon/TextureRect.texture = atlas_tex
	#$MarginContainer/Panel/EntityJobPanel/IconMargin/Icon/TextureRect.scale   = c_scale
#	$MarginContainer/Panel/EntityJobPanel/IconMargin/Icon/TextureRect.offset  = c_offset  
	# Show the ID and pos in the box below
