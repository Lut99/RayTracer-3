/* PRE RENDER SPHERE V 1.glsl
 *   by Lut99
 *
 * Created:
 *   05/05/2021, 15:29:36
 * Last edited:
 *   05/05/2021, 18:44:48
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



/* Define specialization constants. */
// The width of the target frame
layout (constant_id = 0) const uint stage = 0;



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

// The temporary buffer of points which we use to create vertices. Will be 2 + n_parallels * n_meridians large
layout(std430, set = 0, binding = 1) buffer Points {
    vec4 data[];
} points;

// The output buffer of vertices. There will be space for n_meridians + 2 * ((n_parallels - 2) * n_meridians) + n_meridians
layout(std430, set = 0, binding = 2) buffer Vertices {
    Vertex data[];
} vertices;



/* The entry point to the first shader stage: we simply generate all points, including the north & south poles. */
void generate_points() {
    // Get the circle & point on it we're supposed to generate
	uint x = gl_GlobalInvocationID.x;
	uint y = gl_GlobalInvocationID.y;
    uint max_x = sphere.n_meridians;
    uint max_y = sphere.n_parallels;

    // Switch to the correct generation algorithm
    if (x == 0 && y == 0) {
        // Generate the north pole point
        points.data[0] = vec4(sphere.center + vec3(0.0, sphere.radius, 0.0), float(y));
    } else if (x == 0 && y == max_y - 1) {
        // Generate the south pole point
        points.data[1 + y * max_x] = vec4(sphere.center - vec3(0.0, sphere.radius, 0.0), float(y));
    } else if (x < max_x && (y > 0 && y < max_y - 1)) {
        // Generate points in between the caps

        // Pre-compute some values as float
        float fx = float(x);
        float fy = float(y);
        float fmax_x = float(max_x);
        float fmax_y = float(max_y);

        // All other instances compute a point in between
        vec3 point = vec3(
            sin(M_PI * (fy / fmax_y)) * cos(2 * M_PI * (fx / fmax_x)),
            cos(M_PI * (fy / fmax_y)),
            sin(M_PI * (fy / fmax_y)) * sin(2 * M_PI * (fx / fmax_x))
        );

        // Don't forget to scale to the appropriate size & offset
        points.data[1 + (y - 1) * max_x + x] = vec4(sphere.radius * point + sphere.center, float(y));
    }
}



/* Entry point to the second shade stage: we take the points computed before, and convert them to real Vertex structs. */
void generate_vertices() {
    // Get the circle & point on it we're supposed to generate
	uint x = gl_GlobalInvocationID.x;
	uint y = gl_GlobalInvocationID.y + 1;
    uint max_x = sphere.n_meridians;
    uint max_y = sphere.n_parallels;

    // Switch to the correct generation algorithm based on the y value
    // Note that we do not launch for the north pole
    if (y == 1) {
        // Layer below the north cap
        
        // Get the polar point
        vec3 north = points.data[0].xyz;
        // Get the previous point (possibly wrap around)
        vec3 l1;
        if (x == 0) { l1 = points.data[1 + max_x - 1].xyz; }
        else { l1 = points.data[1 + x - 1].xyz; }
        // Get the current point
        vec3 l2 = points.data[1 + x].xyz;

        // Compute the normal for these fellas
        vec3 normal = normalize(cross(l2 - north, l1 - north));

        // Store as vertex
        vertices.data[x].p1 = north;
        vertices.data[x].p2 = l1;
        vertices.data[x].p3 = l2;
        vertices.data[x].normal = normal;
        vertices.data[x].color = sphere.color;

    } else if (y < max_y - 1) {
        // Layer below another layer

        // Compute some indices first
        uint p_prev_circle = 1 + (y - 2) * max_x;
        uint p_this_circle = 1 + (y - 1) * max_x;
        uint v_this_circle = max_x + 2 * (y - 2) * max_x;

        // First, collect all points
        // Get the previous point on the previous layer, possibly wrapping around
        vec3 pc_pp;
        if (x == 0) { pc_pp = points.data[p_prev_circle + max_x - 1].xyz; }
        else { pc_pp = points.data[p_prev_circle + x - 1].xyz; }
        // Get this point on the previous layer
        vec3 pc_tp = points.data[p_prev_circle + x].xyz;
        // Get the previous point on this layer
        vec3 tc_pp;
        if (x == 0) { tc_pp = points.data[p_this_circle + max_x - 1].xyz; }
        else { tc_pp = points.data[p_this_circle + x - 1].xyz; }
        // Get this point on this layer
        vec3 tc_tp = points.data[p_this_circle + x].xyz;

        // Compute the two normals for the two vertices
        vec3 normal1 = normalize(cross(tc_tp - pc_pp, tc_pp - pc_pp));
        vec3 normal2 = normalize(cross(tc_tp - pc_pp, pc_tp - pc_pp));

        // Store them both
        vertices.data[v_this_circle + x].p1 = pc_pp;
        vertices.data[v_this_circle + x].p2 = tc_pp;
        vertices.data[v_this_circle + x].p3 = tc_tp;
        vertices.data[v_this_circle + x].normal = normal1;
        vertices.data[v_this_circle + x].color = sphere.color;

        vertices.data[v_this_circle + x + 1].p1 = pc_pp;
        vertices.data[v_this_circle + x + 1].p2 = pc_tp;
        vertices.data[v_this_circle + x + 1].p3 = tc_tp;
        vertices.data[v_this_circle + x + 1].normal = normal2;
        vertices.data[v_this_circle + x + 1].color = sphere.color;
        
    } else {
        // South cap

        // Compute some indices first
        uint p_prev_circle = 1 + (y - 2) * max_x;

        // Get the polar point
        vec3 south = points.data[y * max_x].xyz;
        // Get the previous point (possibly wrap around)
        vec3 l1;
        if (x == 0) { l1 = points.data[p_prev_circle + max_x - 1].xyz; }
        else { l1 = points.data[p_prev_circle + x - 1].xyz; }
        // Get the current point
        vec3 l2 = points.data[p_prev_circle + x].xyz;

        // Compute the normal for these fellas
        vec3 normal = normalize(cross(l2 - south, l1 - south));

        // Store as vertex
        vertices.data[x].p1 = south;
        vertices.data[x].p2 = l1;
        vertices.data[x].p3 = l2;
        vertices.data[x].normal = normal;
        vertices.data[x].color = sphere.color;
    }
}



/* The actual entry point. Uses a push constant to decide which of the two kernels to run. */
void main() {
    if (stage == 0) {
        generate_points();
    } else if (stage == 1) {
        generate_vertices();
    }
}
