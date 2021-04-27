/* DESCRIPTOR SET LAYOUT.cpp
 *   by Lut99
 *
 * Created:
 *   26/04/2021, 15:33:41
 * Last edited:
 *   27/04/2021, 16:49:16
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Contains the DescriptorSetLayout class, which describes the format for
 *   a single type of buffer.
**/

#include <CppDebugger.hpp>

#include "ErrorCodes.hpp"

#include "DescriptorSetLayout.hpp"

using namespace std;
using namespace RayTracer;
using namespace RayTracer::Compute;
using namespace CppDebugger::SeverityValues;


/***** POPULATE FUNCTIONS *****/
/* Populates given VkDescriptorSetLayoutBinding struct. */
static void populate_descriptor_set_binding(VkDescriptorSetLayoutBinding& descriptor_set_binding, uint32_t bind_index, VkDescriptorType descriptor_type, VkShaderStageFlags shader_stage) {
    DENTER("populate_descriptor_set_binding");

    // Set to default
    descriptor_set_binding = {};

    // Set the index of the binding, must be equal to the place in the shader
    descriptor_set_binding.binding = bind_index;
    
    // Set the type of the descriptor
    descriptor_set_binding.descriptorType = descriptor_type;
    
    // Set the number of descriptors to use
    descriptor_set_binding.descriptorCount = 1;
    
    // Set the stage flags
    descriptor_set_binding.stageFlags = shader_stage;
    
    // For now, we won't use the next field, as this is for multi-sampling and we don't use that yet
    descriptor_set_binding.pImmutableSamplers = nullptr;

    // Done
    DRETURN;
}

/* Populates a given VkDescriptorSetLayoutCreateInfo struct. */
static void populate_descriptor_set_layout_info(VkDescriptorSetLayoutCreateInfo& descriptor_set_layout_info, const VkDescriptorSetLayoutBinding& descriptor_set_binding) {
    DENTER("populate_descriptor_set_layout_info");

    // Initialize to default
    descriptor_set_layout_info = {};
    descriptor_set_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

    // Set the bindings to use
    descriptor_set_layout_info.bindingCount = 1;
    descriptor_set_layout_info.pBindings = &descriptor_set_binding;

    // Done!
    DRETURN;
}





/***** DESCRIPTORSETLAYOUT CLASS *****/
/* Constructor for the DescriptorSetLayout class, which takes a gpu to bind the buffer to, the index of this bind as seen on the shader, the type of the buffer we describe for and the shader stage where the uniform buffer will eventually be bound to. */
DescriptorSetLayout::DescriptorSetLayout(const GPU& gpu, uint32_t bind_index, VkDescriptorType descriptor_type, VkShaderStageFlags shader_stage) :
    gpu(gpu)
{
    DENTER("Compute::DescriptorSetLayout::DescriptorSetLayout");
    DLOG(info, "Initializing descriptor set layout...");

    // Define how to bind the layout descriptor (i.e., where)
    VkDescriptorSetLayoutBinding descriptor_set_binding;
    populate_descriptor_set_binding(descriptor_set_binding, bind_index, descriptor_type, shader_stage);

    // Next, we'll create the DescriptorSetLayout itself via a create info (who could've guessed)
    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_info;
    populate_descriptor_set_layout_info(descriptor_set_layout_info, descriptor_set_binding);
    
    // Finally, call the constructor
    VkResult vk_result;
    if ((vk_result = vkCreateDescriptorSetLayout(this->gpu, &descriptor_set_layout_info, nullptr, &this->vk_descriptor_set_layout)) != VK_SUCCESS) {
        DLOG(fatal, "Could not create descriptor set layout: " + vk_error_map[vk_result]);
    }

    // Done
    DLEAVE;
}

/* Move constructor for the DescriptorSetLayout class. */
DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout&& other) :
    gpu(other.gpu),
    vk_descriptor_set_layout(other.vk_descriptor_set_layout)
{
    // Set the layout to a nullptr to avoid deallocation
    other.vk_descriptor_set_layout = nullptr;
}

/* Destructor for the DescriptorSetLayout class. */
DescriptorSetLayout::~DescriptorSetLayout() {
    DENTER("Compute::DescriptorSetLayout::~DescriptorSetLayout");
    DLOG(info, "Cleaning DescriptorSetLayout...");

    if (this->vk_descriptor_set_layout != nullptr) {
        vkDestroyDescriptorSetLayout(this->gpu, this->vk_descriptor_set_layout, nullptr);
    }

    DLEAVE;
}



/* Move assignment operator for the DescriptorSetLayout class. */
DescriptorSetLayout& DescriptorSetLayout::operator=(DescriptorSetLayout&& other) {
    if (this != &other) { swap(*this, other); }
    return *this;
}

/* Swap operator for the DescriptorSetLayout class. */
void Compute::swap(DescriptorSetLayout& dsl1, DescriptorSetLayout& dsl2) {
    DENTER("Compute::swap(DescriptorSetLayout)");

    using std::swap;

    #ifndef NDEBUG
    // If the GPU is not the same, then initialize to all nullptrs and everything
    if (dsl1.gpu.name() != dsl2.gpu.name()) {
        DLOG(fatal, "Cannot swap descriptor set layouts with different GPUs");
    }
    #endif

    // Swap all fields
    swap(dsl1.vk_descriptor_set_layout, dsl2.vk_descriptor_set_layout);

    // Done
    DRETURN;
}
