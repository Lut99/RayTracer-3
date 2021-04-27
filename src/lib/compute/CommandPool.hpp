/* COMMAND POOL.hpp
 *   by Lut99
 *
 * Created:
 *   27/04/2021, 13:03:55
 * Last edited:
 *   27/04/2021, 14:03:11
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Contains the CommandPool class, which handles allocating and
 *   destroying command buffers for a single device queue.
**/

#ifndef COMPUTE_COMMAND_POOL_HPP
#define COMPUTE_COMMAND_POOL_HPP

#include <vulkan/vulkan.h>
#include <unordered_map>

#include "tools/Array.hpp"

#include "GPU.hpp"

namespace RayTracer::Compute {
    /* Handle for CommandBuffers, which can be used to retrieve them from the Pool. Note that each pool has its own set of handles. */
    using CommandBufferHandle = uint32_t;

    /* The CommandBuffer class, which acts as a reference to an allocated CommandBuffer in the CommandPool. Can thus be comfortably deallocated and then later re-acquired by its matching handle. */
    class CommandBuffer {
    private:
        /* The VkCommandBuffer object that we wrap. */
        VkCommandBuffer vk_command_buffer;

        /* Private constructor for the CommandBuffer, which only takes the VkCommandBuffer object to wrap. */
        CommandBuffer(VkCommandBuffer vk_command_buffer);

        /* Mark the CommandPool class as friend. */
        friend class CommandPool;

    public:
        /* Begins recording the command buffer. Overwrites whatever is already recorded here, for some reason. Takes optional usage flags for this recording. */
        void begin(VkCommandBufferUsageFlags usage_flags = 0);
        /* Ends recording the command buffer, but does not yet submit to any queue unless one is given. If so, then you can optionally specify to wait or not to wait for the queue to become idle. */
        void end(VkQueue vk_queue = nullptr, bool wait_queue_idle = true);

        /* Explititly returns the internal VkCommandBuffer object. */
        inline VkCommandBuffer command_buffer() const { return this->vk_command_buffer; }
        /* Implicitly returns the internal VkCommandBuffer object. */
        inline operator VkCommandBuffer() const { return this->vk_command_buffer; }

    };



    /* The CommandPool class, which manages CommandBuffers for a single device queue. */
    class CommandPool {
    private:
        /* Constant reference to the device that we're managing the pool for. */
        const GPU& gpu;

        /* The command pool object that we wrap. */
        VkCommandPool vk_command_pool;
        /* The device queue index of this pool. */
        uint32_t vk_queue_index;

        /* Map of the CommandBuffers allocated with this pool. */
        std::unordered_map<CommandBufferHandle, VkCommandBuffer> vk_command_buffers;

    public:
        /* Constructor for the CommandPool class, which takes the GPU to allocate for, the queue index for which this pool allocates buffers and optionally create flags for the pool. */
        CommandPool(const GPU& gpu, uint32_t queue_index, VkCommandPoolCreateFlags create_flags = 0);
        /* Copy constructor for the CommandPool class, which is deleted. */
        CommandPool(const CommandPool& other) = delete;
        /* Move constructor for the CommandPool class. */
        CommandPool(CommandPool&& other);
        /* Destructor for the CommandPool class. */
        ~CommandPool();

        /* Returns a CommandBuffer from the given handle, which can be used as a CommandBuffer. Does not perform any checks on the handle validity. */
        inline CommandBuffer operator[](CommandBufferHandle buffer) { return CommandBuffer(this->vk_command_buffers.at(buffer)); }
        /* Returns a CommandBuffer from the given handle, which can be used as a CommandBuffer. Does perform checks on the handle validity. */
        CommandBuffer at(CommandBufferHandle buffer);

        /* Allocates a single, new command buffer of the given level. Returns by handle. */
        CommandBufferHandle allocate(VkCommandBufferLevel buffer_level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
        /* Allocates N new command buffers of the given level. Returns by handles. */
        Tools::Array<CommandBufferHandle> nallocate(uint32_t n_buffers, VkCommandBufferLevel buffer_level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
        /* Deallocates the CommandBuffer behind the given handle. Note that all buffers are deallocated automatically when the CommandPool is destructed, but this could save you memory. */
        void deallocate(CommandBufferHandle buffer);

        /* Expliticly returns the internal VkCommandPool object. */
        inline VkCommandPool command_pool() const { return this->vk_command_pool; }
        /* Implicitly returns the internal VkCommandPool object. */
        inline operator VkCommandPool() const { return this->vk_command_pool; }

        /* Copy assignment operator for the CommandPool class, which is deleted. */
        CommandPool& operator=(const CommandPool& other) = delete;
        /* Move assignment operator for the CommandPool class. */
        CommandPool& operator=(CommandPool&& other);
        /* Swap operator for the CommandPool class. */
        friend void swap(CommandPool& cp1, CommandPool& cp2);

    };

    /* Swap operator for the CommandPool class. */
    void swap(CommandPool& cp1, CommandPool& cp2);

}

#endif
