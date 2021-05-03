/* SPHERE.cpp
 *   by Lut99
 *
 * Created:
 *   01/05/2021, 12:45:50
 * Last edited:
 *   03/05/2021, 18:19:36
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Contains code for pre-rendering a Sphere on the CPU (single threaded).
**/

#include <cmath>
#include <CppDebugger.hpp>

#include "Sphere.hpp"

using namespace std;
using namespace RayTracer;
using namespace RayTracer::ECS;
using namespace CppDebugger::SeverityValues;


/***** SPHERE FUNCTIONS *****/
/* Creates a new Sphere struct based on the given properties. */
Sphere* ECS::create_sphere(const glm::vec3& center, float radius, uint32_t n_meridians, uint32_t n_parallels, const glm::vec3& color) {
    DENTER("ECS::create_sphere");

    // Allocate the struct
    Sphere* result = new Sphere;

    // Set the RenderEntity fields
    result->type = EntityType::et_sphere;
    result->pre_render_mode = EntityPreRenderModeFlags::eprmf_cpu;
    result->pre_render_operation = EntityPreRenderOperation::epro_generate_sphere;

    // Set the conceptual properties of the sphere
    result->center = center;
    result->radius = radius;

    // Set the rendering properties of the sphere
    result->n_meridians = n_meridians;
    result->n_parallels = n_parallels;
    result->color = color;

    // Done!
    DRETURN result;
}



/* Pre-renders the sphere on the CPU, single-threaded. */
void ECS::cpu_pre_render_sphere(Tools::Array<Vertex>& vertices, Sphere* sphere) {
    DENTER("ECS::cpu_pre_render_sphere");
    DLOG(info, "Pre-rendering sphere with " + std::to_string(sphere->n_meridians) + " meridians and " + std::to_string(sphere->n_parallels) + " parallels...");

    // We implement a standard / UV sphere for simplicity
    float triangle_radius = 0.008f;
    uint32_t total = sphere->n_parallels * sphere->n_meridians;
    vertices.reserve(sphere->n_parallels * sphere->n_meridians);

    // First, define two buffers used to generate the vertices
    Tools::Array<glm::vec3> prev_circle(sphere->n_meridians);
    Tools::Array<glm::vec3> circle(sphere->n_meridians);

    // // Compute the points of the marker triangle around this dot
    // glm::vec3 p1 = glm::vec3(
    //     sphere->center.x - triangle_radius,
    //     sphere->center.y + glm::vec3({ 0.0, sphere->radius, 0.0 }).y,
    //     sphere->center.z
    // );
    // glm::vec3 p2 = glm::vec3(
    //     sphere->center.x,
    //     sphere->center.y + glm::vec3({ 0.0, sphere->radius, 0.0 }).y + triangle_radius,
    //     sphere->center.z
    // );
    // glm::vec3 p3 = glm::vec3(
    //     sphere->center.x + triangle_radius,
    //     sphere->center.y + glm::vec3({ 0.0, sphere->radius, 0.0 }).y,
    //     sphere->center.z
    // );

    // // Append to the list of vertices
    // vertices.push_back({
    //     p1, p2, p3,
    //     glm::normalize(glm::cross(p3 - p1, p2 - p1)),
    //     { 0.0f, 0.0f, 0.0f }
    // });

    // // Compute the points of the marker triangle around this dot
    // p1 = glm::vec3(
    //     sphere->center.x - triangle_radius,
    //     sphere->center.y,
    //     sphere->center.z
    // );
    // p2 = glm::vec3(
    //     sphere->center.x,
    //     sphere->center.y + triangle_radius,
    //     sphere->center.z
    // );
    // p3 = glm::vec3(
    //     sphere->center.x + triangle_radius,
    //     sphere->center.y,
    //     sphere->center.z
    // );

    // // Append to the list of vertices
    // vertices.push_back({
    //     p1, p2, p3,
    //     glm::normalize(glm::cross(p3 - p1, p2 - p1)),
    //     { 0.0f, 0.0f, 0.0f }
    // });

    // For all circles that are not poles, let's generate them
    for (uint32_t p = 1; p < sphere->n_parallels - 1; p++) {
        for (uint32_t m = 0; m < sphere->n_meridians; m++) {
            // Generate a single dot on this location
            glm::vec3 dot(
                sinf(M_PI * ((float) p / (float) sphere->n_parallels)) * cosf(2 * M_PI * ((float) m / (float) sphere->n_meridians)) * sphere->radius,
                cosf(M_PI * ((float) p / (float) sphere->n_parallels)) * sphere->radius,
                sinf(M_PI * ((float) p / (float) sphere->n_parallels)) * sinf(2 * M_PI * ((float) m / (float) sphere->n_meridians)) * sphere->radius
            );
            // Translate the dot to the correct origin point
            dot += sphere->center;

            // Add it to the current circle
            circle.push_back(dot);
            
            // // Compute the points of the marker triangle around this dot
            // glm::vec3 p1 = glm::vec3(
            //     dot.x - triangle_radius,
            //     dot.y,
            //     dot.z
            // );
            // glm::vec3 p2 = glm::vec3(
            //     dot.x,
            //     dot.y + triangle_radius,
            //     dot.z
            // );
            // glm::vec3 p3 = glm::vec3(
            //     dot.x + triangle_radius,
            //     dot.y,
            //     dot.z
            // );

            // // Append to the list of vertices
            // vertices.push_back({
            //     p1, p2, p3,
            //     glm::normalize(glm::cross(p3 - p1, p2 - p1)),
            //     { 0.0f, 0.0f, 0.0f }
            // });
        }

        // With the circle complete, generate a list of triangles as seen from the previous circle
        if (p == 1) {
            // It's the toplevel circle of vertices. Just create triangle with the top point being the top of the circle.
            for (size_t i = 0; i < sphere->n_meridians; i++) {
                // Compute the points of the vertex
                glm::vec3 p1 = sphere->center + glm::vec3({ 0.0, sphere->radius, 0.0 });
                glm::vec3 p2 = i > 0 ? circle[i - 1] : circle[sphere->n_meridians - 1];
                glm::vec3 p3 = circle[i];
                glm::vec3 normal = glm::normalize(glm::cross(p3 - p1, p2 - p1));
                // glm::vec3 color = sphere->color * -glm::dot(normal, glm::vec3(0.0, 0.0, -1.0));
                // glm::vec3 color = glm::vec3((float) vertices.size() / (float) (2 + (sphere->n_parallels - 2) * sphere->n_meridians), 0.0, 0.0);
                // glm::vec3 color = glm::vec3(1.0, 0.0, 0.0) * fabs(glm::dot(normal, glm::vec3(0.0, 0.0, -1.0)));;
                glm::vec3 color = sphere->color * fabs(glm::dot(normal, glm::vec3(0.0, 0.0, -1.0)));

                // Append it to the list
                vertices.push_back({
                    p1, p2, p3,
                    normal,
                    color, 
                });
            }
        } else {
            // It's a non-toplevel circle of vertices. That means that for every pair of two points, we generate two to fill in the square
            for (size_t i = 0; i < sphere->n_meridians; i++) {
                // Compute the points of the first vertex
                glm::vec3 p1 = i > 0 ? prev_circle[i - 1] : prev_circle[sphere->n_meridians - 1];
                glm::vec3 p2 = i > 0 ? circle[i - 1] : circle[sphere->n_meridians - 1];
                glm::vec3 p3 = circle[i];
                glm::vec3 normal = glm::normalize(glm::cross(p3 - p1, p2 - p1));
                glm::vec3 color = sphere->color * fabs(glm::dot(normal, glm::vec3(0.0, 0.0, -1.0)));
                // glm::vec3 color = glm::vec3(0.0, 1.0, 0.0) * fabs(glm::dot(normal, glm::vec3(0.0, 0.0, -1.0)));
                // glm::vec3 color = glm::vec3(0.0, 1.0, 0.0);

                // Append it to the list
                vertices.push_back({
                    p1, p2, p3,
                    glm::normalize(glm::cross(p3 - p1, p2 - p1)),
                    color, 
                });

                // Compute the points of the second vertex
                p2 = prev_circle[i];
                normal = glm::normalize(glm::cross(p3 - p1, p2 - p1));
                // color = glm::vec3(0.0, 1.0, 0.0) * fabs(glm::dot(normal, glm::vec3(0.0, 0.0, -1.0)));
                color = sphere->color * fabs(glm::dot(normal, glm::vec3(0.0, 0.0, -1.0)));

                // Append it as well
                vertices.push_back({
                    p1, p2, p3,
                    glm::normalize(glm::cross(p3 - p1, p2 - p1)),
                    color, 
                });
            }
        }

        // Shuffle them around
        prev_circle = circle;
        circle.clear();
        circle.reserve(sphere->n_meridians);
    }

    // Once done, generate vertices for the soutch polar cap
    for (size_t i = 0; i < sphere->n_meridians; i++) {
        // Compute the points of the vertex
        glm::vec3 p1 = sphere->center - glm::vec3({ 0.0, sphere->radius, 0.0 });
        glm::vec3 p2 = i > 0 ? prev_circle[i - 1] : prev_circle[sphere->n_meridians - 1];
        glm::vec3 p3 = prev_circle[i];
        glm::vec3 normal = glm::normalize(glm::cross(p3 - p1, p2 - p1));
        glm::vec3 color = sphere->color * fabs(glm::dot(normal, glm::vec3(0.0, 0.0, -1.0)));
        // glm::vec3 color = glm::vec3(0.0, 0.0, 1.0) * fabs(glm::dot(normal, glm::vec3(0.0, 0.0, -1.0)));;

        // Append it to the list
        vertices.push_back({
            p1, p2, p3,
            normal,
            color, 
        });
    }

    // Done!
    DRETURN;
}
