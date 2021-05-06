/* RAYTRACER V 1.glsl
 *   by Lut99
 *
 * Created:
 *   03/05/2021, 13:59:41
 * Last edited:
 *   06/05/2021, 16:15:32
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Contains the code for the very very first version of the raytracer,
 *   that simply sends out one ray per pixel and does not really do
 *   bouncing.
**/

#version 450

/* Define the block size(s). */
layout (local_size_x = 32, local_size_y = 32) in;



/* Structs */
// The GVertex struct, which is a single vertex ready to be rendered on the GPU. */
struct GVertex {
    /* The first point of the vertex. */
    uint p1;
    /* The second point of the vertex. */
    uint p2;
    /* The third point of the vertex. */
    uint p3;

    /* The normal of the vertex. */
    vec4 normal;
    /* The color of the vertex. */
    vec4 color;  
};



/* Define specialization constants. */
// The width of the target frame
layout (constant_id = 0) const int width = 0;

// The height of the target frame
layout (constant_id = 1) const int height = 0;



/* Define the buffers. */
// The output frame to which we render
layout(std430, set = 0, binding = 0) buffer Frame {
    vec4 pixels[];
} frame;

// The input data for the camera
layout(std430, set = 0, binding = 1) buffer Camera {
    /* Vector placing the origin (middle) of the camera viewport in the world. */
    vec3 origin;
    /* Vector determining the horizontal line of the camera viewport in the game world, and also its conceptual size. */
    vec3 horizontal;
    /* Vector determining the vertical line of the camera viewport in the game world, and also its conceptual size. */
    vec3 vertical;
    /* Vector describing the lower left corner of the viewport. Shortcut based on the other three. */
    vec3 lower_left_corner;
} camera;

// The list of vertices we're supposed to render
layout(std430, set = 0, binding = 2) buffer GVertices {
    GVertex data[];
} vertices;

// Finally, the list of unique points used by the vertices
layout(std430, set = 0, binding = 3) buffer Points {
    vec4 data[];
} points;



/* Computes the color of a ray given the vector representing it. */
vec4 ray_color(vec3 origin, vec3 direction) {
    // Loop through the vertices so find any one we hit
    uint min_i = 0;
    float min_t = 1e99;
    for (uint i = 0; i < vertices.data.length(); i++) {
        // First, check if the ray happens to be perpendicular to the triangle's plane
        vec3 normal = vertices.data[i].normal.xyz;
        if (dot(direction, normal) == 0) {
            // No intersection for sure
            continue;
        }

        // Otherwise, fetch the points from the point list
        vec3 p1 = points.data[vertices.data[i].p1].xyz;
        vec3 p2 = points.data[vertices.data[i].p2].xyz;
        vec3 p3 = points.data[vertices.data[i].p3].xyz;

        // Otherwise, compute the distance point of the plane
        float plane_distance = dot(normal, p1);

        // Use that to compute the distance the ray travels before it hits the plane
        float t = (dot(normal, origin) + plane_distance) / dot(normal, direction);
        if (t < 0 || t >= min_t) {
            // Negative t or a t further than one we already found as closer, so we hit the triangle behind us
            continue;
        }

        // Now, compute the actual point where we hit the plane
        vec3 hitpoint = origin + t * direction;

        // We now perform the inside-out test to see if the triangle is hit within the plane
        // General idea: https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/barycentric-coordinates
        if (-dot(normal, cross(p2 - p1, hitpoint - p1)) >= 0.0 &&
            -dot(normal, cross(p3 - p2, hitpoint - p2)) >= 0.0 &&
            -dot(normal, cross(p1 - p3, hitpoint - p3)) >= 0.0)
        {
            // It's a hit! Store it as the closest t so far
            min_i = i;
            min_t = t;
            continue;
        }
    }

    // If we hit a vertex (or its too far away), return its color
    if (min_t < 1e99) {
        return vertices.data[min_i].color;
    } else {
        // Return the blue sky
        vec3 unit_direction = direction / length(direction);
        float t = 0.5 * (unit_direction.y + 1.0);
        return vec4((1.0 - t) * vec3(1.0) + t * vec3(0.5, 0.7, 1.0), 0.0);
    }
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
        vec3 ray = camera.lower_left_corner + u * camera.horizontal + v * camera.vertical - camera.origin;

        // Compute the ray's color and store it as a vector
        frame.pixels[y * width + x] = ray_color(camera.origin, ray);

        // if (y * width + x == 0) {
        //     // Return the number of vertices for comparison purposes
        //     frame.pixels[0] = vec4(float(vertices.data.length()), points.data[vertices.data[0].p1].x, points.data[vertices.data[0].p1].y, points.data[vertices.data[0].p1].z);
        // } else if (y * width + x == 1) {
        //     frame.pixels[1] = points.data[vertices.data[0].p2];
        // } else if (y * width + x == 2) {
        //     frame.pixels[2] = points.data[vertices.data[0].p3];
        // } else if (y * width + x == 3) {
        //     frame.pixels[3] = vertices.data[0].normal;
        // } else if (y * width + x == 4) {
        //     frame.pixels[4] = vertices.data[0].color;
        // } else if (y * width + x == 5) {
        //     frame.pixels[5] = vec4(camera.origin, 0.0);
        // } else if (y * width + x == 6) {
        //     frame.pixels[6] = vec4(camera.horizontal, 0.0);
        // } else if (y * width + x == 7) {
        //     frame.pixels[7] = vec4(camera.vertical, 0.0);
        // } else if (y * width + x == 8) {
        //     frame.pixels[8] = vec4(camera.lower_left_corner, 0.0);
        // }
    }
}
