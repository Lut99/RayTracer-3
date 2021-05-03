/* VULKAN RENDERER.cpp
 *   by Lut99
 *
 * Created:
 *   30/04/2021, 13:34:23
 * Last edited:
 *   03/05/2021, 13:57:12
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Implementation of the Renderer class that uses Vulkan compute shaders.
**/

#include <functional>
#include <CppDebugger.hpp>

#include "compute/Pipeline.hpp"
#include "compute/ErrorCodes.hpp"

#include "entities/Triangle.hpp"

#include "tools/Common.hpp"

#include "VulkanRenderer.hpp"

using namespace std;
using namespace RayTracer;
using namespace RayTracer::Compute;
using namespace RayTracer::ECS;
using namespace CppDebugger::SeverityValues;


/***** STRUCTS *****/
/* Struct used to carry camera data to the GPU. */
struct GCameraData {
    alignas(16) glm::vec3 origin;
    alignas(16) glm::vec3 horizontal;
    alignas(16) glm::vec3 vertical;
    alignas(16) glm::vec3 lower_left_corner;
};





/***** VULKANRENDERER CLASS *****/
/* Constructor for the VulkanRenderer class. */
VulkanRenderer::VulkanRenderer() :
    Renderer()
{
    DENTER("VulkanRenderer::VulkanRenderer");
    DLOG(info, "Initializing the Vulkan-based renderer...");
    DINDENT;

    // First, create a new instance
    this->instance = new Instance();

    // First, initialize the gpu
    this->gpu = new GPU(*this->instance);

    // Before we continue, select suitable memory types for each pool.
    uint32_t device_memory_type = MemoryPool::select_memory_type(*this->gpu, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    uint32_t stage_memory_type = MemoryPool::select_memory_type(*this->gpu, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    // Next, initialize all  pools
    this->device_memory_pool = new MemoryPool(*this->gpu, device_memory_type, VulkanRenderer::device_memory_size, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    this->stage_memory_pool = new MemoryPool(*this->gpu, stage_memory_type, VulkanRenderer::stage_memory_size, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    this->descriptor_pool = new DescriptorPool(*this->gpu, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VulkanRenderer::max_descriptors, VulkanRenderer::max_descriptor_sets);
    this->compute_command_pool = new CommandPool(*this->gpu, this->gpu->queue_info().compute());
    this->memory_command_pool = new CommandPool(*this->gpu, this->gpu->queue_info().memory(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    // Initialize the descriptor set layouts
    this->raytrace_dsl = new DescriptorSetLayout(*this->gpu);
    this->raytrace_dsl->add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT);
    this->raytrace_dsl->add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT);
    this->raytrace_dsl->add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT);
    this->raytrace_dsl->add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT);
    this->raytrace_dsl->finalize();

    // Allocate re-useable command buffers
    this->staging_cb_h = this->memory_command_pool->allocate();

    // Finally, initialize the pipelines
    

    DDEDENT;
    DLEAVE;
}

/* Copy constructor for the VulkanRenderer class. */
VulkanRenderer::VulkanRenderer(const VulkanRenderer& other) :
    Renderer(other),
    entity_vertices(other.entity_vertices),
    entity_points(other.entity_points)
{
    DENTER("VulkanRenderer::VulkanRenderer(copy)");

    // Copy the instance
    this->instance = new Instance(*other.instance);

    // Copy the GPU
    this->gpu = new GPU(*other.gpu);

    // Copy all pools
    this->device_memory_pool = new MemoryPool(*other.device_memory_pool);
    this->stage_memory_pool = new MemoryPool(*other.stage_memory_pool);
    this->descriptor_pool = new DescriptorPool(*other.descriptor_pool);
    this->compute_command_pool = new CommandPool(*other.compute_command_pool);
    this->memory_command_pool = new CommandPool(*other.memory_command_pool);

    // Copy the descriptor set layouts
    this->raytrace_dsl = new DescriptorSetLayout(*other.raytrace_dsl);

    // And copy command buffers
    this->staging_cb_h = this->memory_command_pool->allocate();

    DLEAVE;
}

/* Move constructor for the VulkanRenderer class. */
VulkanRenderer::VulkanRenderer(VulkanRenderer&& other) :
    Renderer(other),
    instance(other.instance),
    gpu(other.gpu),
    device_memory_pool(other.device_memory_pool),
    stage_memory_pool(other.stage_memory_pool),
    descriptor_pool(other.descriptor_pool),
    compute_command_pool(other.compute_command_pool),
    memory_command_pool(other.memory_command_pool),
    raytrace_dsl(other.raytrace_dsl),
    staging_cb_h(other.staging_cb_h),
    entity_vertices(other.entity_vertices),
    entity_points(other.entity_points)
{
    // Set the other's deallocateable pointers to nullptrs to avoid just that
    other.instance = nullptr;
    other.gpu = nullptr;
    other.device_memory_pool = nullptr;
    other.stage_memory_pool = nullptr;
    other.descriptor_pool = nullptr;
    other.compute_command_pool = nullptr;
    other.memory_command_pool = nullptr;
    other.raytrace_dsl = nullptr;
}

/* Destructor for the VulkanRenderer class. */
VulkanRenderer::~VulkanRenderer() {
    DENTER("VulkanRenderer::~VulkanRenderer");
    DLOG(info, "Cleaning renderer...");
    DINDENT;

    if (this->raytrace_dsl != nullptr) {
        delete this->raytrace_dsl;
    }

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
    if (this->instance != nullptr) {
        delete this->instance;
    }

    DDEDENT;
    DLEAVE;
}



/* Pre-renders the given list of RenderEntities, accelerated using Vulkan compute shaders. */
void VulkanRenderer::prerender(const Tools::Array<ECS::RenderEntity*>& entities) {
    DENTER("VulkanRenderer::prerender");

    // We start by throwing out any old vertices we might have
    this->entity_vertices.clear();
    this->entity_points.clear();

    // Prepare the buffers for the per-entity vertices
    Tools::Array<Vertex> vertices;

    // Next, loop through all entities to render them
    for (size_t i = 0; i < entities.size(); i++) {
        // Clean the temporary buffers
        vertices.clear();

        // Select the proper pre-render mode
        if (entities[i]->pre_render_mode & EntityPreRenderModeFlags::eprmf_gpu) {
            // Determine the type of pre-rendering operation we need to do
            switch (entities[i]->pre_render_operation) {
                // case EntityPreRenderOperation::generate_sphere:
                //     /* Prepare the pipeline using this shader. */
                //     break;

                default:
                    DLOG(fatal, "Entity " + std::to_string(i) + " wants to be pre-rendered on the CPU using unsupported operation '" + entity_pre_render_operation_names[entities[i]->pre_render_operation] + "'.");

            }

        } else if (entities[i]->pre_render_mode & EntityPreRenderModeFlags::eprmf_cpu) {
            // Determine the type of pre-rendering operation we need to do
            switch (entities[i]->pre_render_operation) {
                // case EntityPreRenderOperation::generate_sphere:
                //     /* Call the generate sphere CPU function. */
                //     break;

                case EntityPreRenderOperation::epro_generate_triangle:
                    /* Call the generate triangle CPU function. */
                    cpu_pre_render_triangle(vertices, (Triangle*) entities[i]);
                    break;

                default:
                    DLOG(fatal, "Entity " + std::to_string(i) + " wants to be pre-rendered on the GPU using unsupported operation '" + entity_pre_render_operation_names[entities[i]->pre_render_operation] + "'.");

            }

        } else {
            DLOG(fatal, "Entity " + std::to_string(i) + " of type " + entity_type_names[entities[i]->type] + " with the Vulkan compute shader back-end.");
        }

        // Regardless of how we pre-rendered, we can now condense the resulting arrays in indexed equivalents
        for (size_t i = 0; i < vertices.size(); i++) {
            // For each of the points, see if there are existing entries for them
            uint32_t indices[] = { numeric_limits<uint32_t>::max(), numeric_limits<uint32_t>::max(), numeric_limits<uint32_t>::max() };
            for (size_t lp = 0; lp < 3; lp++) {
                // Get a vec4-equivalent of the correct point
                glm::vec4 point = glm::vec4(vertices[i].points[lp], 0.0);

                // Search the point list to see if it indeed exists
                for (size_t gp= 0; gp < this->entity_points.size(); gp++) {
                    if (this->entity_points[gp] == point) {
                        // It does; store this index
                        indices[lp] = static_cast<uint32_t>(gp);
                        break;
                    }
                }

                // If it does not, append it instead
                if (indices[lp] == numeric_limits<uint32_t>::max()) {
                    indices[lp] = this->entity_points.size();
                    this->entity_points.push_back(point);
                }
            }

            // Now, we have enough to create a new GVertex
            this->entity_vertices.push_back({
                indices[0], indices[1], indices[2],
                glm::vec4(vertices[i].normal, 0.0),
                glm::vec4(vertices[i].color, 0.0)
            });
        }
    }

    // We're done! We pre-rendered all objects!
    DRETURN;
}

/* Renders the internal list of vertices to a frame using the given camera position. */
void VulkanRenderer::render(Camera& cam) const {
    DENTER("VulkanRenderer::render");

    /* Step 1: Buffer initialization. */
    // We begin by allocating the buffers required for the main data
    size_t vertices_size = this->entity_vertices.size() * sizeof(GVertex);
    size_t points_size = this->entity_points.size() * sizeof(glm::vec4);
    BufferHandle vertices_h = this->device_memory_pool->allocate(vertices_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    BufferHandle points_h = this->device_memory_pool->allocate(points_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    Buffer vertices = (*this->device_memory_pool)[vertices_h];
    Buffer points = (*this->device_memory_pool)[points_h];

    // Next, get two staging buffers so we can (hopefully) do things concurrently
    BufferHandle vertices_staging_h = this->stage_memory_pool->allocate(vertices_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    BufferHandle points_staging_h = this->stage_memory_pool->allocate(points_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    Buffer vertices_staging = (*this->stage_memory_pool)[vertices_staging_h];
    Buffer points_staging = (*this->stage_memory_pool)[points_staging_h];

    // Next, fill the vertex staging buffer
    void* mapped_area;
    vertices_staging.map(*this->gpu, &mapped_area);
    memcpy(mapped_area, (void*) this->entity_vertices.rdata(), vertices_size);
    vertices_staging.flush(*this->gpu);
    vertices_staging.unmap(*this->gpu);

    // Then do the points
    points_staging.map(*this->gpu, &mapped_area);
    memcpy(mapped_area, (void*) this->entity_points.rdata(), points_size);
    points_staging.flush(*this->gpu);
    points_staging.unmap(*this->gpu);

    // Next, we schedule the copies of the staging buffers to the real buffers
    CommandBuffer staging_cb = (*this->memory_command_pool)[this->staging_cb_h];
    vertices_staging.copyto(this->gpu->memory_queue(), staging_cb, vertices, true);
    points_staging.copyto(this->gpu->memory_queue(), staging_cb, points, true);

    // Once that's done, deallocate the staging buffers
    this->stage_memory_pool->deallocate(vertices_staging_h);
    this->stage_memory_pool->deallocate(points_staging_h);



    /* Step 2: Camera buffer initialization. */
    // First, allocate buffers for the frame and the camera data
    uint32_t width = cam.w(), height = cam.h();
    size_t camera_size = sizeof(GCameraData);
    size_t frame_size = width * height * sizeof(glm::vec4);
    BufferHandle camera_h = this->device_memory_pool->allocate(camera_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    BufferHandle frame_h = this->device_memory_pool->allocate(camera_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    Buffer camera = (*this->device_memory_pool)[camera_h];
    Buffer frame = (*this->device_memory_pool)[frame_h];

    // Next, get a staging buffer for the camera only
    BufferHandle camera_staging_h = this->stage_memory_pool->allocate(camera_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    Buffer camera_staging = (*this->stage_memory_pool)[camera_staging_h];

    // Map the staging buffer to host-reachable memory
    void* camera_staging_map;
    camera_staging.map(*this->gpu, &camera_staging_map);

    // Populate the data. For the camera this is rather manual
    ((GCameraData*) camera_staging_map)->origin = cam.origin;
    ((GCameraData*) camera_staging_map)->horizontal = cam.horizontal;
    ((GCameraData*) camera_staging_map)->vertical = cam.vertical;
    ((GCameraData*) camera_staging_map)->lower_left_corner = cam.lower_left_corner;

    // Flush & unmap the staging buffer
    camera_staging.flush(*this->gpu);
    camera_staging.unmap(*this->gpu);

    // Next, we schedule the copies of the staging buffer to the real buffer
    camera_staging.copyto(this->gpu->memory_queue(), staging_cb, camera);

    // Once that's done, deallocate the staging buffer for the camera.
    this->stage_memory_pool->deallocate(camera_staging_h);



    /* Step 3: Descriptor set initialization. */
    // Allocate a DescriptorSet & set all bindings
    DescriptorSet descriptor_set = this->descriptor_pool->allocate(*this->raytrace_dsl);
    descriptor_set.set(*this->gpu, 0, Tools::Array<Buffer>({ frame }));
    descriptor_set.set(*this->gpu, 1, Tools::Array<Buffer>({ camera }));
    descriptor_set.set(*this->gpu, 2, Tools::Array<Buffer>({ vertices }));
    descriptor_set.set(*this->gpu, 3, Tools::Array<Buffer>({ points }));



    /* Step 4: Render the frame. */
    // Initialize a new pipeline using the new layout
    Pipeline pipeline(
        *this->gpu,
        Shader(*this->gpu, Tools::get_executable_path() + "/shaders/raytracer_v1.spv"),
        Tools::Array<DescriptorSetLayout>({ *this->raytrace_dsl }),
        std::unordered_map<uint32_t, std::tuple<uint32_t, void*>>({ { 0, std::make_tuple(sizeof(uint32_t), (void*) &width) }, { 1, std::make_tuple(sizeof(uint32_t), (void*) &height) } })
    );

    // Next, start recording the compute command buffer
    CommandBufferHandle cb_compute_h = this->compute_command_pool->allocate();
    CommandBuffer cb_compute = (*this->compute_command_pool)[cb_compute_h];
    cb_compute.begin();
    pipeline.bind(cb_compute);
    descriptor_set.bind(cb_compute, pipeline.layout());
    vkCmdDispatch(cb_compute, (width / 32) + 1, (height / 32) + 1, 1);
    cb_compute.end();

    // With the buffer recorded, submit it to the given queue
    VkResult vk_result;
    VkSubmitInfo submit_info = cb_compute.get_submit_info();
    if ((vk_result = vkQueueSubmit(this->gpu->compute_queue(), 1, &submit_info, VK_NULL_HANDLE)) != VK_SUCCESS) {
        DLOG(fatal, "Could not submit command buffer to queue: " + vk_error_map[vk_result]);
    }

    // Finally, wait until the rendering is done
    if ((vk_result = vkQueueWaitIdle(this->gpu->compute_queue())) != VK_SUCCESS) {
        DLOG(fatal, "Could not wait for queue to become idle: " + vk_error_map[vk_result]);
    }



    /* Step 4: Frame retrieval. */
    // Get a staging buffer for the frame
    BufferHandle frame_staging_h = this->stage_memory_pool->allocate(frame_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    Buffer frame_staging = (*this->stage_memory_pool)[frame_staging_h];

    // Copy the contents of the frame buffer back to the staging buffer
    frame.copyto(this->gpu->memory_queue(), staging_cb, frame_staging);

    // Next, map the staging buffer's memory to host-accessible memory and copy the frame data back to the CPU
    void* frame_staging_map;
    frame_staging.map(*this->gpu, &frame_staging_map);
    memcpy((void*) cam.get_frame().d(), frame_staging_map, frame_size);
    frame_staging.flush(*this->gpu);
    frame_staging.unmap(*this->gpu);

    // Once we're done, we can clean the frame staging buffer
    this->stage_memory_pool->deallocate(frame_staging_h);
    


    /* Step 5: Cleanup. */
    // Cleanup the command buffer
    this->compute_command_pool->deallocate(cb_compute_h);

    // Cleanup the descriptor set
    this->descriptor_pool->deallocate(descriptor_set);

    // Cleanup the GPU buffers
    this->device_memory_pool->deallocate(frame_h);
    this->device_memory_pool->deallocate(camera_h);
    this->device_memory_pool->deallocate(points_h);
    this->device_memory_pool->deallocate(vertices_h);

    // Done!
    DRETURN;
}



/* Swap operator for the VulkanRenderer class. */
void RayTracer::swap(VulkanRenderer& r1, VulkanRenderer& r2) {
    using std::swap;

    swap(r1.instance, r2.instance);
    swap(r1.gpu, r2.gpu);

    swap(r1.device_memory_pool, r2.device_memory_pool);
    swap(r1.stage_memory_pool, r2.stage_memory_pool);
    swap(r1.descriptor_pool, r2.descriptor_pool);
    swap(r1.compute_command_pool, r2.compute_command_pool);
    swap(r1.memory_command_pool, r2.memory_command_pool);

    swap(r1.raytrace_dsl, r2.raytrace_dsl);
    swap(r1.staging_cb_h, r2.staging_cb_h);

    swap(r1.entity_vertices, r2.entity_vertices);
    swap(r1.entity_points, r2.entity_points);

}





/***** FACTORY METHOD *****/
Renderer* RayTracer::initialize_renderer() {
    DENTER("initialize_renderer");

    // Initialize a new object
    VulkanRenderer* result = new VulkanRenderer();

    // Done, return
    DRETURN (Renderer*) result;
}
