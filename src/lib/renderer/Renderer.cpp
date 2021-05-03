/* RENDERER.cpp
 *   by Lut99
 *
 * Created:
 *   30/04/2021, 13:17:35
 * Last edited:
 *   03/05/2021, 15:53:17
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
Renderer::Renderer(const Renderer& other) :
    entity_cvertices(other.entity_cvertices),
    entity_cpoints(other.entity_cpoints),
    entity_gvertices(other.entity_gvertices),
    entity_gpoints(other.entity_gpoints)
{
    /* Does nothing */
}

/* Move constructor for the Renderer baseclass. */
Renderer::Renderer(Renderer&& other) :
    entity_cvertices(other.entity_cvertices),
    entity_cpoints(other.entity_cpoints),
    entity_gvertices(other.entity_gvertices),
    entity_gpoints(other.entity_gpoints)
{
    /* Does nothing */
}

/* Virtual destructor for the Renderer baseclass. */
Renderer::~Renderer() {
    /* Does nothing */
}



/* Given a list of vertices pre-rendered from an entity, injects them into the list of points and indexed list of CVertices. */
void Renderer::insert_cvertices(Tools::Array<CVertex>& cvertices, Tools::Array<glm::vec3>& points, const Array<Vertex>& vertices) {
    DENTER("Renderer::insert_cvertices");

    // Regardless of how we pre-rendered, we can now condense the resulting arrays in indexed equivalents
    for (size_t i = 0; i < vertices.size(); i++) {
        // For each of the points, see if there are existing entries for them
        uint32_t indices[] = { numeric_limits<uint32_t>::max(), numeric_limits<uint32_t>::max(), numeric_limits<uint32_t>::max() };
        for (size_t lp = 0; lp < 3; lp++) {
            // Get a reference to the correct point
            const glm::vec3& point = vertices[i].points[lp];

            // Search the point list to see if it indeed exists
            for (size_t gp= 0; gp < points.size(); gp++) {
                if (points[gp] == point) {
                    // It does; store this index
                    indices[lp] = static_cast<uint32_t>(gp);
                    break;
                }
            }

            // If it does not, append it instead
            if (indices[lp] == numeric_limits<uint32_t>::max()) {
                indices[lp] = points.size();
                points.push_back(point);
            }
        }

        // Now, we have enough to create a new GVertex
        cvertices.push_back({
            indices[0], indices[1], indices[2],
            vertices[i].normal,
            vertices[i].color
        });
    }

    // Done
    DRETURN;
}

/* Given a list of vertices pre-rendered from an entity, injects them into the list of points and indexed list of GVertices. */
void Renderer::insert_gvertices(Tools::Array<GVertex>& gvertices, Tools::Array<glm::vec4>& points, const Array<Vertex>& vertices) {
    DENTER("Renderer::insert_gvertices");

    // Regardless of how we pre-rendered, we can now condense the resulting arrays in indexed equivalents
    for (size_t i = 0; i < vertices.size(); i++) {
        // For each of the points, see if there are existing entries for them
        uint32_t indices[] = { numeric_limits<uint32_t>::max(), numeric_limits<uint32_t>::max(), numeric_limits<uint32_t>::max() };
        for (size_t lp = 0; lp < 3; lp++) {
            // Get a vec4-equivalent of the correct point
            glm::vec4 point = glm::vec4(vertices[i].points[lp], 0.0);

            // Search the point list to see if it indeed exists
            for (size_t gp= 0; gp < points.size(); gp++) {
                if (points[gp] == point) {
                    // It does; store this index
                    indices[lp] = static_cast<uint32_t>(gp);
                    break;
                }
            }

            // If it does not, append it instead
            if (indices[lp] == numeric_limits<uint32_t>::max()) {
                indices[lp] = points.size();
                points.push_back(point);
            }
        }

        // Now, we have enough to create a new GVertex
        gvertices.push_back({
            indices[0], indices[1], indices[2],
            glm::vec4(vertices[i].normal, 0.0),
            glm::vec4(vertices[i].color, 0.0)
        });
    }

    // Done
    DRETURN;
}



/* Swap operator for the Renderer baseclass. */
void RayTracer::swap(Renderer& r1, Renderer& r2) {
    using std::swap;

    swap(r1.entity_cvertices, r2.entity_cvertices);
    swap(r1.entity_cpoints, r2.entity_cpoints);
    swap(r1.entity_gvertices, r2.entity_gvertices);
    swap(r1.entity_gpoints, r2.entity_gpoints);
}
