/* RENDERER.cpp
 *   by Lut99
 *
 * Created:
 *   30/04/2021, 13:17:35
 * Last edited:
 *   25/05/2021, 17:23:26
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Implements the rendering process. More accurately, does two things:
 *   first, it takes a list of RenderEntities and returns a list of
 *   vertices, normals and colours that can be rendered. Then, it takes
 *   those lists and utiliized RayTracing to render those vertices to an
 *   image.
**/

#include <unordered_map>
#include "debugger/CppDebugger.hpp"

#include "Renderer.hpp"

using namespace std;
using namespace RayTracer;
using namespace RayTracer::ECS;
using namespace CppDebugger::SeverityValues;


/***** MACROS *****/
/* Shortcut for getting the i'th point in the face. */
#define FACE_POINT(FACE, I) \
    ((I) == 0 ? (FACE).v1 : ((I) == 1 ? (FACE).v2 : (FACE).v3))





/***** HELPER FUNCTIONS *****/
namespace std {
    template <>
    struct hash<glm::vec3> {
        size_t operator()(const glm::vec3& elem) const {
            return ((std::hash<float>{}(elem[0]) ^
                (std::hash<float>{}(elem[1]) << 1)) >> 1) ^
                (std::hash<float>{}(elem[2]) << 1);
        }
    };

    template <>
    struct hash<glm::vec4> {
        size_t operator()(const glm::vec4& elem) const {
            return ((std::hash<float>{}(elem[0]) ^
                (std::hash<float>{}(elem[1]) << 1)) >> 1) ^
                (std::hash<float>{}(elem[2]) << 1) ^
                (std::hash<float>{}(elem[3]) >> 1);
        }
    };
}





/***** RENDERER BASECLASS *****/
/* Protected constructor for the Renderer Baseclass, which is used by derived classes to initialize the base elements. */
Renderer::Renderer() {
    /* Does nothing */
}

/* Copy constructor for the Renderer baseclass. */
Renderer::Renderer(const Renderer& other) {
    /* Does nothing */
    (void) other;
}

/* Move constructor for the Renderer baseclass. */
Renderer::Renderer(Renderer&& other) {
    /* Does nothing */
    (void) other;
}

/* Virtual destructor for the Renderer baseclass. */
Renderer::~Renderer() {
    /* Does nothing */
}



/* Swap operator for the Renderer baseclass. */
void RayTracer::swap(Renderer& r1, Renderer& r2) {
    using std::swap;
    (void) r1;
    (void) r2;
}
