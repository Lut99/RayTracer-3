/* RENDERER.cpp
 *   by Lut99
 *
 * Created:
 *   30/04/2021, 13:17:35
 * Last edited:
 *   11/05/2021, 21:19:59
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
#include <CppDebugger.hpp>

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
Renderer::Renderer(const Renderer& other) :
    entity_faces(other.entity_faces),
    entity_vertices(other.entity_vertices)
{
    /* Does nothing */
}

/* Move constructor for the Renderer baseclass. */
Renderer::Renderer(Renderer&& other) :
    entity_faces(other.entity_faces),
    entity_vertices(other.entity_vertices)
{
    /* Does nothing */
}

/* Virtual destructor for the Renderer baseclass. */
Renderer::~Renderer() {
    /* Does nothing */
}



/* Given a list of faces pre-rendered from an entity, injects them into the list of vertices and indexed list of Faces. */
void Renderer::insert_faces(Tools::Array<GFace>& faces, Tools::Array<glm::vec4>& points, const Array<Face>& new_faces) {
    DENTER("Renderer::insert_faces");

    // Define an unordered map, which we use to more quickly recognize if a vertex exists or not
    std::unordered_map<glm::vec4, uint32_t> point_map;

    // Regardless of how we pre-rendered, we can now condense the resulting arrays in indexed equivalents
    point_map.reserve(new_faces.size());
    faces.reserve(faces.size() + new_faces.size());
    points.reserve(points.size() + new_faces.size());
    for (size_t i = 0; i < new_faces.size(); i++) {
        // For each of the points, see if there are existing entries for them
        uint32_t indices[] = { numeric_limits<uint32_t>::max(), numeric_limits<uint32_t>::max(), numeric_limits<uint32_t>::max() };
        for (size_t lp = 0; lp < 3; lp++) {
            // Get a vec4 equivalent of the correct point
            glm::vec4 point = glm::vec4(FACE_POINT(new_faces[i], lp), 0.0);

            // Search the point list to see if it indeed exists
            std::unordered_map<glm::vec4, uint32_t>::iterator iter = point_map.find(point);
            if (iter != point_map.end()) {
                indices[lp] = (*iter).second;
            }

            // If it does not, append it instead
            if (indices[lp] == numeric_limits<uint32_t>::max()) {
                indices[lp] = points.size();
                points.push_back(point);
                // Also update the point map
                point_map.insert(std::make_pair(point, indices[lp]));
            }
        }

        // Now, we have enough to create a new GVertex
        faces.push_back({
            indices[0], indices[1], indices[2],
            new_faces[i].normal,
            new_faces[i].color
        });

        // if (i % 100 == 0) {
        //     DLOG(info, "Examined vertex " + std::to_string(i + 1) + "/" + std::to_string(new_vertices.size()));
        //     DINDENT;
        //     DLOG(auxillary, "Current vertices length : " + std::to_string(vertices.size()));
        //     DLOG(auxillary, "Current points length   : " + std::to_string(points.size()));
        //     DLOG(auxillary, "Current map size        : " + std::to_string(point_map.size()));
        //     DDEDENT;
        // }
    }

    // Done
    DRETURN;
}

/* Appends a given list of indexed faces to the global list of indexed Faces. */
void Renderer::append_faces(Tools::Array<GFace>& faces, Tools::Array<glm::vec4>& vertices, const Tools::Array<GFace>& new_faces, const Tools::Array<glm::vec4>& new_vertices) {
    DENTER("Renderer::append_faces");

    // We do not check for uniqueness, but simply append, since objects are probably not likely to share vertices
    uint32_t offset = static_cast<uint32_t>(faces.size());
    for (size_t i = 0; i < new_faces.size(); i++) {
        // Add it to the new list, except that we add the offset to each index
        faces.push_back({
            new_faces[i].v1 + offset, new_faces[i].v2 + offset, new_faces[i].v3 + offset,
            new_faces[i].normal,
            new_faces[i].color
        });
    }

    // Also append the points
    for (size_t i = 0; i < new_vertices.size(); i++) {
        vertices.push_back(new_vertices[i]);
    }

    DRETURN;
}



/* Swap operator for the Renderer baseclass. */
void RayTracer::swap(Renderer& r1, Renderer& r2) {
    using std::swap;

    swap(r1.entity_faces, r2.entity_faces);
    swap(r1.entity_vertices, r2.entity_vertices);
}
