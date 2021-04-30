/* RENDERER.cpp
 *   by Lut99
 *
 * Created:
 *   30/04/2021, 13:17:35
 * Last edited:
 *   30/04/2021, 13:33:39
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

#include <CppDebugger.hpp>

#include "Renderer.hpp"

using namespace std;
using namespace RayTracer;
using namespace CppDebugger::SeverityValues;


/***** RENDERER BASECLASS *****/
/* Protected constructor for the Renderer Baseclass, which is used by derived classes to initialize the base elements. */
Renderer::Renderer() {
    /* Does nothing */
}

/* Copy constructor for the Renderer baseclass. */
Renderer::Renderer(const Renderer& other) {
    /* Does nothing */
}

/* Move constructor for the Renderer baseclass. */
Renderer::Renderer(Renderer&& other) {
    /* Does nothing */
}

/* Virtual destructor for the Renderer baseclass. */
Renderer::~Renderer() {
    /* Does nothing */
}



/* Swap operator for the Renderer baseclass. */
void RayTracer::swap(Renderer& r1, Renderer& r2) {
    /* Does nothing */
}
