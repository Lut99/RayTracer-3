/* VULKAN ONLINE RENDERER.hpp
 *   by Lut99
 *
 * Created:
 *   09/05/2021, 18:30:37
 * Last edited:
 *   10/05/2021, 16:43:37
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Derived class of the VulkanRenderer class, which renders to a
 *   swapchain in real-time instead of to images. Therefore, the call to
 *   this render is blocking, returning an empty frame once the user closes
 *   the window.
**/

#ifndef RENDERER_VULKAN_ONLINE_RENDERER_HPP
#define RENDERER_VULKAN_ONLINE_RENDERER_HPP

#include "glm/glm.hpp"

#include "compute/Instance.hpp"
#include "compute/GPU.hpp"
#include "compute/MemoryPool.hpp"
#include "compute/DescriptorPool.hpp"
#include "compute/CommandPool.hpp"

#include "tools/Array.hpp"

#include "Renderer.hpp"

namespace RayTracer {
    /* The VulkanOnlineRenderer class, which renders to a window in real-time instead of a picture. */
    class VulkanOnlineRenderer: public Renderer {
    public:
         /* Constant that determines the pool size of the device-local memory. */
        static const constexpr VkDeviceSize device_memory_size = 1024 * 1024 * 1024;
        /* Constant that determines the pool size of the transfer memory. */
        static const constexpr VkDeviceSize stage_memory_size = 1024 * 1024 * 1024;
        /* The maximum number of descriptor sets in the desriptor pool. */
        static const constexpr uint32_t max_descriptor_sets = 1;

    protected:
        /* The instance used to select the GPU from. */
        Compute::Instance* instance;
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

    public:
        /* Constructor for the VulkanOnlineRenderer class, which takes nothing to be compatible. */
        VulkanOnlineRenderer();
        /* Copy constructor for the VulkanOnlineRenderer class. */
        VulkanOnlineRenderer(const VulkanOnlineRenderer& other);
        /* Move constructor for the VulkanOnlineRenderer class. */
        VulkanOnlineRenderer(VulkanOnlineRenderer&& other);
        /* Destructor for the VulkanOnlineRenderer class. */
        virtual ~VulkanOnlineRenderer();

        /* Pre-renders the given list of RenderEntities, accelerated using Vulkan compute shaders. */
        virtual void prerender(const Tools::Array<ECS::RenderEntity*>& entities);
        /* Renders the internal list of vertices to a window using the given camera position. Renders the entire simulation, including update steps, and returns a blackened frame since the output is unreachable and simultaneously not interesting. */
        virtual void render(Camera& camera) const;

        /* Copy assignment operator for the VulkanOnlineRenderer class. */
        virtual VulkanOnlineRenderer& operator=(const VulkanOnlineRenderer& other) { return *this = VulkanOnlineRenderer(other); }
        /* Move assignment operator for the VulkanOnlineRenderer class. */
        virtual VulkanOnlineRenderer& operator=(VulkanOnlineRenderer&& other) { if (this != &other) { swap(*this, other); } return *this; }
        /* Swap operator for the VulkanOnlineRenderer class. */
        friend void swap(VulkanOnlineRenderer& r1, VulkanOnlineRenderer& r2);

    };

    /* Swap operator for the VulkanOnlineRenderer class. */
    void swap(VulkanOnlineRenderer& r1, VulkanOnlineRenderer& r2);
}

#endif
