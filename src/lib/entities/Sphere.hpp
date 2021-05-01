/* SPHERE.hpp
 *   by Lut99
 *
 * Created:
 *   01/05/2021, 11:59:00
 * Last edited:
 *   01/05/2021, 13:31:14
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Contains code for pre-rendering a Sphere on the CPU (single threaded).
**/

#ifndef ENTITIES_SPHERE_HPP
#define ENTITIES_SPHERE_HPP

#include "glm/glm.hpp"

#include "RenderEntity.hpp"

#include "tools/Array.hpp"

namespace RayTracer::ECS {
    /* The Sphere struct, which builds on the RenderEntity struct in an entity-component-system way. */
    struct Sphere: public RenderEntity {
        /* The center of the sphere. */
        glm::vec3 center;
        /* The radius of the sphere. */
        float radius;
        /* The number of meridians in the sphere (i.e., vertical lines). */
        uint32_t n_meridians;
        /* The number of parallels in the sphere (i.e., horizontal lines). */
        uint32_t n_parallels;
    };



    /* Creates a new Sphere struct based on the given properties. */
    Sphere* create_sphere(const glm::vec3& center, float radius, uint32_t n_meridians, uint32_t n_parallels);

    /* Pre-renders the sphere on the CPU, single-threaded. */
    void cpu_pre_render_sphere(Tools::Array<uint32_t>& indices, Tools::Array<glm::vec4>& vertices, Tools::Array<glm::vec4>& normals, Tools::Array<glm::vec4>& colors, const Sphere& sphere);

}

#endif
