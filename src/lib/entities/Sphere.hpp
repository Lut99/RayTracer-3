/* SPHERE.hpp
 *   by Lut99
 *
 * Created:
 *   01/05/2021, 11:59:00
 * Last edited:
 *   19/05/2021, 17:05:34
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Contains code for pre-rendering a Sphere on the CPU (single threaded).
**/

#ifndef ENTITIES_SPHERE_HPP
#define ENTITIES_SPHERE_HPP

#include "glm/glm.hpp"

#include "renderer/Vertex.hpp"

#include "RenderEntity.hpp"

#include "tools/Array.hpp"

#ifdef ENABLE_VULKAN
#include <vulkan/vulkan.h>

#include "compute/Suite.hpp"
#endif

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
        /* The color of the sphere. */
        glm::vec3 color;
    };



    /* Creates a new Sphere struct based on the given properties. */
    Sphere* create_sphere(const glm::vec3& center, float radius, uint32_t n_meridians, uint32_t n_parallels, const glm::vec3& color);

    /* Pre-renders the sphere on the CPU, single-threaded, and returns a list of CPU-side buffers with the GFaces and the vertices. Assumes the given buffers contains irrelevant data, and already have the correct size. */
    void cpu_pre_render_sphere(Tools::Array<GFace>& faces_buffer, Tools::Array<glm::vec4>& vertex_buffer, Sphere* sphere);
    #ifdef ENABLE_VULKAN
    /* Pre-renders the sphere on the GPU using Vulkan compute shaders. Uses the given GPU-allocated buffers as target buffers, so we won't have to get the stuff back to the CPU. The given offsets specify from where the pre-rendering is safe to put its results, up until that offset plus the specified size in the Sphere. */
    void gpu_pre_render_sphere(const Compute::Buffer& faces_buffer, uint32_t faces_offset, const Compute::Buffer& vertex_buffer, uint32_t vertex_offset, Compute::Suite& gpu, Sphere* sphere);
    #endif

}

#endif
