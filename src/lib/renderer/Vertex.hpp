/* VERTEX.hpp
 *   by Lut99
 *
 * Created:
 *   02/05/2021, 15:45:39
 * Last edited:
 *   05/05/2021, 15:24:35
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
        /* The first point of the vertex. */
        alignas(16) glm::vec3 p1;
        /* The second point of the vertex. */
        alignas(16) glm::vec3 p2;
        /* The third point of the vertex. */
        alignas(16) glm::vec3 p3;

        /* The normal of the vertex. */
        alignas(16) glm::vec3 normal;
        /* The color of the vertex. */
        alignas(16) glm::vec3 color;
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
        alignas(16) glm::vec3 normal;
        /* The color of the vertex. */
        alignas(16) glm::vec3 color;
    };

}

#endif
