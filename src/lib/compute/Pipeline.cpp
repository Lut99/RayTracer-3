/* PIPELINE.cpp
 *   by Lut99
 *
 * Created:
 *   27/04/2021, 14:44:19
 * Last edited:
 *   27/04/2021, 18:21:16
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Contains the Pipeline class, which defines the compute pipeline, and
 *   thus how the shaders are organised and how they get their data.
**/

#include <CppDebugger.hpp>

#include "ErrorCodes.hpp"

#include "Pipeline.hpp"

using namespace std;
using namespace RayTracer;
using namespace RayTracer::Compute;
using namespace CppDebugger::SeverityValues;


/***** POPULATE FUNCTIONS *****/
/* Function that populates the given VkPipelineShaderStageCreateInfo struct using the given Shader object. */
static void populate_shader_stage_info(VkPipelineShaderStageCreateInfo& shader_stage_info, const Shader& shader) {
    DENTER("populate_compute_info");

    // Set to default
    shader_stage_info = {};
    shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

    // Tell Vulkan that this shader is a compute shader
    shader_stage_info.stage = VK_SHADER_STAGE_COMPUTE_BIT;

    // Pass the module we're passing
    shader_stage_info.module = shader;

    // Tell Vulkan which function it should start at
    shader_stage_info.pName = shader.entry_function().c_str();

    // Optionally, we can bind certain constants here before real compilation; but we don't
    shader_stage_info.pSpecializationInfo = nullptr;

    // Done
    DRETURN;
}

/* Function that populates the given VkPipelineLayoutCreateInfo struct using the given values. */
static void populate_layout_info(VkPipelineLayoutCreateInfo& layout_info, const Tools::Array<VkDescriptorSetLayout>& vk_descriptor_set_layouts) {
    DENTER("populate_layout_info");

    // Set to default
    layout_info = {};
    layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    // Set the layouts to use
    layout_info.setLayoutCount = static_cast<uint32_t>(vk_descriptor_set_layouts.size());
    layout_info.pSetLayouts = vk_descriptor_set_layouts.rdata();

    // Also set the pushconstants to use; but we don't use any, at least not for now
    layout_info.pushConstantRangeCount = 0;
    layout_info.pPushConstantRanges = nullptr;

    // Done
    DRETURN;
}

/* Function that populates the given VkComputePipelineCreateInfo using the given values. */
static void populate_compute_info(VkComputePipelineCreateInfo& compute_info, VkPipelineLayout pipeline_layout, const VkPipelineShaderStageCreateInfo& shader_stage_info) {
    DENTER("populate_compute_info");

    // Set to default
    compute_info = {};
    compute_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;

    // Define the layout of the pipeline
    compute_info.layout = pipeline_layout;

    // Define the shader(s) to use
    compute_info.stage = shader_stage_info;

    // Done
    DRETURN;
}





/***** PIPELINE CLASS *****/
/* Constructor for the Pipeline class, which takes the GPU where we create it for, the shader to load and the set of descriptor set layouts which describe all buffers used in the pipeline.  */
Pipeline::Pipeline(const GPU& gpu, const Shader& shader, const Tools::Array<DescriptorSetLayout>& descriptor_set_layouts) :
    gpu(gpu)
{
    DENTER("Compute::Pipeline::Pipeline");
    DLOG(info, "Initializing Pipeline...");
    DINDENT;

    // Prepare the shader stage description for the shader
    DLOG(info, "Preparing shader...");
    VkPipelineShaderStageCreateInfo shader_stage_info;
    populate_shader_stage_info(shader_stage_info, shader);



    // Next, we'll define the layout for the Pipeline
    DLOG(info, "Preparing pipeline layout...");

    // Create an array with the raw VkDescriptorSetLayouts
    Tools::Array<VkDescriptorSetLayout> vk_descriptor_set_layouts(descriptor_set_layouts.size());
    for (size_t i = 0; i < descriptor_set_layouts.size(); i++) {
        vk_descriptor_set_layouts.push_back(descriptor_set_layouts[i]);
    }

    // Use that to populate the struct
    VkPipelineLayoutCreateInfo layout_info;
    populate_layout_info(layout_info, vk_descriptor_set_layouts);

    // Create the layout
    VkResult vk_result;
    if ((vk_result = vkCreatePipelineLayout(this->gpu, &layout_info, nullptr, &this->vk_compute_pipeline_layout)) != VK_SUCCESS) {
        DLOG(fatal, "Could not create pipeline layout: " + vk_error_map[vk_result]);
    }



    // We start by populating the VkComputePipelineCreateInfo struct
    DLOG(info, "Constructing pipeline...");
    VkComputePipelineCreateInfo compute_info;
    populate_compute_info(compute_info, this->vk_compute_pipeline_layout, shader_stage_info);

    // Call the create function!
    if ((vk_result = vkCreateComputePipelines(this->gpu, VK_NULL_HANDLE, 1, &compute_info, nullptr, &this->vk_compute_pipeline)) != VK_SUCCESS) {
        DLOG(fatal, "Could not create compute pipeline: " + vk_error_map[vk_result]);
    }



    // Done :)
    DDEDENT;
    DLEAVE;
}

/* Move constructor for the Pipeline class. */
Pipeline::Pipeline(Pipeline&& other) :
    gpu(other.gpu),
    vk_compute_pipeline(other.vk_compute_pipeline),
    vk_compute_pipeline_layout(other.vk_compute_pipeline_layout)
{
    // Set the pipeline to nullptr to avoid deallocation
    other.vk_compute_pipeline = nullptr;
    other.vk_compute_pipeline_layout = nullptr;
}

/* Destructor for the Pipeline class. */
Pipeline::~Pipeline() {
    DENTER("Compute::Pipeline::~Pipeline");
    DLOG(info, "Cleaning Pipeline...");
    DINDENT;

    if (this->vk_compute_pipeline_layout != nullptr) {
        DLOG(info, "Destroying pipeline layout...");
        vkDestroyPipelineLayout(this->gpu, this->vk_compute_pipeline_layout, nullptr);
    }

    if (this->vk_compute_pipeline != nullptr) {
        DLOG(info, "Destroying pipeline...");
        vkDestroyPipeline(this->gpu, this->vk_compute_pipeline, nullptr);
    }

    DDEDENT;
    DLEAVE;
}



/* Schedules the compute pipeline in the given CommandBuffer. Note that we assume the command buffer has already been started. */
void Pipeline::bind(const CommandBuffer& buffer) {
    DENTER("Compute::Pipeline::schedule");

    // Only one command, ez game
    vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_COMPUTE, this->vk_compute_pipeline);

    DRETURN;
}



/* Move assignment operator for the Pipeline class. */
Pipeline& Pipeline::operator=(Pipeline&& other) {
    if (this != &other) { swap(*this, other); }
    return *this;
}

/* Swap operator for the Pipeline class. */
void Compute::swap(Pipeline& p1, Pipeline& p2) {
    DENTER("Compute::swap(Pipeline)");

    using std::swap;

    #ifndef NDEBUG
    // If the GPU is not the same, then initialize to all nullptrs and everything
    if (p1.gpu.name() != p2.gpu.name()) {
        DLOG(fatal, "Cannot swap pipelines with different GPUs");
    }
    #endif

    // Swap all fields
    swap(p1.vk_compute_pipeline, p2.vk_compute_pipeline);
    swap(p1.vk_compute_pipeline_layout, p2.vk_compute_pipeline_layout);

    // Done
    DRETURN;
}
