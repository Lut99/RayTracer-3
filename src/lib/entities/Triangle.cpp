/* TRIANGLE.cpp
 *   by Lut99
 *
 * Created:
 *   01/05/2021, 13:35:10
 * Last edited:
 *   25/05/2021, 17:23:26
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Contains code to handle a simple triangle.
**/

#include <cmath>
#include "debugger/CppDebugger.hpp"

#include "Triangle.hpp"

using namespace std;
using namespace RayTracer;
using namespace RayTracer::ECS;
using namespace CppDebugger::SeverityValues;


/***** SPHERE FUNCTIONS *****/
/* Creates a new Triangle struct based on the given properties. */
Triangle* ECS::create_triangle(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec3& color) {
    DENTER("ECS::create_triangle");

    // Allocate the struct
    Triangle* result = new Triangle;

    // Set the RenderEntity fields
    result->type = EntityType::et_triangle;
    result->pre_render_mode = EntityPreRenderModeFlags::eprmf_cpu;
    result->pre_render_operation = EntityPreRenderOperation::epro_generate_triangle;
    // Compute how many faces & vertices to generate
    result->pre_render_faces = 1;
    result->pre_render_vertices = 3;

    // Set all points
    result->points[0] = p1;
    result->points[1] = p2;
    result->points[2] = p3;

    // Set the normal & color
    result->normal = glm::normalize(glm::cross(p3 - p1, p2 - p1));
    result->color = color;

    // Done, return it
    DRETURN result;
}



/* Pre-renders the sphere on the CPU, single-threaded. */
void ECS::cpu_pre_render_triangle(Tools::Array<GFace>& faces_buffer, Tools::Array<glm::vec4>& vertex_buffer, Triangle* triangle) {
    DENTER("ECS::cpu_pre_render_triangle");
    DLOG(info, "Pre-rendering triangle...");

    // Copy the points
    vertex_buffer[0] = glm::vec4(triangle->points[0], 0.0);
    vertex_buffer[1] = glm::vec4(triangle->points[1], 0.0);
    vertex_buffer[2] = glm::vec4(triangle->points[2], 0.0);

    // We set one vertex
    faces_buffer = {{
        0, 1, 2,
        triangle->normal,
        triangle->color
    }};

    // Done!
    DRETURN;
}
