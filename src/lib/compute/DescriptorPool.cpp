/* DESCRIPTOR POOL.cpp
 *   by Lut99
 *
 * Created:
 *   26/04/2021, 14:38:48
 * Last edited:
 *   26/04/2021, 16:57:37
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Contains the DescriptorPool class, which serves as a memory pool for
 *   descriptors, which in turn describe how a certain buffer or other
 *   piece of memory should be accessed on the GPU.
**/

#include <CppDebugger.hpp>

#include "ErrorCodes.hpp"

#include "DescriptorPool.hpp"

using namespace std;
using namespace RayTracer;
using namespace RayTracer::Compute;
using namespace CppDebugger::SeverityValues;


/***** POPULATE FUNCTIONS *****/
/* Populates a given VkDescriptorPoolSize struct. */
static void populate_descriptor_pool_size(VkDescriptorPoolSize& descriptor_pool_size, VkDescriptorType descriptor_type, uint32_t n_descriptors) {
    DENTER("populate_descriptor_pool_size");

    // Initialize to default
    descriptor_pool_size = {};
    descriptor_pool_size.type = descriptor_type;
    descriptor_pool_size.descriptorCount = n_descriptors;

    // Done;
    DRETURN;
}

/* Populates a given VkDescriptorPoolCreateInfo struct. */
static void populate_descriptor_pool_info(VkDescriptorPoolCreateInfo& descriptor_pool_info, const VkDescriptorPoolSize& descriptor_pool_size, uint32_t n_sets, VkDescriptorPoolCreateFlags descriptor_pool_flags) {
    DENTER("populate_descriptor_pool_info");

    // Initialize to default
    descriptor_pool_info = {};
    descriptor_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;

    // Set the pool size to use
    descriptor_pool_info.poolSizeCount = 1;
    descriptor_pool_info.pPoolSizes = &descriptor_pool_size;

    // Set the maximum number of sets allowed
    descriptor_pool_info.maxSets = n_sets;

    // Set the flags to use
    descriptor_pool_info.flags = descriptor_pool_flags | VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    // Done;
    DRETURN;
}

/* Populates a given VkDescriptorSetAllocateInfo struct. */
static void populate_descriptor_set_info(VkDescriptorSetAllocateInfo& descriptor_set_info, VkDescriptorPool vk_descriptor_pool, const DescriptorSetLayout& descriptor_set_layout, uint32_t n_sets) {
    DENTER("populate_descriptor_set_info");

    // Set to default
    descriptor_set_info = {};
    descriptor_set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;

    // Set the pool that we use to allocate them
    descriptor_set_info.descriptorPool = vk_descriptor_pool;

    // Set the number of sets to allocate
    descriptor_set_info.descriptorSetCount = n_sets;

    // Set the layout for this descriptor
    descriptor_set_info.pSetLayouts = &descriptor_set_layout.descriptor_set_layout();

    // Done
    DRETURN;
}





/***** DESCRIPTORSET CLASS *****/
/* Constructor for the DescriptorSet class, which takes the vk_descriptor_set it wraps. */
DescriptorSet::DescriptorSet(VkDescriptorSet descriptor_set) :
    vk_descriptor_set(descriptor_set)
{}





/***** DESCRIPTORPOOL CLASS *****/
/* Constructor for the DescriptorPool class, which takes the GPU to create the pool on, the number of descriptors we want to allocate in the pool, the maximum number of descriptor sets that can be allocated and optionally custom create flags. */
DescriptorPool::DescriptorPool(const GPU& gpu, VkDescriptorType descriptor_type, uint32_t n_descriptors, uint32_t max_sets, VkDescriptorPoolCreateFlags flags):
    gpu(gpu),
    max_size(max_sets),
    vk_descriptor_sets(max_sets)
{
    DENTER("Compute::DescriptorPool::DescriptorPool");
    DLOG(info, "Initializing DescriptorPool...");
    DINDENT;



    // First, we define how large the pool will be
    DLOG(info, "Preparing structs...");
    VkDescriptorPoolSize descriptor_pool_size;
    populate_descriptor_pool_size(descriptor_pool_size, descriptor_type, n_descriptors);

    // Prepare the create info
    VkDescriptorPoolCreateInfo descriptor_pool_info;
    populate_descriptor_pool_info(descriptor_pool_info, descriptor_pool_size, max_sets, flags);



    // Actually allocate the pool
    DLOG(info, "Allocating pool...");
    VkResult vk_result;
    if ((vk_result = vkCreateDescriptorPool(this->gpu, &descriptor_pool_info, nullptr, &this->vk_descriptor_pool)) != VK_SUCCESS) {
        DLOG(fatal, "Could not allocate descriptor pool: " + vk_error_map[vk_result]);
    }



    DDEDENT;
    DLEAVE;
}

/* Move constructor for the DescriptorPool. */
DescriptorPool::DescriptorPool(DescriptorPool&& other):
    gpu(other.gpu),
    vk_descriptor_pool(other.vk_descriptor_pool),
    max_size(other.max_size),
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
        if ((vk_result = vkFreeDescriptorSets(this->gpu, this->vk_descriptor_pool, this->vk_descriptor_sets.size(), this->vk_descriptor_sets.rdata())) != VK_SUCCESS) {
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
    if (static_cast<uint32_t>(this->vk_descriptor_sets.size()) >= this->max_size) {
        DLOG(fatal, "Cannot allocate new DescriptorSet: already allocated maximum of " + std::to_string(this->max_size) + " sets.");
    }

    // Next, populate the create info
    VkDescriptorSetAllocateInfo descriptor_set_info;
    populate_descriptor_set_info(descriptor_set_info, this->vk_descriptor_pool, descriptor_set_layout, 1);

    // We can now call the allocate function
    VkResult vk_result;
    if ((vk_result = vkAllocateDescriptorSets(this->gpu, &descriptor_set_info, this->vk_descriptor_sets.wdata(this->vk_descriptor_sets.size() + 1) + this->vk_descriptor_sets.size() - 1)) != VK_SUCCESS) {
        DLOG(fatal, "Failed to allocate new DescriptorSet: " + vk_error_map[vk_result]);
    }

    // Return a reference to this descriptor set
    DRETURN DescriptorSet(this->vk_descriptor_sets[this->vk_descriptor_sets.size() - 1]);
}

/* Allocates multiple descriptor sets with the given layout, returning them as an Array. Will fail with errors if there's no more space. */
Tools::Array<DescriptorSet> DescriptorPool::nallocate(uint32_t n_sets, const DescriptorSetLayout& descriptor_set_layout) {
    DENTER("Compute::DescriptorPool::nallocate");

    #ifndef NDEBUG
    // If n_sets if null, nothing to do
    if (n_sets == 0) {
        DLOG(warning, "Request to allocate 0 sets received; nothing to do.");
    }
    #endif

    // Check if we have enough space left
    if (static_cast<uint32_t>(this->vk_descriptor_sets.size()) + n_sets >= this->max_size) {
        DLOG(fatal, "Cannot allocate " + std::to_string(n_sets) + " new DescriptorSets: only space for " + std::to_string(this->max_size - static_cast<uint32_t>(this->vk_descriptor_sets.size())) + "sets");
    }

    // Next, populate the create info
    VkDescriptorSetAllocateInfo descriptor_set_info;
    populate_descriptor_set_info(descriptor_set_info, this->vk_descriptor_pool, descriptor_set_layout, n_sets);

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



/* Move assignment operator for the DescriptorPool class. */
DescriptorPool& DescriptorPool::operator=(DescriptorPool&& other) {
    if (this != &other) { swap(*this, other); }
    return *this;
}

/* Swap operator for the DescriptorPool class. */
void Compute::swap(DescriptorPool& dp1, DescriptorPool& dp2) {
    DENTER("Compute::swap(DescriptorPool)");

    using std::swap;

    #ifndef NDEBUG
    // If the GPU is not the same, then initialize to all nullptrs and everything
    if (dp1.gpu.name() != dp2.gpu.name()) {
        DLOG(fatal, "Cannot swap descriptor pools with different GPUs");
    }
    #endif

    // Swap all fields
    swap(dp1.vk_descriptor_pool, dp2.vk_descriptor_pool);
    swap(dp1.max_size, dp2.max_size);
    swap(dp1.vk_descriptor_sets, dp2.vk_descriptor_sets);

    // Done
    DRETURN;
}
