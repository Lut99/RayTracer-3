/* VULKAN RENDERER.hpp
 *   by Lut99
 *
 * Created:
 *   30/04/2021, 13:34:28
 * Last edited:
 *   30/04/2021, 15:16:12
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Implementation of the Renderer class that uses Vulkan compute shaders.
**/

#ifndef RENDERER_VULKAN_RENDERER_HPP
#define RENDERER_VULKAN_RENDERER_HPP

#include "glm/glm.hpp"

#include "compute/Instance.hpp"
#include "compute/GPU.hpp"
#include "compute/MemoryPool.hpp"
#include "compute/DescriptorPool.hpp"
#include "compute/CommandPool.hpp"

#include "Renderer.hpp"

namespace RayTracer {
    namespace Compute {
        /* Global Vulkan instance. */
        const Compute::Instance vk_instance;
    }



    /* The VulkanRenderer class, which implements the standard Renderer using Vulkan compute shaders. */
    class VulkanRenderer: public Renderer {
    public:
        /* Constant that determines the pool size of the device-local memory. */
        static const constexpr VkDeviceSize device_memory_size = 1024 * 1024 * 1024;
        /* Constant that determines the pool size of the transfer memory. */
        static const constexpr VkDeviceSize stage_memory_size = 1024 * 1024 * 1024;
        /* The maximum number of descriptors per set in the desriptor pool. */
        static const constexpr uint32_t max_descriptors = 2;
        /* The maximum number of descriptor sets in the desriptor pool. */
        static const constexpr uint32_t max_descriptor_sets = 2;

    private:
        /* The GPU on which we work. */
        Compute::GPU* gpu;

        /* The memory pool for GPU-active memory. */
        Compute::MemoryPool* device_memory_pool;
        /* The memory pool used for staging buffers. */
        Compute::MemoryPool* stage_memory_pool;

        /* The pool we use to allocate descriptor sets from. */
        Compute::DescriptorPool* descriptor_pool;

        /* Command pool used to schedule compute command buffers from. */
        Compute::CommandPool* compute_command_pool;
        /* Command pool used to schedule quick memory jobs on. */
        Compute::CommandPool* memory_command_pool;

        /* The pre-rendered list of vertices, which we can send to the GPU. */
        Tools::Array<glm::vec4> entity_vertices;
        /* The pre-rendered list of vertex normals, which we can send to the GPU. */
        Tools::Array<glm::vec4> entity_normals;
        /* The pre-rendered list of vertex colors, which we can send to the GPU. */
        Tools::Array<glm::vec4> entity_colors;
        /* The number of vertices that we pre-rendered. */
        uint32_t n_entity_vertices;

    public:
        /* Constructor for the VulkanRenderer class. */
        VulkanRenderer();
        /* Copy constructor for the VulkanRenderer class. */
        VulkanRenderer(const VulkanRenderer& other);
        /* Move constructor for the VulkanRenderer class. */
        VulkanRenderer(VulkanRenderer&& other);
        /* Destructor for the VulkanRenderer class. */
        virtual ~VulkanRenderer();
        
        /* Pre-renders the given list of RenderEntities, accelerated using Vulkan compute shaders. */
        virtual void prerender(const Tools::Array<ECS::RenderEntity>& entities);
        /* Renders the internal list of vertices to a frame using the given camera position. */
        virtual void render(Camera& camera) const;

        /* Copy assignment operator for the VulkanRenderer class. */
        virtual VulkanRenderer& operator=(const VulkanRenderer& other) { return *this = VulkanRenderer(other); }
        /* Move assignment operator for the VulkanRenderer class. */
        virtual VulkanRenderer& operator=(VulkanRenderer&& other) { if (this != &other) { swap(*this, other); } return *this; }
        /* Swap operator for the VulkanRenderer class. */
        friend void swap(VulkanRenderer& r1, VulkanRenderer& r2);

    };

    /* Swap operator for the VulkanRenderer class. */
    void swap(VulkanRenderer& r1, VulkanRenderer& r2);
}

#endif