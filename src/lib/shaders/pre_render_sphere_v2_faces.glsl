/* PRE RENDER SPHERE V 2 FACES.glsl
 *   by Lut99
 *
 * Created:
 *   05/05/2021, 15:29:36
 * Last edited:
 *   15/05/2021, 14:07:43
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Second version of the GPU-accelerated pre-render function, that directly
 *   renders to a list of indexed vertices that never sees CPU memory.
 *   Implemented in two stages. This is the second of the two, where we take
 *   the list of points generated and convert them to indexed faces.
**/

#version 450

/* Constants */
#define M_PI 3.14159265358979323846



/* Define the block size(s). */
layout (local_size_x = 32, local_size_y = 32) in;



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

// The output buffer of faces. There will be space for n_meridians + 2 * ((n_parallels - 2) * n_meridians) + n_meridians faces
layout(std430, set = 0, binding = 1) buffer Faces {
    GFace data[];
} faces;
// The output buffer of vertices. There will be space for 2 + (n_parallels - 2) * n_meridians vertices.
layout(std430, set = 0, binding = 2) buffer Vertices {
    vec3 data[];
} vertices;



/* The actual entry point. */
void main() {
    // Get the circle & point on it we're supposed to generate
	uint x = gl_GlobalInvocationID.x;
	uint y = gl_GlobalInvocationID.y + 1;
    uint max_x = sphere.n_meridians;
    uint max_y = sphere.n_parallels;

    // Pre-compute the minus versions
    uint x_m1 = max_x - 1;
    if (x > 0) { x_m1 = x - 1; }
    uint y_m1 = y - 1;

    // Switch to the correct generation algorithm based on the y value
    // Note that we do not launch for the north pole
    if (x < max_x && y == 1) {
        // Layer below the north cap

        // Get the index of the polar point
        uint p1 = 0;
        // Get the index of the previous point in this circle
        uint p2 = 1 + (y - 1) * max_x + x_m1;
        // Get the index of the current point in this circle
        uint p3 = 1 + (y - 1) * max_x + x;

        // Compute the normal for these fellas
        vec3 n = normalize(cross(vertices.data[p3] - vertices.data[p1], vertices.data[p2] - vertices.data[p1]));
        // And finally, compute the color
        vec3 c = sphere.color * abs(dot(n, vec3(0.0, 0.0, -1.0)));

        // Store as vertex
        faces.data[x].p1 = p1;
        faces.data[x].p2 = p2;
        faces.data[x].p3 = p3;
        faces.data[x].normal = n;
        faces.data[x].color = c;

    } else if (x < max_x && y > 1 && y < max_y) {
        // Layer below another layer

        // First, pre-compute the base index of this layer's vertices in the resulting buffer
        uint f_index = max_x + 2 * (y - 2) * max_x;

        // Get the index of the previous point in previous circle
        uint p2 = 1 + (y_m1 - 1) * max_x + x_m1;
        // Get the index of the current point in previous circle
        uint p3 = 1 + (y_m1 - 1) * max_x + x;
        // Get the index of the previous point in this circle
        uint p2 = 1 + (y - 1) * max_x + x_m1;
        // Get the index of the current point in this circle
        uint p3 = 1 + (y - 1) * max_x + x;
        
        // // Compute the previous point of the previous layer
        // vec3 p1 = compute_point(fx_m1, fy_m1);
        // // Compute the current point of the previous layer
        // vec3 p2 = compute_point(fx, fy_m1);
        // // Compute the previous point of the current layer
        // vec3 p3 = compute_point(fx_m1, fy);
        // // Compute the current point of the current layer
        // vec3 p4 = compute_point(fx, fy);

        // Compute the two normals for the two vertices
        vec3 n1 = normalize(cross(vertices.data[p4] - vertices.data[p1], vertices.data[p3] - vertices.data[p1]));
        vec3 n2 = normalize(cross(vertices.data[p4] - vertices.data[p1], vertices.data[p2] - vertices.data[p1]));
        // And finally, compute the two colors
        vec3 c1 = sphere.color * abs(dot(n1, vec3(0.0, 0.0, -1.0)));
        vec3 c2 = sphere.color * abs(dot(n2, vec3(0.0, 0.0, -1.0)));

        // Store them both
        faces.data[f_index + 2 * x].p1 = p1;
        faces.data[f_index + 2 * x].p2 = p3;
        faces.data[f_index + 2 * x].p3 = p4;
        faces.data[f_index + 2 * x].normal = n1;
        faces.data[f_index + 2 * x].color = c1;

        faces.data[f_index + 2 * x + 1].p1 = p1;
        faces.data[f_index + 2 * x + 1].p2 = p2;
        faces.data[f_index + 2 * x + 1].p3 = p4;
        faces.data[f_index + 2 * x + 1].normal = n2;
        faces.data[f_index + 2 * x + 1].color = c2;
        
    } else if (x < max_x && y == max_y) {
        // South cap

        // Compute some indices first
        uint f_index = max_x + 2 * (y - 2) * max_x;

        // Get the index of the polar point
        uint p1 = 1 + (y - 1) * max_x;
        // Get the index of the previous point in (previous) circle
        uint p2 = 1 + (y_m1 - 1) * max_x + x_m1;
        // Get the index of the current point in (previous) circle
        uint p3 = 1 + (y_m1 - 1) * max_x + x;

        // // Compute the polar point
        // vec3 p1 = sphere.center - vec3(0.0, sphere.radius, 0.0);
        // // Compute the previous point in the (previous) circle
        // vec3 p2 = compute_point(fx_m1, fy_m1);
        // // Compute the current point in the (previous) circle
        // vec3 p3 = compute_point(fx, fy_m1);

        // Compute the normal for these fellas
        vec3 n = normalize(cross(vertices.data[p3] - vertices.data[p1], vertices.data[p2] - vertices.data[p1]));
        // Finally, compute the color
        vec3 c = sphere.color * abs(dot(n, vec3(0.0, 0.0, -1.0)));

        // Store as vertex
        faces.data[f_index + x].p1 = p1;
        faces.data[f_index + x].p2 = p2;
        faces.data[f_index + x].p3 = p3;
        faces.data[f_index + x].normal = n;
        faces.data[f_index + x].color = c;
    }
}
