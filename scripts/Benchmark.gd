# Benchmark.gd — GUARANTEED to show output
extends Node2D

@export var sample_frames: int = 10   # 10 seconds at 60 FPS
@export var show_overlay: bool = true

var frame_count := 0
var label: Label

func _ready():
	print("BENCHMARK SCRIPT STARTED — you should see this line!")
	
	if show_overlay:
		label = Label.new()
		label.position = Vector2(20, 20)
		label.add_theme_font_size_override("font_size", 24)
		label.add_theme_color_override("font_color", Color.YELLOW)
		add_child(label)
		label.text = "BENCHMARK RUNNING..."

func _process(_delta):
	frame_count += 1
	
	# Force visible proof every second
	if frame_count % 60 == 0:
		print("Frame ", frame_count, " — FPS: ", Performance.get_monitor(Performance.TIME_FPS))
		if label:
			label.text = "Frame: %d\nFPS: %.1f" % [frame_count, Performance.get_monitor(Performance.TIME_FPS)]

	# Final result and quit
	if frame_count >= sample_frames:
		var avg_fps = frame_count / Engine.get_frames_drawn() * Engine.get_frames_per_second()
		print("\n===== BENCHMARK DONE =====")
		print("Frames processed: ", frame_count)
		print("Average FPS: ", Performance.get_monitor(Performance.TIME_FPS))
		print("Process time avg: ", Performance.get_monitor(Performance.TIME_PROCESS) * 1000, " ms")
		print("Draw calls avg: ", Performance.get_monitor(Performance.RENDER_TOTAL_DRAW_CALLS_IN_FRAME))
		print("==========================\n")
		
		# Uncomment to auto-quit (great for headless)
		# get_tree().quit()
		
		# Keep running but stop spamming
		set_process(false)
