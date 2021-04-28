/* DESCRIPTOR POOL.hpp
 *   by Lut99
 *
 * Created:
 *   26/04/2021, 14:39:16
 * Last edited:
 *   28/04/2021, 15:07:07
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Contains the DescriptorPool class, which serves as a memory pool for
 *   descriptors, which in turn describe how a certain buffer or other
 *   piece of memory should be accessed on the GPU.
**/

#ifndef COMPUTE_DESCRIPTOR_POOL_HPP
#define COMPUTE_DESCRIPTOR_POOL_HPP

#include <vulkan/vulkan.h>

#include "MemoryPool.hpp"
#include "DescriptorSetLayout.hpp"
#include "CommandPool.hpp"
#include "tools/Array.hpp"

#include "GPU.hpp"

namespace RayTracer::Compute {
    /* The DescriptorSet class, which represents a reference to a single VkDescriptorSet. */
    class DescriptorSet {
    private:
        /* The VkDescriptorSet object that this class wraps. */
        VkDescriptorSet vk_descriptor_set;

        /* Constructor for the DescriptorSet class, which takes the vk_descriptor_set it wraps. */
        DescriptorSet(VkDescriptorSet vk_descriptor_set);

        /* Mark the DescriptorPool as friend. */
        friend class DescriptorPool;

    public:
        /* Binds this descriptor set with the contents of a given buffer to the given bind index. */
        void set(const GPU& gpu, uint32_t bind_index, const Tools::Array<Buffer>& buffers);
        /* Binds the descriptor to the given (compute) command buffer. We assume that the recording already started. */
        void bind(const CommandBuffer& buffer, VkPipelineLayout pipeline_layout);

        /* Explicity returns the internal VkDescriptorSet object. */
        inline VkDescriptorSet descriptor_set() const { return this->vk_descriptor_set; }
        /* Implicitly returns the internal VkDescriptorSet object. */
        inline operator VkDescriptorSet() const { return this->vk_descriptor_set; }

    };



    /* The DescriptorPool class, which is used to generate and manage descriptor(sets) for describing buffers. */
    class DescriptorPool {
    public:
        /* Constant reference to the GPU we allocate this pool on. */
        const GPU& gpu;

    private:
        /* The internal pool used for allocating new pools. */
        VkDescriptorPool vk_descriptor_pool;
        /* The maximum number of sets allowed in this pool. */
        uint32_t max_size;

        /* Internal list of Descriptors. */
        Tools::Array<VkDescriptorSet> vk_descriptor_sets;

    public:
        /* Constructor for the DescriptorPool class, which takes the GPU to create the pool on, the number of descriptors we want to allocate in the pool, the maximum number of descriptor sets that can be allocated and optionally custom create flags. */
        DescriptorPool(const GPU& gpu, VkDescriptorType descriptor_type, uint32_t n_descriptors, uint32_t max_sets, VkDescriptorPoolCreateFlags flags = 0);
        /* Copy constructor for the DescriptorPool, which is deleted. */
        DescriptorPool(const DescriptorPool& other);
        /* Move constructor for the DescriptorPool. */
        DescriptorPool(DescriptorPool&& other);
        /* Destructor for the DescriptorPool. */
        ~DescriptorPool();

        /* Allocates a single descriptor set with the given layout. Will fail with errors if there's no more space. */
        DescriptorSet allocate(const DescriptorSetLayout& descriptor_set_layout);
        /* Allocates multiple descriptor sets with the given layout, returning them as an Array. Will fail with errors if there's no more space. */
        Tools::Array<DescriptorSet> nallocate(uint32_t n_sets, const Tools::Array<DescriptorSetLayout>& descriptor_set_layouts);

        /* Returns the current number of sets allocated in this pool. */
        inline size_t size() const { return this->vk_descriptor_sets.size(); }
        /* Returns the maximum number of sets we can allocate in this pool. */
        inline size_t capacity() const { return static_cast<size_t>(this->max_size); }

        /* Explicitly returns the internal VkDescriptorPool object. */
        inline VkDescriptorPool descriptor_pool() const { return this->vk_descriptor_pool; }
        /* Implicitly returns the internal VkDescriptorPool object. */
        inline operator VkDescriptorPool() const { return this->vk_descriptor_pool; }

        /* Copy assignment operator for the DescriptorPool class, which is deleted. */
        DescriptorPool& operator=(const DescriptorPool& other) = delete;
        /* Move assignment operator for the DescriptorPool class. */
        DescriptorPool& operator=(DescriptorPool&& other);
        /* Swap operator for the DescriptorPool class. */
        friend void swap(DescriptorPool& dp1, DescriptorPool& dp2);

    };

    /* Swap operator for the DescriptorPool class. */
    void swap(DescriptorPool& dp1, DescriptorPool& dp2);
}

#endif
