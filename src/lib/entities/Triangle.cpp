/* TRIANGLE.cpp
 *   by Lut99
 *
 * Created:
 *   01/05/2021, 13:35:10
 * Last edited:
 *   16/05/2021, 12:45:08
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Contains code to handle a simple triangle.
**/

#include <cmath>
#include <CppDebugger.hpp>

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
void ECS::cpu_pre_render_triangle(Tools::Array<Face>& faces, Triangle* triangle) {
    DENTER("ECS::cpu_pre_render_triangle");
    DLOG(info, "Pre-rendering triangle...");

    // We set one vertex
    faces = {{
        triangle->points[0], triangle->points[1], triangle->points[2],
        triangle->normal,
        triangle->color
    }};

    // Done!
    DRETURN;
}



/* Returns the number of faces & vertices for this triangle appended to the given integers. */
void ECS::get_size_triangle(uint32_t& n_faces, uint32_t& n_vertices, Triangle* triangle) {
    DENTER("ECS::get_size_triangle");

    n_faces += 1;
    n_vertices += 3;
    (void) triangle;

    DRETURN;
}
