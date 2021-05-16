/* TRIANGLE.hpp
 *   by Lut99
 *
 * Created:
 *   01/05/2021, 13:35:06
 * Last edited:
 *   16/05/2021, 12:45:00
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Contains code to handle a simple triangle.
**/

#ifndef ENTITIES_TRIANGLE_HPP
#define ENTITIES_TRIANGLE_HPP

#include "glm/glm.hpp"

#include "RenderEntity.hpp"

#include "renderer/Vertex.hpp"

#include "tools/Array.hpp"

namespace RayTracer::ECS {
    /* The Triangle struct, which builds on the RenderEntity struct in an entity-component-system way. */
    struct Triangle: public RenderEntity {
        /* The three points of the triangle. */
        glm::vec3 points[3];
        /* The normal of the triangle. */
        glm::vec3 normal;
        /* The color of the triangle. */
        glm::vec3 color;
    };



    /* Creates a new Triangle struct based on the given properties. */
    Triangle* create_triangle(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec3& color);

    /* Pre-renders the sphere on the CPU, single-threaded. */
    void cpu_pre_render_triangle(Tools::Array<Face>& faces, Triangle* triangle);

    /* Returns the number of faces & vertices for this triangle, appended to the given integers. */
    void get_size_triangle(uint32_t& n_faces, uint32_t& n_vertices, Triangle* triangle);

}

#endif
