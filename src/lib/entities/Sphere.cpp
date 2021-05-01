/* SPHERE.cpp
 *   by Lut99
 *
 * Created:
 *   01/05/2021, 12:45:50
 * Last edited:
 *   01/05/2021, 13:38:27
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
using namespace CppDebugger::SeverityValues;


/***** SPHERE FUNCTIONS *****/
/* Pre-renders the sphere on the CPU, single-threaded. */
void ECS::cpu_pre_render_sphere(Tools::Array<uint32_t>& indices, Tools::Array<glm::vec4>& vertices, Tools::Array<glm::vec4>& normals, Tools::Array<glm::vec4>& colors, const Sphere& sphere) {
    DENTER("ECS::cpu_pre_render_sphere");

    // We implement a standard / UV sphere for simplicity
    Tools::Array<glm::vec3> points(sphere.n_parallels * (sphere.n_meridians - 1));
    for (uint32_t p = 0; p < sphere.n_parallels; p++) {
        for (uint32_t m = 0; m < sphere.n_meridians - 1; m++) {
            // Generate a single dot on this location
            glm::vec3 dot(
                sinf(M_PI * ((float) p / (float) sphere.n_parallels)) * cosf(2 * M_PI * ((float) m / (float) sphere.n_meridians)) * sphere.radius,
                sinf(M_PI * ((float) p / (float) sphere.n_parallels)) * sinf(2 * M_PI * ((float) m / (float) sphere.n_meridians)) * sphere.radius,
                cosf(M_PI * ((float) p / (float) sphere.n_parallels)) * sphere.radius
            );

            // Translate the dot to the correct origin point
            dot += sphere.center;

            // Add it to the list of vertices
            points.push_back(dot);
        }
    }

    // Once we have a list of points, use that to generate vertices
    for (uint32_t p = 0; p < points.size(); p++) {
        
    }

    // Done!
    DRETURN;
}
