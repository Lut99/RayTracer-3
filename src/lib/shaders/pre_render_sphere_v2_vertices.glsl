/* PRE RENDER SPHERE V 2 VERTICES.glsl
 *   by Lut99
 *
 * Created:
 *   05/05/2021, 15:29:36
 * Last edited:
 *   19/05/2021, 16:59:01
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Second version of the GPU-accelerated pre-render function, that directly
 *   renders to a list of indexed vertices that never sees CPU memory.
 *   Implemented in two stages. This is the first of the two, where we generate
 *   a list of vertices for the sphere with the required specs.
**/

#version 450

/* Constants */
#define M_PI 3.14159265358979323846



/* Define the block size(s). */
layout (local_size_x = 32, local_size_y = 32) in;



/* Define specialization constants. */
// The offset in the target buffer
layout (constant_id = 0) const int vertex_offset = 0;



/* Structs */
// The GFace struct, which is a single vertex condensed to indices. */
struct GFace {
    /* The first point of the vertex. */
    uint p1;
    /* The second point of the vertex. */
    uint p2;
    /* The third point of the vertex. */
    uint p3;

    /* The normal of the vertex. */
    vec3 normal;
    /* The color of the vertex. */
    vec3 color;  
};



/* Define the buffers. */
// The input data describing the sphere we render
layout(std140, set = 0, binding = 0) uniform Sphere {
    /* The center of the sphere. */
    vec3 center;
    /* The radius of the sphere. */
    float radius;
    /* The number of meridians in the sphere (i.e., vertical lines). */
    uint n_meridians;
    /* The number of parallels in the sphere (i.e., horizontal lines). */
    uint n_parallels;
    /* The color of the sphere. */
    vec3 color;
} sphere;

// The output buffer of vertices. There will be space for 2 + (n_parallels - 2) * n_meridians vertices.
layout(std430, set = 0, binding = 2) buffer Vertices {
    vec3 data[];
} vertices;



/* Computes the coordinates of a single point on the sphere. */
vec3 compute_point(float fx, float fy) {
    return sphere.center + sphere.radius * vec3(
        sin(M_PI * (fy / float(sphere.n_parallels))) * cos(2 * M_PI * (fx / float(sphere.n_meridians))),
        cos(M_PI * (fy / float(sphere.n_parallels))),
        sin(M_PI * (fy / float(sphere.n_parallels))) * sin(2 * M_PI * (fx / float(sphere.n_meridians)))
    );
}



/* The actual entry point. */
void main() {
    // Get the circle & point on it we're supposed to generate
	uint x = gl_GlobalInvocationID.x;
	uint y = gl_GlobalInvocationID.y;
    uint max_x = sphere.n_meridians;
    uint max_y = sphere.n_parallels;

    // Pre-compute some values as float
    float fx = float(x);
    float fy = float(y);

    // Switch to the correct generation algorithm based on the y value
    // Note that we do not launch for the north pole
    if (x == 0 && y == 0) {
        // Generate the polar cap
        vertices.data[vertex_offset] = compute_point(fx, fy);

    } else if (x < max_x && y > 0 && y < max_y - 1) {
        // Circle layer
        vertices.data[vertex_offset + 1 + (y - 1) * max_x + x] = compute_point(fx, fy);
        
    } else if (x == 0 && y == max_y - 1) {
        // South cap
        vertices.data[vertex_offset + 1 + (y - 1) * max_x] = compute_point(fx, fy);
    }
}
