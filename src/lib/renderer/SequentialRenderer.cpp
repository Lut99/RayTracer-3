/* SEQUENTIAL RENDERER.cpp
 *   by Lut99
 *
 * Created:
 *   03/05/2021, 15:25:06
 * Last edited:
 *   06/05/2021, 18:11:27
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


/***** RAYTRACING FUNCTIONS *****/
glm::vec3 ray_color(const Tools::Array<GFace>& faces, const Tools::Array<glm::vec4>& vertices, glm::vec3 origin, glm::vec3 direction) {
    DENTER("ray_color");

    // Loop through the vertices so find any one we hit
    uint min_i = 0;
    float min_t = INFINITY;
    for (size_t i = 0; i < faces.size(); i++) {
        // First, check if the ray happens to be perpendicular to the triangle's plane
        glm::vec3 normal = faces[i].normal;
        if (dot(direction, normal) == 0) {
            // No intersection for sure
            continue;
        }

        // Otherwise, fetch the points from the point list
        glm::vec3 p1 = vertices[faces[i].v1];
        glm::vec3 p2 = vertices[faces[i].v2];
        glm::vec3 p3 = vertices[faces[i].v3];

        // Otherwise, compute the distance point of the plane
        float plane_distance = dot(normal, p1);

        // Use that to compute the distance the ray travels before it hits the plane
        float t = (dot(normal, origin) + plane_distance) / dot(normal, direction);
        if (t < 0 || t >= min_t) {
            // Negative t or a t further than one we already found as closer, so we hit the triangle behind us
            continue;
        }

        // Now, compute the actual point where we hit the plane
        glm::vec3 hitpoint = origin + t * direction;

        // We can now compute barycentric coordinates from the hitpoint to see if the point is also within the triangle
        // General idea: https://stackoverflow.com/a/37552406/5270125
        // First, we compute alpha by finding the area of the triangle spanned by two edges of the triangle and the point we want to find
        float S = 0.5 * glm::length(glm::cross(p1 - p2, p1 - p3));
        float alpha = (0.5 * glm::length(glm::cross(p2 - hitpoint, p2 - p3))) / S;
        float beta = (0.5 * glm::length(glm::cross(hitpoint - p1, hitpoint - p3))) / S;
        float gamma = (0.5 * glm::length(glm::cross(hitpoint - p1, hitpoint - p2))) / S;

        // With this, we can perform the actual bounds-check
        if (alpha >= 0 && alpha <= 1 &&
            beta  >= 0 && beta  <= 1 &&
            gamma >= 0 && gamma <= 1)
        {
            // It's a hit! Store it as the closest t so far
            min_i = i;
            min_t = t;
            continue;
        }

        // Otherwise, no hit
    }

    // If we hit a vertex, return its color
    if (min_t != INFINITY) {
        DRETURN faces[min_i].color;
    } else {
        // Return the blue sky
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



/* Pre-renders the given list of RenderEntities, not accelerated in any way lmao. */
void SequentialRenderer::prerender(const Tools::Array<ECS::RenderEntity*>& entities) {
    DENTER("SequentialRenderer::prerender");
    DLOG(info, "Pre-rendering entities...");
    DINDENT;

    // We start by throwing out any old vertices we might have
    this->entity_faces.clear();
    this->entity_vertices.clear();

    // Prepare the buffers for the per-entity vertices
    Tools::Array<Face> faces;
    Tools::Array<GFace> gfaces;
    Tools::Array<glm::vec4> gvertices;

    // Next, loop through all entities to render them
    for (size_t i = 0; i < entities.size(); i++) {
        // Clean the temporary buffers
        faces.clear();
        gfaces.clear();
        gvertices.clear();

        // Select the proper pre-render mode (only CPU is supported)
        if (entities[i]->pre_render_mode & EntityPreRenderModeFlags::eprmf_cpu) {
            // Determine the type of pre-rendering operation we need to do
            switch (entities[i]->pre_render_operation) {
                case EntityPreRenderOperation::epro_generate_triangle:
                    /* Call the generate triangle CPU function. */
                    cpu_pre_render_triangle(faces, (Triangle*) entities[i]);
                    break;

                case EntityPreRenderOperation::epro_generate_sphere:
                    /* Call the generate sphere CPU function. */
                    cpu_pre_render_sphere(faces, (Sphere*) entities[i]);
                    break;
                
                case EntityPreRenderOperation::epro_load_object_file:
                    /* Call the load object file CPU function. */
                    cpu_pre_render_object(gfaces, gvertices, (Object*) entities[i]);
                    break;

                default:
                    DLOG(fatal, "Entity " + std::to_string(i) + " wants to be pre-rendered on the CPU using unsupported operation '" + entity_pre_render_operation_names[entities[i]->pre_render_operation] + "'.");

            }

        } else {
            DLOG(fatal, "Entity " + std::to_string(i) + " of type " + entity_type_names[entities[i]->type] + " with the sequential back-end.");
        }

        // Insert the generated list into the global list. However, we do different things based on different operations
        if (entities[i]->pre_render_operation == EntityPreRenderOperation::epro_load_object_file) {
            // Just append the vertices and indices to the list
            this->append_faces(this->entity_faces, this->entity_vertices, gfaces, gvertices);
        } else {
            // Strip all non-unique vertices and generate new indices
            this->insert_faces(this->entity_faces, this->entity_vertices, faces);
        }
    }

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
