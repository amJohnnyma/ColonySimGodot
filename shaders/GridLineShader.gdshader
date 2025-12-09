shader_type canvas_item;

uniform float grid_size : hint_range(1, 64) = 1.0;  // Matches your tile size (1)
uniform vec4 grid_color : source_color = vec4(0.2, 0.2, 0.2, 0.8);  // Dark gray
uniform float line_width : hint_range(0.01, 0.2) = 0.05;  // Thin lines

void fragment() {
    vec2 uv = UV * grid_size;  // Scale to tile grid
    vec2 grid_uv = fract(uv);  // 0-1 per tile

    float line = 0.0;
    if (grid_uv.x < line_width || grid_uv.y < line_width ||
        grid_uv.x > 1.0 - line_width || grid_uv.y > 1.0 - line_width) {
        line = 1.0;
    }

    COLOR.rgb = mix(COLOR.rgb, grid_color.rgb, line * grid_color.a);
    COLOR.a *= 1.0 - line * 0.3;  // Fade original slightly
}