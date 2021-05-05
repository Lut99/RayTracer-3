/* SPHERE.hpp
 *   by Lut99
 *
 * Created:
 *   01/05/2021, 11:59:00
 * Last edited:
 *   05/05/2021, 17:29:59
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

#include "compute/GPU.hpp"
#include "compute/MemoryPool.hpp"
#include "compute/DescriptorPool.hpp"
#include "compute/CommandPool.hpp"
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

    /* Pre-renders the sphere on the CPU, single-threaded. */
    void cpu_pre_render_sphere(Tools::Array<Vertex>& vertices, Sphere* sphere);

    #ifdef ENABLE_VULKAN
    /* Pre-renders the sphere on the GPU using Vulkan compute shaders. */
    void gpu_pre_render_sphere(Tools::Array<Vertex>& vertices, const Compute::GPU& gpu, Compute::MemoryPool& device_memory_pool, Compute::MemoryPool& stage_memory_pool, Compute::DescriptorPool& descriptor_pool, Compute::CommandPool& compute_command_pool, const Compute::CommandBuffer& staging_cb, Sphere* sphere);
    #endif

}

#endif
