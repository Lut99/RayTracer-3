/* TRIANGLE.cpp
 *   by Lut99
 *
 * Created:
 *   01/05/2021, 13:35:10
 * Last edited:
 *   01/05/2021, 13:43:34
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
Triangle* ECS::create_triangle(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec3& normal, const glm::vec3& color) {
    DENTER("ECS::create_triangle");

    // Allocate the struct
    Triangle* result = new Triangle;

    // Set all points
    result->points[0] = p1;
    result->points[1] = p2;
    result->points[2] = p3;

    // Set the normal & color
    result->normal = normal;
    result->color = color;

    // Done, return it
    DRETURN result;
}



/* Pre-renders the sphere on the CPU, single-threaded. */
void ECS::cpu_pre_render_triangle(Tools::Array<uint32_t>& indices, uint32_t n_vertices, Tools::Array<glm::vec4>& vertices, Tools::Array<glm::vec4>& normals, Tools::Array<glm::vec4>& colors, const Triangle& triangle) {
    DENTER("ECS::cpu_pre_render_triangle");

    // Search the list of vertices if it exists already
    bool found = false;
    for (size_t j = 0; j < n_vertices; j++) {
        if (vertices[v] == glm::vec4(triangle.points[i], 0.0) &&
            normals[])
        {
            
        }
    }

    // Done!
    DRETURN;
}

