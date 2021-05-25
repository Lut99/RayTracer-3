/* PIPELINE.cpp
 *   by Lut99
 *
 * Created:
 *   27/04/2021, 14:44:19
 * Last edited:
 *   25/05/2021, 17:23:26
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Contains the Pipeline class, which defines the compute pipeline, and
 *   thus how the shaders are organised and how they get their data.
**/

#include <any>
#include "debugger/CppDebugger.hpp"

#include "ErrorCodes.hpp"

#include "Pipeline.hpp"

using namespace std;
using namespace RayTracer;
using namespace RayTracer::Compute;
using namespace CppDebugger::SeverityValues;


/***** HELPER FUNCTIONS *****/
/* Given an unordered map of constants, creates an array of VkSpecializationMapEntry's and a matching array of data as void*. */
static void flatten_specialization_map(const std::unordered_map<uint32_t, std::tuple<uint32_t, void*>>& constant_map, Tools::Array<VkSpecializationMapEntry>& map_entries, void*& data, uint32_t& data_size) {
    DENTER("flatten_specialization_map");

    // Create an array of mapentries, and then a single, unified data array
    uint32_t offset = 0;
    map_entries.reserve(constant_map.size());
    for(const pair<uint32_t, std::tuple<uint32_t, void*>>& p : constant_map) {
        // Start to intialize a map entry
        VkSpecializationMapEntry map_entry{};

        // Set the constant ID for this entry
        map_entry.constantID = p.first;

        // Set the offset & size in the global constant array
        map_entry.offset = offset;
        map_entry.size = std::get<0>(p.second);

        // Add it to the array
        map_entries.push_back(map_entry);

        // Increment the offset
        offset += std::get<0>(p.second);
    }

    // The offset is now also the size. Use that to populate the data array
    data_size = offset;
    offset = 0;
    data = malloc(data_size);
    for (const pair<uint32_t, std::tuple<uint32_t, void*>>& p : constant_map) {
        // Copy the element's data to the array
        memcpy((void*) (((uint8_t*) data) + offset), std::get<1>(p.second), std::get<0>(p.second));

        // Increment the offset once more
        offset += std::get<0>(p.second);
    }

    // Done
    DRETURN;
}





/***** POPULATE FUNCTIONS *****/
/* Function that populates a VkSpecializationInfo struct based on the given map of constant_ids to values. */
static void populate_specialization_info(VkSpecializationInfo& specialization_info, const Tools::Array<VkSpecializationMapEntry>& map_entries, void* data, uint32_t& data_size) {
    DENTER("populate_specialization_info");

    // Initialize to default
    specialization_info = {};
    
    // Set the specialization map
    specialization_info.mapEntryCount = static_cast<uint32_t>(map_entries.size());
    specialization_info.pMapEntries = map_entries.rdata();

    // Set the data array
    specialization_info.dataSize = data_size;
    specialization_info.pData = data;

    // Done
    DRETURN;
}

/* Function that populates the given VkPipelineShaderStageCreateInfo struct using the given Shader object. */
static void populate_shader_stage_info(VkPipelineShaderStageCreateInfo& shader_stage_info, const Shader& shader, const VkSpecializationInfo& specialization_info) {
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
    shader_stage_info.pSpecializationInfo = &specialization_info;

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
/* Constructor for the Pipeline class, which takes the GPU where we create it for, the shader to load and the set of descriptor set layouts which describe all buffers used in the pipeline. Optionally, a map of specialization constants can be given.  */
Pipeline::Pipeline(const GPU& gpu, const Shader& shader, const Tools::Array<DescriptorSetLayout>& descriptor_set_layouts, const std::unordered_map<uint32_t, std::tuple<uint32_t, void*>>& constant_map) :
    gpu(gpu)
{
    DENTER("Compute::Pipeline::Pipeline");
    DLOG(info, "Initializing Pipeline...");
    DINDENT;

    // Prepare the shader stage description for the shader
    DLOG(info, "Preparing shader...");

    // First, flatten the given map of constants
    Tools::Array<VkSpecializationMapEntry> map_entries;
    void* data;
    uint32_t data_size;
    flatten_specialization_map(constant_map, map_entries, data, data_size);

    // Use that to populate the spezialization struct
    VkSpecializationInfo specialization_info;
    populate_specialization_info(specialization_info, map_entries, data, data_size);

    // Finally, use THAT to populate the shader stage info
    VkPipelineShaderStageCreateInfo shader_stage_info;
    populate_shader_stage_info(shader_stage_info, shader, specialization_info);



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



    // Done; don't forget to delete the array of constants
    free(data);
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
void Pipeline::bind(const CommandBuffer& buffer) const {
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
    if (p1.gpu != p2.gpu) {
        DLOG(fatal, "Cannot swap pipelines with different GPUs");
    }
    #endif

    // Swap all fields
    swap(p1.vk_compute_pipeline, p2.vk_compute_pipeline);
    swap(p1.vk_compute_pipeline_layout, p2.vk_compute_pipeline_layout);

    // Done
    DRETURN;
}
