/* OBJECT.hpp
 *   by Lut99
 *
 * Created:
 *   06/05/2021, 16:52:02
 * Last edited:
 *   06/05/2021, 18:09:06
 * Auto updated?
 *   Yes
 *
 * Description:
 *   File that contains an entity that loads object files. Does not yet
 *   load textures, just the normals and the geometries.
**/

#ifndef ENTITIES_OBJECT_HPP
#define ENTITIES_OBJECT_HPP

#include <string>

#include "glm/glm.hpp"
#include "renderer/Vertex.hpp"
#include "tools/Array.hpp"

#include "RenderEntity.hpp"

namespace RayTracer::ECS {
    /* The Sphere struct, which builds on the RenderEntity struct in an entity-component-system way. */
    struct Object: public RenderEntity {
        /* Path to the object file. */
        std::string file_path;
        /* The position of the object. */
        glm::vec3 center;
        /* The scale of the object. Use 1.0 to use the object's default size. */
        float scale;
        /* The color of the object. */
        glm::vec3 color;
    };



    /* Creates a new Object struct based on the given properties. Note that the actual loading of the object is done during pre-rendering. */
    Object* create_object(const std::string& file_path, const glm::vec3& center, float scale, const glm::vec3& color);

    /* Pre-renders the sphere on the CPU, single-threaded. Basically just loads the file given on creation. Since object files are usually indexed, so are we. */
    void cpu_pre_render_object(Tools::Array<GFace>& faces, Tools::Array<glm::vec4>& vertices, Object* obj);

}

#endif
