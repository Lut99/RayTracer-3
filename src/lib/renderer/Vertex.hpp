/* VERTEX.hpp
 *   by Lut99
 *
 * Created:
 *   02/05/2021, 15:45:39
 * Last edited:
 *   03/05/2021, 15:48:00
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Defines a single (indexed) vertex, aligned for GPU usage.
**/

#ifndef RENDERER_VERTEX_HPP
#define RENDERER_VERTEX_HPP

#include <cstdint>

#include "glm/glm.hpp"

namespace RayTracer {
    /* The Vertex struct, which represents a single vertex, optimised for CPU usage. */
    struct Vertex {
        /* The points of the vertex. */
        glm::vec3 points[3];

        /* The normal of the vertex. */
        glm::vec3 normal;
        /* The color of the vertex. */
        glm::vec3 color;
    };

    /* The CPU-compatible & point-indexed Vertex counterpart, the CVertex struct. */
    struct CVertex {
        /* The first point of the vertex. */
        uint32_t p1;
        /* The second point of the vertex. */
        uint32_t p2;
        /* The third point of the vertex. */
        uint32_t p3;

        /* The normal of the vertex. */
        glm::vec3 normal;
        /* The color of the vertex. */
        glm::vec3 color;
    };

    /* The GPU-compatible & point-indexed Vertex counterpart, the GVertex struct. */
    struct GVertex {
        /* The first point of the vertex. */
        alignas(4) uint32_t p1;
        /* The second point of the vertex. */
        alignas(4) uint32_t p2;
        /* The third point of the vertex. */
        alignas(4) uint32_t p3;

        /* The normal of the vertex. */
        alignas(16) glm::vec4 normal;
        /* The color of the vertex. */
        alignas(16) glm::vec4 color;
    };

}

#endif
