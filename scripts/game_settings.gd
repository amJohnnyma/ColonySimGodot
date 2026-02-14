class_name Game_Settings
extends Node2D

@export var max_world_tiles : int = 500
@export var chunk_size : int = 64
@export var tile_size: float = 1.0  
@export var max_render_distance_chunks: int = 1
@export var render_buffer_chunks: int = max_render_distance_chunks + 2
@export var simulation_distance: int = 20
@export var paused : bool = true
@export var TARGET_FPS : int = 60  # Tune this (30-60)
@export var MIN_SLOWDOWN : float = 0.1  # Never slower than 10% speed
@export var HISTORY_FRAMES : int = 30  # Average FPS over ~0.5s
@export var sim_rate_target: float = 1.0 / 30.0   # aim for ~30 sim ticks/sec (adjustable)
@export var max_sim_delta_per_tick: float = 0.1   # cap single sim step to avoid huge jumps


	
