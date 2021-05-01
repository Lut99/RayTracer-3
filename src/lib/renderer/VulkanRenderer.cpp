/* VULKAN RENDERER.cpp
 *   by Lut99
 *
 * Created:
 *   30/04/2021, 13:34:23
 * Last edited:
 *   01/05/2021, 13:36:56
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Implementation of the Renderer class that uses Vulkan compute shaders.
**/

#include <CppDebugger.hpp>

#include "compute/ErrorCodes.hpp"

#include "VulkanRenderer.hpp"

using namespace std;
using namespace RayTracer;
using namespace RayTracer::Compute;
using namespace RayTracer::ECS;
using namespace CppDebugger::SeverityValues;


/***** VULKANRENDERER CLASS *****/
/* Constructor for the VulkanRenderer class. */
VulkanRenderer::VulkanRenderer() :
    Renderer()
{
    DENTER("VulkanRenderer::VulkanRenderer");
    DLOG(info, "Initializing the Vulkan-based renderer...");
    DINDENT;

    // First, initialize the gpu
    this->gpu = new GPU();

    // Before we continue, select suitable memory types for each pool.
    uint32_t device_memory_type = MemoryPool::select_memory_type(*this->gpu, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    uint32_t stage_memory_type = MemoryPool::select_memory_type(*this->gpu, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    // Next, initialize all  pools
    this->device_memory_pool = new MemoryPool(*this->gpu, device_memory_type, VulkanRenderer::device_memory_size, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    this->stage_memory_pool = new MemoryPool(*this->gpu, stage_memory_type, VulkanRenderer::stage_memory_size, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    this->descriptor_pool = new DescriptorPool(*this->gpu, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VulkanRenderer::max_descriptors, VulkanRenderer::max_descriptor_sets);
    this->compute_command_pool = new CommandPool(*this->gpu, this->gpu->queue_info().compute());
    this->memory_command_pool = new CommandPool(*this->gpu, this->gpu->queue_info().memory(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    DDEDENT;
    DLEAVE;
}

/* Copy constructor for the VulkanRenderer class. */
VulkanRenderer::VulkanRenderer(const VulkanRenderer& other) :
    Renderer(other),
    entity_vertices(other.entity_vertices),
    entity_normals(other.entity_normals),
    entity_colors(other.entity_colors),
    n_entity_vertices(other.n_entity_vertices)
{
    DENTER("VulkanRenderer::VulkanRenderer(copy)");

    // Copy the GPU
    this->gpu = new GPU(*other.gpu);

    // Copy all pools
    this->device_memory_pool = new MemoryPool(*other.device_memory_pool);
    this->stage_memory_pool = new MemoryPool(*other.stage_memory_pool);
    this->descriptor_pool = new DescriptorPool(*other.descriptor_pool);
    this->compute_command_pool = new CommandPool(*other.compute_command_pool);
    this->memory_command_pool = new CommandPool(*other.memory_command_pool);

    DLEAVE;
}

/* Move constructor for the VulkanRenderer class. */
VulkanRenderer::VulkanRenderer(VulkanRenderer&& other) :
    Renderer(other),
    gpu(other.gpu),
    device_memory_pool(other.device_memory_pool),
    stage_memory_pool(other.stage_memory_pool),
    descriptor_pool(other.descriptor_pool),
    compute_command_pool(other.compute_command_pool),
    memory_command_pool(other.memory_command_pool),
    entity_vertices(other.entity_vertices),
    entity_normals(other.entity_normals),
    entity_colors(other.entity_colors),
    n_entity_vertices(other.n_entity_vertices)
{
    // Set the other's deallocateable pointers to nullptrs to avoid just that
    other.gpu = nullptr;
    other.device_memory_pool = nullptr;
    other.stage_memory_pool = nullptr;
    other.descriptor_pool = nullptr;
    other.compute_command_pool = nullptr;
    other.memory_command_pool = nullptr;
}

/* Destructor for the VulkanRenderer class. */
VulkanRenderer::~VulkanRenderer() {
    DENTER("VulkanRenderer::~VulkanRenderer");
    DLOG(info, "Cleaning renderer...");
    DINDENT;

    if (this->memory_command_pool != nullptr) {
        delete this->memory_command_pool;
    }
    if (this->compute_command_pool != nullptr) {
        delete this->compute_command_pool;
    }
    if (this->descriptor_pool != nullptr) {
        delete this->descriptor_pool;
    }
    if (this->stage_memory_pool != nullptr) {
        delete this->stage_memory_pool;
    }
    if (this->device_memory_pool != nullptr) {
        delete this->device_memory_pool;
    }

    if (this->gpu != nullptr) {
        delete this->gpu;
    }

    DDEDENT;
    DLEAVE;
}



/* Pre-renders the given list of RenderEntities, accelerated using Vulkan compute shaders. */
void VulkanRenderer::prerender(const Tools::Array<ECS::RenderEntity>& entities) {
    DENTER("VulkanRenderer::prerender");

    // We start by throwing out any old vertices we might have
    this->entity_vertices.clear();
    this->entity_normals.clear();
    this->entity_colors.clear();
    this->n_entity_vertices = 0;

    // Next, loop through all entities to render them
    for (size_t i = 0; i < entities.size(); i++) {
        // Select the proper pre-render mode
        if (entities[i].pre_render_mode & EntityPreRenderModeFlags::gpu) {
            // Determine the type of pre-rendering operation we need to do
            switch (entities[i].pre_render_operation) {
                // case EntityPreRenderOperation::generate_sphere:
                //     /* Prepare the pipeline using this shader. */
                //     break;

                default:
                    DLOG(fatal, "Entity " + std::to_string(i) + " wants to be pre-rendered on the CPU using unsupported operation '" + entity_pre_render_operation_names[entities[i].pre_render_operation] + "'.");

            }

        } else if (entities[i].pre_render_mode & EntityPreRenderModeFlags::cpu) {
            // Determine the type of pre-rendering operation we need to do
            switch (entities[i].pre_render_operation) {
                case EntityPreRenderOperation::generate_sphere:
                    /* Call the generate sphere CPU function. */
                    break;

                default:
                    DLOG(fatal, "Entity " + std::to_string(i) + " wants to be pre-rendered on the GPU using unsupported operation '" + entity_pre_render_operation_names[entities[i].pre_render_operation] + "'.");

            }

        } else {
            DLOG(fatal, "Entity " + std::to_string(i) + " of type " + entity_type_names[entities[i].type] + " with the Vulkan compute shader back-end.");
        }
    }

    DRETURN;
}

/* Renders the internal list of vertices to a frame using the given camera position. */
void VulkanRenderer::render(Camera& camera) const {
    DENTER("VulkanRenderer::render");

    

    DRETURN;
}



/* Swap operator for the VulkanRenderer class. */
void RayTracer::swap(VulkanRenderer& r1, VulkanRenderer& r2) {
    using std::swap;

    swap(r1.gpu, r2.gpu);

    swap(r1.device_memory_pool, r2.device_memory_pool);
    swap(r1.stage_memory_pool, r2.stage_memory_pool);
    swap(r1.descriptor_pool, r2.descriptor_pool);
    swap(r1.compute_command_pool, r2.compute_command_pool);
    swap(r1.memory_command_pool, r2.memory_command_pool);

    swap(r1.entity_vertices, r2.entity_vertices);
    swap(r1.entity_normals, r2.entity_normals);
    swap(r1.entity_colors, r2.entity_colors);
    swap(r1.n_entity_vertices, r2.n_entity_vertices);

}





/***** FACTORY METHOD *****/
Renderer* RayTracer::initialize_renderer() {
    DENTER("initialize_renderer");



    DRETURN nullptr;
}
