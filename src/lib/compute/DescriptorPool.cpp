/* DESCRIPTOR POOL.cpp
 *   by Lut99
 *
 * Created:
 *   26/04/2021, 14:38:48
 * Last edited:
 *   25/05/2021, 17:23:26
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Contains the DescriptorPool class, which serves as a memory pool for
 *   descriptors, which in turn describe how a certain buffer or other
 *   piece of memory should be accessed on the GPU.
**/

#include "debugger/CppDebugger.hpp"

#include "ErrorCodes.hpp"

#include "DescriptorPool.hpp"

using namespace std;
using namespace RayTracer;
using namespace RayTracer::Compute;
using namespace CppDebugger::SeverityValues;


/***** POPULATE FUNCTIONS *****/
/* Populates a given VkDescriptorBufferInfo struct. */
static void populate_buffer_info(VkDescriptorBufferInfo& buffer_info, const Buffer& buffer) {
    DENTER("populate_buffer_info");

    // Set to default
    buffer_info = {};
    
    // Set the memory properties
    buffer_info.buffer = buffer.buffer();
    buffer_info.offset = 0; // Note that this offset is (probably) relative to the buffer itself, not the vk_memory object it was allocated with
    buffer_info.range = buffer.size();

    // Done
    DRETURN;
}

/* Populates a given VkDescriptorImageInfo struct. */
static void populate_image_info(VkDescriptorImageInfo& image_info, const VkImageView& image_view) {
    DENTER("populate_image_info");

    // Set to default
    image_info = {};
    
    // Set the image view associated with this info and the rest to nullptrs
    image_info.imageView = image_view;
    image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_info.sampler = nullptr;

    // DOne
    DRETURN;
}

/* Populates a given VkWriteDescriptorSet struct to write a buffer. */
static void populate_write_info(VkWriteDescriptorSet& write_info, VkDescriptorSet vk_descriptor_set, VkDescriptorType vk_descriptor_type, uint32_t bind_index, const Tools::Array<VkDescriptorBufferInfo>& buffer_infos) {
    DENTER("populate_write_info(buffers)");

    // Set to default
    write_info = {};
    write_info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;

    // Set the set to which we write
    write_info.dstSet = vk_descriptor_set;

    // Set the binding to use; this one's equal to the sub-binding in the shader
    write_info.dstBinding = bind_index;
    
    // Set the element of the array. For now, we fix this at 0
    write_info.dstArrayElement = 0;

    // Next, set which type of descriptor this is and how many
    write_info.descriptorCount = static_cast<uint32_t>(buffer_infos.size());
    write_info.descriptorType = vk_descriptor_type;

    // Then, the data to pass. This can be of multiple forms, but we pass a buffer.
    write_info.pBufferInfo = buffer_infos.rdata();
    write_info.pImageInfo = nullptr;
    write_info.pTexelBufferView = nullptr;

    // Done
    DRETURN;
}
/* Populates a given VkWriteDescriptorSet struct to write an image. */
static void populate_write_info(VkWriteDescriptorSet& write_info, VkDescriptorSet vk_descriptor_set, VkDescriptorType vk_descriptor_type, uint32_t bind_index, const Tools::Array<VkDescriptorImageInfo>& image_infos) {
    DENTER("populate_write_info(images)");

    // Set to default
    write_info = {};
    write_info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;

    // Set the set to which we write
    write_info.dstSet = vk_descriptor_set;

    // Set the binding to use; this one's equal to the sub-binding in the shader
    write_info.dstBinding = bind_index;
    
    // Set the element of the array. For now, we fix this at 0
    write_info.dstArrayElement = 0;

    // Next, set which type of descriptor this is and how many
    write_info.descriptorCount = static_cast<uint32_t>(image_infos.size());
    write_info.descriptorType = vk_descriptor_type;

    // Then, the data to pass. This can be of multiple forms, but we pass a buffer.
    write_info.pBufferInfo = nullptr;
    write_info.pImageInfo = image_infos.rdata();
    write_info.pTexelBufferView = nullptr;

    // Done
    DRETURN;
}

/* Populates a given VkDescriptorPoolSize struct. */
static void populate_descriptor_pool_size(VkDescriptorPoolSize& descriptor_pool_size, const std::tuple<VkDescriptorType, uint32_t>& descriptor_type) {
    DENTER("populate_descriptor_pool_size");

    // Initialize to default
    descriptor_pool_size = {};
    descriptor_pool_size.type = std::get<0>(descriptor_type);
    descriptor_pool_size.descriptorCount = std::get<1>(descriptor_type);

    // Done;
    DRETURN;
}

/* Populates a given VkDescriptorPoolCreateInfo struct. */
static void populate_descriptor_pool_info(VkDescriptorPoolCreateInfo& descriptor_pool_info, const Tools::Array<VkDescriptorPoolSize>& descriptor_pool_sizes, uint32_t n_sets, VkDescriptorPoolCreateFlags descriptor_pool_flags) {
    DENTER("populate_descriptor_pool_info");

    // Initialize to default
    descriptor_pool_info = {};
    descriptor_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;

    // Set the pool size to use
    descriptor_pool_info.poolSizeCount = static_cast<uint32_t>(descriptor_pool_sizes.size());
    descriptor_pool_info.pPoolSizes = descriptor_pool_sizes.rdata();

    // Set the maximum number of sets allowed
    descriptor_pool_info.maxSets = n_sets;

    // Set the flags to use
    descriptor_pool_info.flags = descriptor_pool_flags | VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    // Done;
    DRETURN;
}

/* Populates a given VkDescriptorSetAllocateInfo struct. */
static void populate_descriptor_set_info(VkDescriptorSetAllocateInfo& descriptor_set_info, VkDescriptorPool vk_descriptor_pool, const Tools::Array<VkDescriptorSetLayout>& vk_descriptor_set_layouts, uint32_t n_sets) {
    DENTER("populate_descriptor_set_info");

    // Set to default
    descriptor_set_info = {};
    descriptor_set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;

    // Set the pool that we use to allocate them
    descriptor_set_info.descriptorPool = vk_descriptor_pool;

    // Set the number of sets to allocate
    descriptor_set_info.descriptorSetCount = n_sets;

    // Set the layout for this descriptor
    descriptor_set_info.pSetLayouts = vk_descriptor_set_layouts.rdata();

    // Done
    DRETURN;
}





/***** DESCRIPTORSET CLASS *****/
/* Constructor for the DescriptorSet class, which takes the vk_descriptor_set it wraps. */
DescriptorSet::DescriptorSet(VkDescriptorSet descriptor_set) :
    vk_descriptor_set(descriptor_set)
{}

/* Binds this descriptor set with the contents of a given buffer to the given bind index. Must be enough buffers to actually populate all bindings of the given type. */
void DescriptorSet::set(const GPU& gpu, VkDescriptorType descriptor_type, uint32_t bind_index, const Tools::Array<Buffer>& buffers) const {
    DENTER("Compute::DescriptorSet::set(Buffer)");

    // We first create a list of buffer infos
    Tools::Array<VkDescriptorBufferInfo> buffer_infos(buffers.size());
    for (size_t i = 0; i < buffers.size(); i++) {
        // Start by creating the buffer info so that the descriptor knows smthng about the buffer
        VkDescriptorBufferInfo buffer_info;
        populate_buffer_info(buffer_info, buffers[i]);

        // Add to the list
        buffer_infos.push_back(buffer_info);
    }

    // Next, generate a VkWriteDescriptorSet with which we populate the buffer information
    // Can also use copies, for descriptor-to-descriptor stuff.
    VkWriteDescriptorSet write_info;
    populate_write_info(write_info, this->vk_descriptor_set, descriptor_type, bind_index, buffer_infos);

    // With the write info populated, update this set. Note that this can be used to perform multiple descriptor write & copies simultaneously
    vkUpdateDescriptorSets(gpu, 1, &write_info, 0, nullptr);

    // Done!
    DRETURN;
}

/* Binds this descriptor set with the contents of a given image view to the given bind index. Must be enough views to actually populate all bindings of the given type. */
void DescriptorSet::set(const GPU& gpu, VkDescriptorType descriptor_type, uint32_t bind_index, const Tools::Array<VkImageView>& image_views) const {
    DENTER("Compute::DescriptorSet::set(VkImageView)");

    // We first create a list of buffer infos
    Tools::Array<VkDescriptorImageInfo> image_infos(image_views.size());
    for (size_t i = 0; i < image_views.size(); i++) {
        // Start by creating the buffer info so that the descriptor knows smthng about the buffer
        VkDescriptorImageInfo image_info;
        populate_image_info(image_info, image_views[i]);

        // Add to the list
        image_infos.push_back(image_info);
    }

    // Next, generate a VkWriteDescriptorSet with which we populate the buffer information
    // Can also use copies, for descriptor-to-descriptor stuff.
    VkWriteDescriptorSet write_info;
    populate_write_info(write_info, this->vk_descriptor_set, descriptor_type, bind_index, image_infos);

    // With the write info populated, update this set. Note that this can be used to perform multiple descriptor write & copies simultaneously
    vkUpdateDescriptorSets(gpu, 1, &write_info, 0, nullptr);

    // Done!
    DRETURN;
}

/* Binds the descriptor to the given (compute) command buffer. We assume that the recording already started. */
void DescriptorSet::bind(const CommandBuffer& buffer, VkPipelineLayout pipeline_layout) const {
    DENTER("Compute::DescriptorSet::bind");

    // Add the binding
    vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &this->vk_descriptor_set, 0, nullptr);

    DRETURN;
}





/***** DESCRIPTORPOOL CLASS *****/
/* Constructor for the DescriptorPool class, which takes the GPU to create the pool on, the number of descriptors we want to allocate in the pool, the maximum number of descriptor sets that can be allocated and optionally custom create flags. */
DescriptorPool::DescriptorPool(const GPU& gpu, VkDescriptorType descriptor_type, uint32_t max_descriptors, uint32_t max_sets, VkDescriptorPoolCreateFlags flags):
    gpu(gpu),
    vk_descriptor_types(Tools::Array<std::tuple<VkDescriptorType, uint32_t>>({ std::make_tuple(descriptor_type, max_descriptors) })),
    vk_max_sets(max_sets),
    vk_create_flags(flags),
    vk_descriptor_sets(max_sets)
{
    DENTER("Compute::DescriptorPool::DescriptorPool");
    DLOG(info, "Initializing DescriptorPool...");
    DINDENT;



    // First, we define how large the pool will be
    DLOG(info, "Preparing structs...");
    VkDescriptorPoolSize descriptor_pool_size;
    populate_descriptor_pool_size(descriptor_pool_size, this->vk_descriptor_types[0]);

    // Prepare the create info
    VkDescriptorPoolCreateInfo descriptor_pool_info;
    populate_descriptor_pool_info(descriptor_pool_info, Tools::Array<VkDescriptorPoolSize>({ descriptor_pool_size }), this->vk_max_sets, this->vk_create_flags);



    // Actually allocate the pool
    DLOG(info, "Allocating pool...");
    VkResult vk_result;
    if ((vk_result = vkCreateDescriptorPool(this->gpu, &descriptor_pool_info, nullptr, &this->vk_descriptor_pool)) != VK_SUCCESS) {
        DLOG(fatal, "Could not allocate descriptor pool: " + vk_error_map[vk_result]);
    }



    DDEDENT;
    DLEAVE;
}

/* Constructor for the DescriptorPool class, which takes the GPU to create the pool on, a list of descriptor types and their counts, the maximum number of descriptor sets that can be allocated and optionally custom create flags. */
DescriptorPool::DescriptorPool(const GPU& gpu, const Tools::Array<std::tuple<VkDescriptorType, uint32_t>>& descriptor_types, uint32_t max_sets, VkDescriptorPoolCreateFlags flags) :
    gpu(gpu),
    vk_descriptor_types(descriptor_types),
    vk_max_sets(max_sets),
    vk_create_flags(flags),
    vk_descriptor_sets(max_sets)
{
    DENTER("Compute::DescriptorPool::DescriptorPool(multiple types)");
    DLOG(info, "Initializing DescriptorPool for multiple types...");
    DINDENT;



    // First, we define how large the pool will be
    DLOG(info, "Preparing structs...");
    Tools::Array<VkDescriptorPoolSize> descriptor_pool_sizes;
    descriptor_pool_sizes.resize(this->vk_descriptor_types.size());
    for (size_t i = 0; i < this->vk_descriptor_types.size(); i++) {
        populate_descriptor_pool_size(descriptor_pool_sizes[i], this->vk_descriptor_types[i]);
    }

    // Prepare the create info
    VkDescriptorPoolCreateInfo descriptor_pool_info;
    populate_descriptor_pool_info(descriptor_pool_info, descriptor_pool_sizes, this->vk_max_sets, this->vk_create_flags);



    // Actually allocate the pool
    DLOG(info, "Allocating pool...");
    VkResult vk_result;
    if ((vk_result = vkCreateDescriptorPool(this->gpu, &descriptor_pool_info, nullptr, &this->vk_descriptor_pool)) != VK_SUCCESS) {
        DLOG(fatal, "Could not allocate descriptor pool: " + vk_error_map[vk_result]);
    }



    DDEDENT;
    DLEAVE;
}

/* Copy constructor for the DescriptorPool. */
DescriptorPool::DescriptorPool(const DescriptorPool& other) :
    gpu(other.gpu),
    vk_descriptor_types(other.vk_descriptor_types),
    vk_max_sets(other.vk_max_sets),
    vk_create_flags(other.vk_create_flags),
    vk_descriptor_sets(other.vk_max_sets)
{
    DENTER("Compute::DescriptorPool::DescriptorPool(copy)");

    // First, we define how large the pool will be
    Tools::Array<VkDescriptorPoolSize> descriptor_pool_sizes;
    descriptor_pool_sizes.resize(this->vk_descriptor_types.size());
    for (size_t i = 0; i < this->vk_descriptor_types.size(); i++) {
        populate_descriptor_pool_size(descriptor_pool_sizes[i], this->vk_descriptor_types[0]);
    }

    // Prepare the create info
    VkDescriptorPoolCreateInfo descriptor_pool_info;
    populate_descriptor_pool_info(descriptor_pool_info, descriptor_pool_sizes, this->vk_max_sets, this->vk_create_flags);

    // Actually allocate the pool
    VkResult vk_result;
    if ((vk_result = vkCreateDescriptorPool(this->gpu, &descriptor_pool_info, nullptr, &this->vk_descriptor_pool)) != VK_SUCCESS) {
        DLOG(fatal, "Could not allocate descriptor pool: " + vk_error_map[vk_result]);
    }

    // Do not copy individual descriptors

    DLEAVE;
}
        
/* Move constructor for the DescriptorPool. */
DescriptorPool::DescriptorPool(DescriptorPool&& other):
    gpu(other.gpu),
    vk_descriptor_pool(other.vk_descriptor_pool),
    vk_descriptor_types(other.vk_descriptor_types),
    vk_max_sets(other.vk_max_sets),
    vk_create_flags(other.vk_create_flags),
    vk_descriptor_sets(other.vk_descriptor_sets)
{
    // Set the other's pool & sets to nullptr to avoid deallocation
    other.vk_descriptor_pool = nullptr;
    other.vk_descriptor_sets.clear();
}

/* Destructor for the DescriptorPool. */
DescriptorPool::~DescriptorPool() {
    DENTER("Compute::DescriptorPool::~DescriptorPool");
    DLOG(info, "Cleaning DescriptorPool...");
    DINDENT;

    VkResult vk_result;
    if (this->vk_descriptor_sets.size() > 0) {
        DLOG(info, "Deallocating descriptor sets...");
        if ((vk_result = vkFreeDescriptorSets(this->gpu, this->vk_descriptor_pool, (uint32_t) this->vk_descriptor_sets.size(), this->vk_descriptor_sets.rdata())) != VK_SUCCESS) {
            DLOG(nonfatal, "Could not deallocate descriptor sets: " + vk_error_map[vk_result]);
        }
    }

    if (this->vk_descriptor_pool != nullptr) {
        DLOG(info, "Deallocating pool...");
        vkDestroyDescriptorPool(this->gpu, this->vk_descriptor_pool, nullptr);
    }

    DDEDENT;
    DLEAVE;
}



/* Allocates a single descriptor set with the given layout. Will fail with errors if there's no more space. */
DescriptorSet DescriptorPool::allocate(const DescriptorSetLayout& descriptor_set_layout) {
    DENTER("Compute::DescriptorPool::allocate");

    // Check if we have enough space left
    if (static_cast<uint32_t>(this->vk_descriptor_sets.size()) >= this->vk_max_sets) {
        DLOG(fatal, "Cannot allocate new DescriptorSet: already allocated maximum of " + std::to_string(this->vk_max_sets) + " sets.");
    }
    
    // Put the layout in a struct s.t. we can pass it and keep it in memory until after the call
    Tools::Array<VkDescriptorSetLayout> vk_descriptor_set_layouts({ descriptor_set_layout.descriptor_set_layout() });

    // Next, populate the create info
    VkDescriptorSetAllocateInfo descriptor_set_info;
    populate_descriptor_set_info(descriptor_set_info, this->vk_descriptor_pool, vk_descriptor_set_layouts, 1);

    // We can now call the allocate function
    VkResult vk_result;
    if ((vk_result = vkAllocateDescriptorSets(this->gpu, &descriptor_set_info, this->vk_descriptor_sets.wdata(this->vk_descriptor_sets.size() + 1) + this->vk_descriptor_sets.size() - 1)) != VK_SUCCESS) {
        DLOG(fatal, "Failed to allocate new DescriptorSet: " + vk_error_map[vk_result]);
    }

    // Return a reference to this descriptor set
    DRETURN DescriptorSet(this->vk_descriptor_sets[this->vk_descriptor_sets.size() - 1]);
}

/* Allocates multiple descriptor sets with the given layout, returning them as an Array. Will fail with errors if there's no more space. */
Tools::Array<DescriptorSet> DescriptorPool::nallocate(uint32_t n_sets, const Tools::Array<DescriptorSetLayout>& descriptor_set_layouts) {
    DENTER("Compute::DescriptorPool::nallocate");

    #ifndef NDEBUG
    // If n_sets if null, nothing to do
    if (n_sets == 0) {
        DLOG(warning, "Request to allocate 0 sets received; nothing to do.");
        DRETURN Tools::Array<DescriptorSet>();
    }

    // If we aren't passed enough layouts, then tell us
    if (n_sets != descriptor_set_layouts.size()) {
        DLOG(fatal, "Not enough descriptor set layouts passed: got " + std::to_string(descriptor_set_layouts.size()) + ", expected " + std::to_string(n_sets));
    }
    #endif

    // Check if we have enough space left
    if (static_cast<uint32_t>(this->vk_descriptor_sets.size()) + n_sets > this->vk_max_sets) {
        DLOG(fatal, "Cannot allocate " + std::to_string(n_sets) + " new DescriptorSets: only space for " + std::to_string(this->vk_max_sets - static_cast<uint32_t>(this->vk_descriptor_sets.size())) + " sets");
    }

    // If we do, prepare a set of n times the same descriptor set layout
    Tools::Array<VkDescriptorSetLayout> vk_descriptor_set_layouts(descriptor_set_layouts.size());
    for (size_t i = 0; i < descriptor_set_layouts.size(); i++) {
        vk_descriptor_set_layouts.push_back(descriptor_set_layouts[i]);
    }

    // Next, populate the create info
    VkDescriptorSetAllocateInfo descriptor_set_info;
    populate_descriptor_set_info(descriptor_set_info, this->vk_descriptor_pool, vk_descriptor_set_layouts, n_sets);

    // We can now call the allocate function
    VkResult vk_result;
    if ((vk_result = vkAllocateDescriptorSets(this->gpu, &descriptor_set_info, this->vk_descriptor_sets.wdata(this->vk_descriptor_sets.size() + n_sets) + this->vk_descriptor_sets.size() - n_sets)) != VK_SUCCESS) {
        DLOG(fatal, "Failed to allocate " + std::to_string(n_sets) + " new DescriptorSets: " + vk_error_map[vk_result]);
    }

    // Construct the set of new sets
    Tools::Array<DescriptorSet> result(n_sets);
    for (size_t i = this->vk_descriptor_sets.size() - n_sets; i < this->vk_descriptor_sets.size(); i++) {
        result.push_back(DescriptorSet(this->vk_descriptor_sets[i]));
    }

    // Return a reference to this descriptor set
    DRETURN result;
}

/* Deallocates the given descriptor set. */
void DescriptorPool::deallocate(const DescriptorSet& descriptor_set) {
    DENTER("Compute::DescriptorPool::deallocate");

    // First, check to see if we know this one
    for (size_t i = 0; i < this->vk_descriptor_sets.size(); i++) {
        if (this->vk_descriptor_sets[i] == descriptor_set.vk_descriptor_set) {
            // Deallocate it
            VkResult vk_result;
            if ((vk_result = vkFreeDescriptorSets(this->gpu, this->vk_descriptor_pool, 1, &this->vk_descriptor_sets[i])) != VK_SUCCESS) {
                DLOG(fatal, "Could not free descriptor set: " + vk_error_map[vk_result]);
            }

            // Erase it from the list
            this->vk_descriptor_sets.erase(i);

            // Done
            DRETURN;
        }
    }

    // It wasn't found
    DLOG(fatal, "Given descriptor set is not allocated with this pool");
    DRETURN;
}

/* Deallocates an array of given descriptors set. */
void DescriptorPool::ndeallocate(const Tools::Array<DescriptorSet>& descriptor_sets) {
    DENTER("Compute::DescriptorPool::ndeallocate");

    // First, collect a list of vulkan descriptor sets
    Tools::Array<VkDescriptorSet> to_free(descriptor_sets.size()); 
    for (size_t i = 0; i < descriptor_sets.size(); i++) {
        for (size_t j = 0; j < this->vk_descriptor_sets.size(); j++) {
            if (this->vk_descriptor_sets[j] == descriptor_sets[i].vk_descriptor_set) {
                // Mark it for deallocation
                to_free.push_back(descriptor_sets[i]);

                // Remove it from the list
                this->vk_descriptor_sets.erase(i);

                // Done
                break;
            }
        }
    }

    // If the sizes are not equal, then some of them aren't found
    if (to_free.size() != descriptor_sets.size()) {
        DLOG(fatal, "Not all given descriptor sets are allocated with this pool");
    }

    // If they do match, then do a single free call to deallocate them all
    VkResult vk_result;
    if ((vk_result = vkFreeDescriptorSets(this->gpu, this->vk_descriptor_pool, static_cast<uint32_t>(to_free.size()), to_free.rdata())) != VK_SUCCESS) {
        DLOG(fatal, "Could not free descriptor sets: " + vk_error_map[vk_result]);
    }

    // Done!
    DRETURN;
}



/* Swap operator for the DescriptorPool class. */
void Compute::swap(DescriptorPool& dp1, DescriptorPool& dp2) {
    DENTER("Compute::swap(DescriptorPool)");

    using std::swap;

    #ifndef NDEBUG
    // If the GPU is not the same, then initialize to all nullptrs and everything
    if (dp1.gpu != dp2.gpu) {
        DLOG(fatal, "Cannot swap descriptor pools with different GPUs");
    }
    #endif

    // Swap all fields
    swap(dp1.vk_descriptor_pool, dp2.vk_descriptor_pool);
    swap(dp1.vk_descriptor_types, dp2.vk_descriptor_types);
    swap(dp1.vk_max_sets, dp2.vk_max_sets);
    swap(dp1.vk_create_flags, dp2.vk_create_flags);
    swap(dp1.vk_descriptor_sets, dp2.vk_descriptor_sets);

    // Done
    DRETURN;
}
