/* VERTEX.hpp
 *   by Lut99
 *
 * Created:
 *   02/05/2021, 15:45:39
 * Last edited:
 *   06/05/2021, 17:01:17
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
    /* The Face struct, which represents a single face, optimised for CPU usage. */
    struct Face {
        /* The first vertex of the face. */
        alignas(16) glm::vec3 v1;
        /* The second vertex of the face. */
        alignas(16) glm::vec3 v2;
        /* The third vertex of the face. */
        alignas(16) glm::vec3 v3;

        /* The normal of the face. */
        alignas(16) glm::vec3 normal;
        /* The color of the face. */
        alignas(16) glm::vec3 color;
    };

    /* The GPU-compatible & vertex-indexed Face counterpart, the GFace struct. */
    struct GFace {
        /* The first vertex of the face. */
        alignas(4) uint32_t v1;
        /* The second vertex of the face. */
        alignas(4) uint32_t v2;
        /* The third vertex of the face. */
        alignas(4) uint32_t v3;

        /* The normal of the face. */
        alignas(16) glm::vec3 normal;
        /* The color of the face. */
        alignas(16) glm::vec3 color;
    };

}

#endif
