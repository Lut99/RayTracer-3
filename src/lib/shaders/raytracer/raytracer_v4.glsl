/* RAYTRACER V4.glsl
 *   by Lut99
 *
 * Created:
 *   25/05/2021, 20:58:29
 * Last edited:
 *   25/05/2021, 22:26:46
 * Auto updated?
 *   Yes
 *
 * Description:
 *   The fourth generation of the raytracing shader, which implements
 *   multi-sampling and supports different types of primitives.
**/

#version 450



/* Define the workgroup size(s). */
layout (local_size_x = 32, local_size_y = 32, local_size_z = 32) in;



/* Structs */
// The Face struct, which is a single face. */
struct Face {
    /* The first vertex of the face. */
    uint v1;
    /* The second vertex of the face. */
    uint v2;
    /* The third vertex of the face. */
    uint v3;

    /* The normal of the face. */
    vec3 normal;
    /* The color of the face. */
    vec3 color;  
};

// The Sphere struct, which is a single sphere. */
struct Sphere {
    /* The center of the sphere. */
    vec3 center;
    /* The radius of the sphere. */
    float radius;
    /* The color of the sphere. */
    vec3 color;
}



/* Define specialization constants. */
// The width of the target frame
layout (constant_id = 0) const int width = 0;

// The height of the target frame
layout (constant_id = 1) const int height = 0;

// The number of samples we take per pixel
layout (constant_id = 2) const int n_samples = 0;

// The maximum recursion depth for bouncing
layout (constant_id = 3) const int max_bounces = 0;



/* Define the buffers. */
// The information for the current invocation
layout(std140, set = 0, binding = 0) uniform BlockInfo {
    // The number of pixels that we are offset w.r.t. the topleft of the image
    uint x;
    // The number of pixels that we are offset w.r.t. the topleft of the image
    uint y;
    // The width of this block
    uint w;
    // The height of this block
    uint h;
} block_info;

// The input data for the camera
layout(std140, set = 1, binding = 0) uniform Camera {
    // Vector placing the origin (middle) of the camera viewport in the world
    vec3 origin;
    // Vector determining the horizontal line of the camera viewport in the game world, and also its conceptual size
    vec3 horizontal;
    // Vector determining the vertical line of the camera viewport in the game world, and also its conceptual size
    vec3 vertical;
    // Vector describing the lower left corner of the viewport. Shortcut based on the other three
    vec3 lower_left_corner;
} camera;

// The list of faces that we may hit
layout(std430, set = 2, binding = 0) buffer Faces {
    Face data[];
} faces;
// The list of vertices referenced by the faces
layout(std430, set = 2, binding = 1) buffer Vertices {
    vec3 data[];
} vertices;

// The list of spheres that we may hit
layout(std430, set = 2, binding = 2) buffer Spheres {
    Sphere data[];
} spheres;

// The output of the shader, which is a temporary storage buffer of block_info.w * block_info.h * n_samples colors that we can multisample from
layout(std430, set = 2, binding = 3) buffer SampleStorage {
    // The resulting color for this pixel
    uint pixels[];
} sample_storage;



/* Computes if the face with this given index is hit by the given ray. If so, returns the distance. If not, returns 1e99. */
float hit_vertex(uint index, vec3 origin, vec3 direction) {
    // First, check if the ray happens to be perpendicular to the triangle's plane
    vec3 normal = faces.data[i].normal;
    if (dot(direction, normal) == 0) {
        // No intersection for sure, as the ray is perpendicular
        return 1e99;
    }

    // Otherwise, fetch the points from the point list
    vec3 p1 = vertices.data[faces.data[i]];
    vec3 p2 = vertices.data[faces.data[i]];
    vec3 p3 = vertices.data[faces.data[i]];

    // Otherwise, compute the distance point of the plane
    float plane_distance = dot(normal, p1);

    // Use that to compute the distance the ray travels before it hits the plane
    float t = (dot(normal, origin) + plane_distance) / dot(normal, direction);
    if (t < 0) {
        // Negative t
        return 1e99;
    }

    // Now, compute the actual point where we hit the plane
    vec3 hitpoint = origin + t * direction;

    // We now perform the inside-out test to see if the triangle is hit within the plane
    // General idea: https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/barycentric-coordinates
    if (-dot(normal, cross(p2 - p1, hitpoint - p1)) >= 0.0 &&
        -dot(normal, cross(p3 - p2, hitpoint - p2)) >= 0.0 &&
        -dot(normal, cross(p1 - p3, hitpoint - p3)) >= 0.0)
    {
        // It's a hit! Return that that's what we did
        return t;
    }

    // If we reached here, then it wasn't a hit
    return 1e99;
}

/* Computes if the sphere with this given index is hit by the given ray. If so, returns the distance. If not, returns 1e99. */
float hit_sphere(uint index, vec3 origin, vec3 direction) {
    // Loop through all vertices to find the closest one
    vec3 vertex = ;

    // Compute if the ray hits this "sphere" using the abc-formula
    vec3 oc = origin - spheres.data[i].origin;
    float a = dot(direction, direction);
    float b = 2.0 * dot(oc, direction);
    float c = dot(oc, oc) - spheres.data[i].radius * spheres.data[i].radius;
    float D = b * b - 4 * a * c;
    if (D >= 0) {
        // Hit! But only if t is non-negative
        float t = (-b - sqrt(D)) / (2.0 * a);
        if (t >= 0.0) {
            return t;
        }
        return 1e99;
    }

    // No hit
    return 1e99;
}



/* The entry point to the shader. */
void main() {		
    // Get the index we're supposed to render
	uint x = gl_GlobalInvocationID.x;
	uint y = gl_GlobalInvocationID.y;
    uint z = gl_GlobalInvocationID.z;

    // Only continue if this instance is within range of our frame
    if (x < width && y < height && z < n_samples) {
        /* Step 1: Ray preparation. */
        // Compute the u & v, which are normalized x & y
        float u = float((block_info.x + x)) / (float(width) - 1.0);
        float v = float(height - 1 - (block_info.y + y)) / (float(height) - 1.0);

        // Give it some small offset based on the z and the amount of samples
        if (n_samples > 1) {
            // First, compute the size of each of the edges of a square that would have the number of samples as its area
            uint edge_size = sqrt(n_samples);

            // Add a fraction ranging -0.5 - 0.5 to the u that is proportionate to the index in the square given by z
            uint sx = z % edge_size;
            uint sy = z / edge_size;
            u += (float(sx) / float(edge_size - 1)) - 0.5;
            v += (float(sy) / float(edge_size - 1)) - 0.5;
        }

        // Use those to compute the starting ray
        vec3 origin = camera.origin;
        vec3 direction = camera.lower_left_corner + u * camera.horizontal + v * camera.vertical - camera.origin;

        // Finally, we set the color of the background as the initial color
        float t = 0.5 * ((direction / length(direction)).y + 1.0);
        vec3 color = (1.0 - t) * vec3(1.0) + t * vec3(0.5, 0.7, 1.0);



        /* Step 2: Tracing. */
        // Cast the ray, then keep casting until the ray doesn't bounce anymore
        for (uint i = 0; i < max_bounces; i++) {
            /* Step 2.1: Hitting. */
            // We hit in separate stages per supported primitive
            uint min_i;
            uint min_t = 1e99;
            uint min_type;

            // Vertices
            for (uint i = 0; i < faces.data.length(); i++) {
                float t = hit_vertex(i, origin, direction);
                if (t < min_t) {
                    min_i = i;
                    min_t = t;
                    min_type = 0;
                }
            }

            // Spheres
            for (uint i = 0; i < spheres.data.length(); i++) {
                float t = hit_sphere(i, origin, direction);
                if (t < min_t) {
                    min_i = i;
                    min_t = t;
                    min_type = 1;
                }
            }

            // If we did not hit anything, then we can quit
            if (min_t >= 1e99) {
                break;
            }



            /* Step 2.2: Normal computation. */
            // Compute the normal differently, depending on which type of primitive we hit
            // We do branch here, since this operation is relatively singular and because a single warp is not likely to hit that many different objects
            vec3 obj_normal;
            vec3 obj_color;
            uint obj_material;
            if (min_type == 0) {
                // For vertices, we can simply take the normal, material and color of the vertex we hit
                obj_normal = faces.data[min_i].normal;
                obj_color = faces.data[min_i].color;
                obj_material = 0;
            } else if (min_type == 1) {
                // For spheres, we will have to compute the normal based on the hitpoint (which is origin + t * direction)
                obj_normal = (origin + min_t * direction) - spheres[min_i].center;
                obj_normal = obj_normal / length(obj_normal);
                obj_color = spheres[min_i].color;
                obj_material = 0;
            }



            /* Step 2.3: Bouncing. */
            // Depending on the material, use the computed normal to bounce the ray off
            // Again we branch to to relative cheapness
            if (material == 0) {
                // To test anti-aliasing and the new system, let's not bounce just yet
                color *= obj_color;
                break;
            }
        }



        /* Step 3: Returning. */
        // We can now write the color to the temporary buffer
        sample_storage.pixels[y * width + x] = color;
    }
}
