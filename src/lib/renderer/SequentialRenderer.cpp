/* SEQUENTIAL RENDERER.cpp
 *   by Lut99
 *
 * Created:
 *   03/05/2021, 15:25:06
 * Last edited:
 *   25/05/2021, 18:14:13
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Simplest implementation of the Renderer class, which just renders a
 *   frame sequentially on the CPU, no fancy strings attached.
**/

#include <CppDebugger.hpp>

#include "entities/Triangle.hpp"
#include "entities/Sphere.hpp"
#include "entities/Object.hpp"

#include "SequentialRenderer.hpp"

using namespace std;
using namespace RayTracer;
using namespace RayTracer::ECS;
using namespace CppDebugger::SeverityValues;


/***** MACROS *****/
/* Macro that implements a very fast dot-product between two vec3 vectors. */
#define dot3(V1, V2) \
    ((V1).x * (V2).x + (V1).y * (V2).y + (V1).z * (V2).z)





/***** RAYTRACING FUNCTIONS *****/
/* @brief Computes the color of a pixel, as if a ray was shot out of it and it could have hit any of the faces in our mesh.
 * @param faces The list of faces against we may hit.
 * @param vertices The list of vertices referenced by the faces.
 * @param origin The origin of the ray.
 * @param direction The (normalized) direction of the ray.
 * @return The color as a three-dimensional vector.
 */
static glm::vec3 ray_color(const Tools::Array<GFace>& faces, const Tools::Array<glm::vec4>& vertices, glm::vec3 origin, glm::vec3 direction) {
    DENTER("ray_color");

    // Loop through the vertices so find any one we hit
    uint min_i = 0;
    float min_t = 1e99;
    for (size_t i = 0; i < faces.size(); i++) {
        // First, check if the ray happens to be perpendicular to the triangle's plane
        glm::vec3 normal = faces[i].normal;
        if (dot3(direction, normal) == 0) {
            // No intersection for sure
            continue;
        }

        // Otherwise, fetch the points from the point list
        glm::vec3 p1 = vertices[faces[i].v1];
        glm::vec3 p2 = vertices[faces[i].v2];
        glm::vec3 p3 = vertices[faces[i].v3];

        // Otherwise, compute the distance point of the plane
        float plane_distance = dot3(normal, p1);

        // Use that to compute the distance the ray travels before it hits the plane
        float t = (dot3(normal, origin) + plane_distance) / dot3(normal, direction);
        if (t < 0 || t >= min_t) {
            // Negative t (the face is behind us) or a t further than one we already found, so no need in doing the close check
            continue;
        }

        // Now, compute the actual point where we hit the plane
        glm::vec3 hitpoint = origin + t * direction;

        // We now perform the inside-out test to see if the triangle is hit within the plane
        // General idea: https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/barycentric-coordinates
        // First, compute various cross products
        glm::vec3 a = glm::cross(p2 - p1, hitpoint - p1);
        glm::vec3 b = glm::cross(p3 - p2, hitpoint - p2);
        glm::vec3 c = glm::cross(p1 - p3, hitpoint - p3);

        // Use them to check if the hitpoint is in the area
        if (-dot3(normal, a) >= 0.0 &&
            -dot3(normal, b) >= 0.0 &&
            -dot3(normal, c) >= 0.0)
        {
            // It's a hit! Store it as the closest t so far
            min_i = i;
            min_t = t;
            continue;
        }

        // Otherwise, no hit
    }

    // If we hit a vertex, return its color
    if (min_t < 1e99) {
        DRETURN faces[min_i].color;
    } else {
        // Return the blue sky
        glm::vec3 unit_direction = direction / length(direction);
        float t = 0.5 * (unit_direction.y + 1.0);
        DRETURN glm::vec3((1.0f - t) * glm::vec3(1.0) + t * glm::vec3(0.5, 0.7, 1.0));
    }
}

/* @brief Debug algorithm, that prints dots on the given vertices instead of rendering the faces.
 * @param faces The list of faces we thus don't render, and is here only for compatibility with ray_color. Not actually used.
 * @param vertices The list of vertices to print dots for.
 * @param origin The origin of the ray.
 * @param direction The (normalized) direction of the ray.
 * @return The color of the ray/pixel as a three-dimensional vector.
 */
static glm::vec3 ray_dot(const Tools::Array<GFace>& faces, const Tools::Array<glm::vec4>& vertices, glm::vec3 origin, glm::vec3 direction) {
    DENTER("ray_dot");
    (void) faces;    

    // Determines the radius for all dots
    const constexpr float dot_radius = 0.05;
    // Determines the distance before dots go fully black
    const constexpr float black_distance = 4.0;

    // Loop through all vertices to find the closest one
    float min_t = 1e99;
    for (size_t i = 0; i < vertices.size(); i++) {
        const glm::vec3& vertex = vertices[i];

        // Compute if the ray hits this "sphere" using the abc-formula
        glm::vec3 oc = origin - vertex;
        float a = glm::dot(direction, direction);
        float b = 2.0 * glm::dot(oc, direction);
        float c = glm::dot(oc, oc) - dot_radius * dot_radius;
        float D = b * b - 4 * a * c;
        if (D >= 0) {
            float t = (-b - sqrtf(D)) / (2.0 * a);
            if (t < min_t) {
                min_t = t;
            }
        }
    }

    // If we found a t, return the (possibly darker) pixel
    if (min_t < 1e99) {
        DRETURN std::max(1.0f - min_t / black_distance, 0.0f) * glm::vec3(1.0, 0.0, 0.0);
    } else {
        // Return the background
        glm::vec3 unit_direction = direction / length(direction);
        float t = 0.5 * (unit_direction.y + 1.0);
        DRETURN glm::vec3((1.0f - t) * glm::vec3(1.0) + t * glm::vec3(0.5, 0.7, 1.0));
    }
}





/***** SEQUENTIALRENDERER CLASS *****/
/* Constructor for the SequentialRenderer class. */
SequentialRenderer::SequentialRenderer() :
    Renderer()
{
    DENTER("VulkanRenderer::VulkanRenderer");
    DLOG(info, "Initializing the sequential renderer...");
    DLEAVE;   
}



/* Helper function that merges the given newly pre-rendered faces & vertex buffers and inserts them in the global buffers. */
void SequentialRenderer::transfer_entity(Tools::Array<GFace>& faces_buffer, Tools::Array<glm::vec4>& vertex_buffer, const Tools::Array<GFace>& new_faces_buffer, const Tools::Array<glm::vec4>& new_vertex_buffer) {
    DENTER("SequentialRenderer::transfer_entity");

    // Loop to add the faces, with new offsets
    uint32_t offset = vertex_buffer.size();
    faces_buffer.reserve(faces_buffer.size() + new_faces_buffer.size());
    for (size_t i = 0; i < new_faces_buffer.size(); i++) {
        // Push to the back of the global buffer
        faces_buffer.push_back(new_faces_buffer[i]);

        // Offset the indices in the faces buffer
        faces_buffer[faces_buffer.size() - 1].v1 += offset;
        faces_buffer[faces_buffer.size() - 1].v2 += offset;
        faces_buffer[faces_buffer.size() - 1].v3 += offset;
    }

    // We can append the vertex buffer immediately
    vertex_buffer += new_vertex_buffer;

    // Done!
    DRETURN;
}



/* Pre-renders the given list of RenderEntities, not accelerated in any way lmao. */
void SequentialRenderer::prerender(const Tools::Array<ECS::RenderEntity*>& entities) {
    DENTER("SequentialRenderer::prerender");
    DLOG(info, "Pre-rendering entities...");
    DINDENT;

    // We start by throwing out any old vertices we might have
    this->entity_faces.clear();
    this->entity_vertices.clear();

    // Prepare the buffers for the per-entity vertices
    Tools::Array<GFace> entity_faces;
    Tools::Array<glm::vec4> entity_vertices;

    // Next, loop through all entities to render them
    for (size_t i = 0; i < entities.size(); i++) {
        // Select the proper pre-render mode (only CPU is supported)
        if (entities[i]->pre_render_mode & EntityPreRenderModeFlags::eprmf_cpu) {
            // Clear the buffers and set them to the correct size
            entity_faces.clear();
            entity_vertices.clear();
            entity_faces.resize(entities[i]->pre_render_faces);
            entity_vertices.resize(entities[i]->pre_render_vertices);
            
            // Determine the type of pre-rendering operation we need to do
            switch (entities[i]->pre_render_operation) {
                case EntityPreRenderOperation::epro_generate_triangle:
                    /* Call the generate triangle CPU function. */
                    cpu_pre_render_triangle(entity_faces, entity_vertices, (Triangle*) entities[i]);
                    break;

                case EntityPreRenderOperation::epro_generate_sphere:
                    /* Call the generate sphere CPU function. */
                    cpu_pre_render_sphere(entity_faces, entity_vertices, (Sphere*) entities[i]);
                    break;
                
                case EntityPreRenderOperation::epro_load_object_file:
                    /* Call the load object file CPU function. */
                    cpu_pre_render_object(entity_faces, entity_vertices, (Object*) entities[i]);
                    break;

                default:
                    DLOG(fatal, "Entity " + std::to_string(i) + " wants to be pre-rendered on the CPU using unsupported operation '" + entity_pre_render_operation_names[entities[i]->pre_render_operation] + "'.");

            }

            // Once done, merge them in the general lists
            this->transfer_entity(this->entity_faces, this->entity_vertices, entity_faces, entity_vertices);      

        } else {
            DLOG(fatal, "Entity " + std::to_string(i) + " of type " + entity_type_names[entities[i]->type] + " with the sequential back-end.");
        }

        // No need to insert again, as transfer_entity does this for us now
    }

    // cout << "Faces:" << endl;
    // for (size_t i = 0; i < this->entity_faces.size(); i++) {
    //     glm::vec3 v1 = this->entity_vertices[this->entity_faces[i].v1];
    //     glm::vec3 v2 = this->entity_vertices[this->entity_faces[i].v2];
    //     glm::vec3 v3 = this->entity_vertices[this->entity_faces[i].v3];
    //     cout << "    x: {" << v1.x << "," << v1.y << "," << v1.z << "}" << " | y: {" << v2.x << "," << v2.y << "," << v2.z << "} | z: {" << v3.x << "," << v3.y << "," << v3.z << "}" << endl;
    // }

    // We're done! We pre-rendered all objects!
    DDEDENT;
    DRETURN;
}

/* Renders the internal list of vertices to a frame using the given camera position. */
void SequentialRenderer::render(Camera& camera) const {
    DENTER("SequentialRenderer::render");
    // Print some info
    DLOG(info, "Rendering for camera:");
    DINDENT;
    DLOG(auxillary, "Camera origin            : (" + std::to_string(camera.origin.x) + "," + std::to_string(camera.origin.y) + "," + std::to_string(camera.origin.z) + ")");
    DLOG(auxillary, "Camera horizontal        : (" + std::to_string(camera.horizontal.x) + "," + std::to_string(camera.horizontal.y) + "," + std::to_string(camera.horizontal.z) + ")");
    DLOG(auxillary, "Camera vertical          : (" + std::to_string(camera.vertical.x) + "," + std::to_string(camera.vertical.y) + "," + std::to_string(camera.vertical.z) + ")");
    DLOG(auxillary, "Camera lower_left_corner : (" + std::to_string(camera.lower_left_corner.x) + "," + std::to_string(camera.lower_left_corner.y) + "," + std::to_string(camera.lower_left_corner.z) + ")");
    DDEDENT;

    // Loop through all pixels to render
    // Due to unsignedness, we use this funky operator which is '(y--) > 0'
    DLOG(info, "Rendering...");
    DINDENT;
    uint32_t width = camera.w(), height = camera.h();
    uint32_t i = 0;
    for (uint32_t y = height - 1; y --> 0 ;) {
        for (uint32_t x = 0; x < width; x++) {
            // Compute the u & v, which is basically the ray's coordinates as a float
            float u = float(x) / (float(width) - 1.0);
            float v = float(height - 1 - y) / (float(height) - 1.0);

            // Compute the ray itself
            glm::vec3 ray = camera.lower_left_corner + u * camera.horizontal + v * camera.vertical - camera.origin;

            // Compute the ray's color and store it as a vector
            camera.get_frame().d()[y * width + x] = ray_color(this->entity_faces, this->entity_vertices, camera.origin, ray);
            (void) ray_dot;

            if (i % 1000 == 0) { DLOG(info, "Rendered ray " + std::to_string(i) + "/" + std::to_string(width * height)); }
            i++;
        }
    }
    DDEDENT;

    // Done!
    DRETURN;
}





/***** FACTORY METHOD *****/
Renderer* RayTracer::initialize_renderer() {
    DENTER("initialize_renderer");

    // Initialize a new object
    SequentialRenderer* result = new SequentialRenderer();

    // Done, return
    DRETURN (Renderer*) result;
}
