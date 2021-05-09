/* PRE RENDER SPHERE V 1.glsl
 *   by Lut99
 *
 * Created:
 *   05/05/2021, 15:29:36
 * Last edited:
 *   09/05/2021, 17:13:35
 * Auto updated?
 *   Yes
 *
 * Description:
 *   First version of the GPU-accelerated pre-render function.
**/

#version 450

/* Constants */
#define M_PI 3.14159265358979323846



/* Define the block size(s). */
layout (local_size_x = 32, local_size_y = 32) in;



/* Structs */
// The Vertex struct, which is a single vertex not yet condensed to indices. */
struct Vertex {
    /* The first point of the vertex. */
    vec3 p1;
    /* The second point of the vertex. */
    vec3 p2;
    /* The third point of the vertex. */
    vec3 p3;

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

// The output buffer of vertices. There will be space for n_meridians + 2 * ((n_parallels - 2) * n_meridians) + n_meridians
layout(std430, set = 0, binding = 1) buffer Vertices {
    Vertex data[];
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
	uint y = gl_GlobalInvocationID.y + 1;
    uint max_x = sphere.n_meridians;
    uint max_y = sphere.n_parallels;

    // Pre-compute some values as float
    float fx_m1 = float(max_x - 1);
    if (x > 0) { fx_m1 = float(x - 1); }
    float fx = float(x);
    float fy_m1 = float(y - 1);
    float fy = float(y);
    float fmax_x = float(max_x);
    float fmax_y = float(max_y);

    // Switch to the correct generation algorithm based on the y value
    // Note that we do not launch for the north pole
    if (x < max_x && y == 1) {
        // Layer below the north cap
        
        // Compute the polar point
        vec3 p1 = sphere.center + vec3(0.0, sphere.radius, 0.0);
        // Compute the previous point in this circle
        vec3 p2 = compute_point(fx_m1, fy);
        // Compute the current point in this circle
        vec3 p3 = compute_point(fx, fy);

        // Compute the normal for these fellas
        vec3 n = normalize(cross(p3 - p1, p2 - p1));
        // And finally, compute the color
        vec3 c = sphere.color * abs(dot(n, vec3(0.0, 0.0, -1.0)));

        // Store as vertex
        vertices.data[x].p1 = p1;
        vertices.data[x].p2 = p2;
        vertices.data[x].p3 = p3;
        vertices.data[x].normal = n;
        vertices.data[x].color = c;

    } else if (x < max_x && y > 1 && y < max_y) {
        // Layer below another layer

        // First, pre-compute the base index of this layer's vertices in the resulting buffer
        uint v_index = max_x + 2 * (y - 2) * max_x;

        // Compute the previous point of the previous layer
        vec3 p1 = compute_point(fx_m1, fy_m1);
        // Compute the current point of the previous layer
        vec3 p2 = compute_point(fx, fy_m1);
        // Compute the previous point of the current layer
        vec3 p3 = compute_point(fx_m1, fy);
        // Compute the current point of the current layer
        vec3 p4 = compute_point(fx, fy);

        // Compute the two normals for the two vertices
        vec3 n1 = normalize(cross(p4 - p1, p3 - p1));
        vec3 n2 = normalize(cross(p4 - p1, p2 - p1));
        // And finally, compute the two colors
        vec3 c1 = sphere.color * abs(dot(n1, vec3(0.0, 0.0, -1.0)));
        vec3 c2 = sphere.color * abs(dot(n2, vec3(0.0, 0.0, -1.0)));

        // Store them both
        vertices.data[v_index + 2 * x].p1 = p1;
        vertices.data[v_index + 2 * x].p2 = p3;
        vertices.data[v_index + 2 * x].p3 = p4;
        vertices.data[v_index + 2 * x].normal = n1;
        vertices.data[v_index + 2 * x].color = c1;

        vertices.data[v_index + 2 * x + 1].p1 = p1;
        vertices.data[v_index + 2 * x + 1].p2 = p2;
        vertices.data[v_index + 2 * x + 1].p3 = p4;
        vertices.data[v_index + 2 * x + 1].normal = n2;
        vertices.data[v_index + 2 * x + 1].color = c2;
        
    } else if (x < max_x && y == max_y) {
        // South cap

        // Compute some indices first
        uint v_index = max_x + 2 * (y - 2) * max_x;

        // Compute the polar point
        vec3 p1 = sphere.center - vec3(0.0, sphere.radius, 0.0);
        // Compute the previous point in the (previous) circle
        vec3 p2 = compute_point(fx_m1, fy_m1);
        // Compute the current point in the (previous) circle
        vec3 p3 = compute_point(fx, fy_m1);

        // Compute the normal for these fellas
        vec3 n = normalize(cross(p3 - p1, p2 - p1));
        // Finally, compute the color
        vec3 c = sphere.color * abs(dot(n, vec3(0.0, 0.0, -1.0)));

        // Store as vertex
        vertices.data[v_index + x].p1 = p1;
        vertices.data[v_index + x].p2 = p2;
        vertices.data[v_index + x].p3 = p3;
        vertices.data[v_index + x].normal = n;
        vertices.data[v_index + x].color = c;
    }
}
