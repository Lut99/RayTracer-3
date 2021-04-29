/* VERTEX SHADER
 *   by Lut99
 *
 * Shader for mapping the vertices from world space to the output frame space.
 */

#version 450

// Define the block size(s)
layout (local_size_x = 32, local_size_y = 32) in;



/* Define specialization constants. */
// The width of the target frame
layout (constant_id = 0) const int width = 0;

// The height of the target frame
layout (constant_id = 1) const int height = 0;



/* Define the buffers. */
// The input data for the camera
layout(set = 0, binding = 0) buffer Camera {
    /* Vector placing the origin (middle) of the camera viewport in the world. */
    vec4 origin;
    /* Vector determining the horizontal line of the camera viewport in the game world, and also its conceptual size. */
    vec4 horizontal;
    /* Vector determining the vertical line of the camera viewport in the game world, and also its conceptual size. */
    vec4 vertical;
    /* Vector describing the lower left corner of the viewport. Shortcut based on the other three. */
    vec4 lower_left_corner;
} camera;

// The output frame to which we render
layout(set = 0, binding = 1) buffer Frame {
    vec4 pixels[];
} frame;



/* Computes the color of a ray given the vector representing it. */
vec3 ray_color(vec3 direction) {
    vec3 unit_direction = direction / length(direction);
    float t = 0.5 * (unit_direction.y + 1.0);
    return (1.0 - t) * vec3(1.0) + t * vec3(0.5, 0.7, 1.0);
}



/* The entry point to the shader. */
void main() {		
    // Get the index we're supposed to render
	uint x = gl_GlobalInvocationID.x;
	uint y = gl_GlobalInvocationID.y;

    // Set our value of the background, but only if still within range
    if (x < width && y < height) {
        // Compute the u & v, which is basically the ray's coordinates as a float
        float u = float(x) / (float(width) - 1.0);
        float v = float(height - 1 - y) / (float(height) - 1.0);

        // Compute the ray itself
        vec3 ray = camera.lower_left_corner.xyz + u * camera.horizontal.xyz + v * camera.vertical.xyz - camera.origin.xyz;

        // Store the computed color
        frame.pixels[y * width + x] = vec4(ray_color(ray), 0.0);
    }
}
