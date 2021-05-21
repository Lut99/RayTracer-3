/* DESCRIPTOR POOL.hpp
 *   by Lut99
 *
 * Created:
 *   26/04/2021, 14:39:16
 * Last edited:
 *   21/05/2021, 18:07:47
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
        void set(const GPU& gpu, VkDescriptorType descriptor_type, uint32_t bind_index, const Tools::Array<Buffer>& buffers) const;
        /* Binds this descriptor set with the contents of a given image view to the given bind index. Must be enough views to actually populate all bindings of the given type. */
        void set(const GPU& gpu, VkDescriptorType descriptor_type, uint32_t bind_index, const Tools::Array<VkImageView>& image_views) const;
        /* Binds the descriptor to the given (compute) command buffer. We assume that the recording already started. */
        void bind(const CommandBuffer& buffer, VkPipelineLayout pipeline_layout) const;

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
        /* Type of descriptors that can be allocated here and how many per type. */
        Tools::Array<std::tuple<VkDescriptorType, uint32_t>> vk_descriptor_types;
        /* The maximum number of sets allowed in this pool. */
        uint32_t vk_max_sets;
        /* The create flags used to initialize this pool. */
        VkDescriptorPoolCreateFlags vk_create_flags;

        /* Internal list of Descriptors. */
        Tools::Array<VkDescriptorSet> vk_descriptor_sets;

    public:
        /* Constructor for the DescriptorPool class, which takes the GPU to create the pool on, the type of descriptors, the number of descriptors we want to allocate in the pool, the maximum number of descriptor sets that can be allocated and optionally custom create flags. */
        DescriptorPool(const GPU& gpu, VkDescriptorType descriptor_type, uint32_t max_descriptors, uint32_t max_sets, VkDescriptorPoolCreateFlags flags = 0);
        /* Constructor for the DescriptorPool class, which takes the GPU to create the pool on, a list of descriptor types and their counts, the maximum number of descriptor sets that can be allocated and optionally custom create flags. */
        DescriptorPool(const GPU& gpu, const Tools::Array<std::tuple<VkDescriptorType, uint32_t>>& descriptor_types, uint32_t max_sets, VkDescriptorPoolCreateFlags flags = 0);
        /* Copy constructor for the DescriptorPool. */
        DescriptorPool(const DescriptorPool& other);
        /* Move constructor for the DescriptorPool. */
        DescriptorPool(DescriptorPool&& other);
        /* Destructor for the DescriptorPool. */
        ~DescriptorPool();

        /* Allocates a single descriptor set with the given layout. Will fail with errors if there's no more space. */
        DescriptorSet allocate(const DescriptorSetLayout& descriptor_set_layout);
        /* Allocates multiple descriptor sets with the given layout, returning them as an Array. Will fail with errors if there's no more space. */
        Tools::Array<DescriptorSet> nallocate(uint32_t n_sets, const Tools::Array<DescriptorSetLayout>& descriptor_set_layouts);
        /* Deallocates the given descriptor set. */
        void deallocate(const DescriptorSet& descriptor_set);
        /* Deallocates an array of given descriptors set. */
        void ndeallocate(const Tools::Array<DescriptorSet>& descriptor_sets);

        /* Returns the current number of sets allocated in this pool. */
        inline size_t size() const { return this->vk_descriptor_sets.size(); }
        /* Returns the maximum number of sets we can allocate in this pool. */
        inline size_t capacity() const { return static_cast<size_t>(this->vk_max_sets); }

        /* Explicitly returns the internal VkDescriptorPool object. */
        inline VkDescriptorPool descriptor_pool() const { return this->vk_descriptor_pool; }
        /* Implicitly returns the internal VkDescriptorPool object. */
        inline operator VkDescriptorPool() const { return this->vk_descriptor_pool; }

        /* Copy assignment operator for the DescriptorPool class. */
        inline DescriptorPool& operator=(const DescriptorPool& other) { return *this = DescriptorPool(other); }
        /* Move assignment operator for the DescriptorPool class. */
        inline DescriptorPool& operator=(DescriptorPool&& other) { if (this != &other) { swap(*this, other); } return *this; }
        /* Swap operator for the DescriptorPool class. */
        friend void swap(DescriptorPool& dp1, DescriptorPool& dp2);

    };

    /* Swap operator for the DescriptorPool class. */
    void swap(DescriptorPool& dp1, DescriptorPool& dp2);
}

#endif
