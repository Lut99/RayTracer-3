/* VULKAN RENDERER.cpp
 *   by Lut99
 *
 * Created:
 *   30/04/2021, 13:34:23
 * Last edited:
 *   21/05/2021, 15:29:59
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Implementation of the Renderer class that uses Vulkan compute shaders.
**/

// We define that vulkan is enabled
#ifndef ENABLE_VULKAN
#define ENABLE_VULKAN
#endif

#include <functional>
#include <CppDebugger.hpp>

#include "compute/Pipeline.hpp"
#include "compute/ErrorCodes.hpp"

#include "entities/Triangle.hpp"
#include "entities/Sphere.hpp"
#include "entities/Object.hpp"

#include "tools/Common.hpp"

#include "VulkanRenderer.hpp"

using namespace std;
using namespace RayTracer;
using namespace RayTracer::Compute;
using namespace RayTracer::ECS;
using namespace CppDebugger::SeverityValues;


/***** STRUCTS *****/
/* Struct used to carry sum constants to the gpu. */
struct ConstantData {
    alignas(4) uint32_t i;
    alignas(4) float duplicate_constant;  
};

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
    Renderer(),
    vk_entity_faces(MemoryPool::NullHandle),
    vk_entity_vertices(MemoryPool::NullHandle)
{
    DENTER("VulkanRenderer::VulkanRenderer");
    DLOG(info, "Initializing the Vulkan-based renderer...");
    DINDENT;

    // First, create a new instance
    this->instance = new Instance();

    //  Next, initialize the GPU
    this->gpu = new GPU(*this->instance);

    // Before we continue, select suitable memory types for each pool.
    uint32_t device_memory_type = MemoryPool::select_memory_type(*this->gpu, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    uint32_t stage_memory_type = MemoryPool::select_memory_type(*this->gpu, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    // Next, initialize all  pools
    this->device_memory_pool = new MemoryPool(*this->gpu, device_memory_type, VulkanRenderer::device_memory_size, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    this->stage_memory_pool = new MemoryPool(*this->gpu, stage_memory_type, VulkanRenderer::stage_memory_size, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    this->descriptor_pool = new DescriptorPool(
        *this->gpu,
        Tools::Array<std::tuple<VkDescriptorType, uint32_t>>({
            std::make_tuple(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1),
            std::make_tuple(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 4)
        }),
        VulkanRenderer::max_descriptor_sets
    );

    this->compute_command_pool = new CommandPool(*this->gpu, this->gpu->queue_info().compute());
    this->memory_command_pool = new CommandPool(*this->gpu, this->gpu->queue_info().memory(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    // Initialize the descriptor set layout for the insert call
    this->insert_dsl = new DescriptorSetLayout(*this->gpu);
    this->insert_dsl->add_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT);
    this->insert_dsl->add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT);
    this->insert_dsl->add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT);
    this->insert_dsl->finalize();

    // Initialize the descriptor set layout for the raytrace call
    this->raytrace_dsl = new DescriptorSetLayout(*this->gpu);
    this->raytrace_dsl->add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT);
    this->raytrace_dsl->add_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT);
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
    vk_entity_faces(other.vk_entity_faces),
    vk_entity_vertices(other.vk_entity_vertices)
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
    this->insert_dsl = new DescriptorSetLayout(*other.insert_dsl);
    this->raytrace_dsl = new DescriptorSetLayout(*other.raytrace_dsl);

    // And copy command buffers
    this->staging_cb_h = this->memory_command_pool->allocate();

    // Copy the entity buffers, if any
    if (other.vk_entity_faces != MemoryPool::NullHandle) {
        this->vk_entity_faces = this->device_memory_pool->allocate_buffer_h(other.device_memory_pool->deref_buffer(other.vk_entity_faces));
    }
    if (other.vk_entity_vertices != MemoryPool::NullHandle) {
        this->vk_entity_vertices = this->device_memory_pool->allocate_buffer_h(other.device_memory_pool->deref_buffer(other.vk_entity_vertices));
    }

    // Done, return
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
    insert_dsl(other.insert_dsl),
    raytrace_dsl(other.raytrace_dsl),
    staging_cb_h(other.staging_cb_h),
    vk_entity_faces(other.vk_entity_faces),
    vk_entity_vertices(other.vk_entity_vertices)
{
    // Set the other's deallocateable pointers to nullptrs to avoid just that
    other.instance = nullptr;
    other.gpu = nullptr;
    other.device_memory_pool = nullptr;
    other.stage_memory_pool = nullptr;
    other.descriptor_pool = nullptr;
    other.compute_command_pool = nullptr;
    other.memory_command_pool = nullptr;
    other.insert_dsl = nullptr;
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
    if (this->insert_dsl != nullptr) {
        delete this->insert_dsl;
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



/* Helper function that takes a GPU-allocated faces & vertex buffer and inserts the data from the CPU-side faces & vertex at the given offsets. */
void VulkanRenderer::transfer_entity(const Compute::Buffer& vk_faces_buffer, uint32_t vk_faces_offset, const Compute::Buffer& vk_vertex_buffer, uint32_t vk_vertex_offset, const Tools::Array<GFace>& faces_buffer, const Tools::Array<glm::vec4>& vertex_buffer, Compute::Suite& suite) {
    DENTER("VulkanRenderer::transfer_entity");

    // First, allocate a staging buffer large enough to transfer both the faces and the vertices
    uint32_t faces_size = faces_buffer.size() * sizeof(GFace);
    uint32_t vertex_size = vertex_buffer.size() * sizeof(glm::vec4);
    uint32_t staging_size = std::max({ faces_size, vertex_size });
    Buffer staging = suite.stage_memory_pool.allocate_buffer(staging_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    // Next, we copy the faces
    {
        // First, map the memory
        GFace* mapped_memory;
        staging.map(suite.gpu, (void**) &mapped_memory);

        // Then, copy the CPU buffer over - all the while while we add the vertex offset to the indices
        for (size_t i = 0; i < faces_buffer.size(); i++) {
            mapped_memory[i] = faces_buffer[i];
            mapped_memory[i].v1 += vk_vertex_offset;
            mapped_memory[i].v2 += vk_vertex_offset;
            mapped_memory[i].v3 += vk_vertex_offset;
        }

        // Unmap and then flush the staging area
        staging.unmap(suite.gpu);
        staging.flush(suite.gpu);

        // Copy the data over, but make sure to only copy to the given offset in the target buffer
        staging.copyto(suite.staging_cb, suite.gpu.memory_queue(), vk_faces_buffer, (VkDeviceSize) faces_size, (VkDeviceSize) vk_faces_offset);
    }

    // Then, we copy the vertices
    {
        // First, map the memory
        void* mapped_memory;
        staging.map(suite.gpu, &mapped_memory);

        // Then, copy the CPU buffer over
        memcpy(mapped_memory, vertex_buffer.rdata(), vertex_size);

        // Unmap and then flush the staging area
        staging.unmap(suite.gpu);
        staging.flush(suite.gpu);

        // Copy the data over, but make sure to only copy to the given offset in the target buffer
        staging.copyto(suite.staging_cb, suite.gpu.memory_queue(), vk_vertex_buffer, (VkDeviceSize) vertex_size, (VkDeviceSize) vk_vertex_offset);
    }

    // Deallocate the staging buffer and we're done
    suite.stage_memory_pool.deallocate(staging);
    DRETURN;
}


        
/* Pre-renders the given list of RenderEntities, accelerated using Vulkan compute shaders. */
void VulkanRenderer::prerender(const Tools::Array<ECS::RenderEntity*>& entities) {
    DENTER("VulkanRenderer::prerender");
    DLOG(info, "Pre-rendering entities...");
    DINDENT;

    // First, if any, deallocate the old buffers
    if (this->vk_entity_faces != MemoryPool::NullHandle) {
        this->device_memory_pool->deallocate(this->vk_entity_faces);
    }
    if (this->vk_entity_vertices != MemoryPool::NullHandle) {
        this->device_memory_pool->deallocate(this->vk_entity_vertices);
    }

    // Next, loop through all the entities to find out the total number of faces & vertices we'll get
    uint32_t n_faces = 0;
    uint32_t n_vertices = 0;
    for (size_t i = 0; i < entities.size(); i++) {
        n_faces += entities[i]->pre_render_faces;
        n_vertices += entities[i]->pre_render_vertices;
    }
    DLOG(info, "Total: " + std::to_string(entities.size()) + " entities, with " + std::to_string(n_faces) + " faces (" + std::to_string(n_faces * sizeof(GFace)) + " bytes) and " + std::to_string(n_vertices) + " vertices (" + std::to_string(n_vertices * sizeof(glm::vec4)) + " bytes)");

    // With the size, initialize the two output buffers
    this->vk_entity_faces = this->device_memory_pool->allocate_buffer_h(n_faces * sizeof(GFace), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    Buffer vk_entity_faces = this->device_memory_pool->deref_buffer(this->vk_entity_faces);
    this->vk_entity_vertices = this->device_memory_pool->allocate_buffer_h(n_vertices * sizeof(glm::vec4), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    Buffer vk_entity_vertices = this->device_memory_pool->deref_buffer(this->vk_entity_vertices);

    // Prepare a suite of the GPU-related structures to pass to GPU-enabled functions as necessary
    Compute::Suite suite = this->get_suite();

    // Prepare two temporary Array structures to pre-render on the CPU in
    Tools::Array<GFace> entity_faces;
    Tools::Array<glm::vec4> entity_vertices;

    // Next, loop through all entities to render them
    uint32_t faces_offset = 0, vertex_offset = 0;
    for (size_t i = 0; i < entities.size(); i++) {
        // Select the proper pre-render mode
        if (entities[i]->pre_render_mode & EntityPreRenderModeFlags::eprmf_gpu) {
            // Determine the type of pre-rendering operation we need to do
            switch (entities[i]->pre_render_operation) {
                case EntityPreRenderOperation::epro_generate_sphere:
                    /* Prepare the pipeline using this shader. */
                    gpu_pre_render_sphere(
                        vk_entity_faces,
                        faces_offset,
                        vk_entity_vertices,
                        vertex_offset,
                        suite,
                        (Sphere*) entities[i]
                    );
                    break;

                default:
                    DLOG(fatal, "Entity " + std::to_string(i) + " wants to be pre-rendered on the GPU using unsupported operation '" + entity_pre_render_operation_names[entities[i]->pre_render_operation] + "'.");

            }

            // Increment the offset, and we're done!
            faces_offset += entities[i]->pre_render_faces;
            vertex_offset += entities[i]->pre_render_vertices;

            // // For debugging purposes, fetch the faces and compare them with the GPU-generated counterpart
            // GFace faces[n_faces];
            // Buffer staging = suite.stage_memory_pool.allocate_buffer(n_faces * sizeof(GFace), VK_BUFFER_USAGE_TRANSFER_DST_BIT);
            // vk_entity_faces.get(suite.gpu, staging, suite.staging_cb, suite.gpu.memory_queue(), faces, n_faces * sizeof(GFace));
            // suite.stage_memory_pool.deallocate(staging);

            // entity_faces.clear();
            // entity_vertices.clear();
            // entity_faces.resize(entities[i]->pre_render_faces);
            // entity_vertices.resize(entities[i]->pre_render_vertices);
            // cpu_pre_render_sphere(entity_faces, entity_vertices, (Sphere*) entities[i]);

            // // Compare
            // for (size_t i = 0; i < n_faces; i++) {
            //     if (faces[i].v1 != entity_faces[i].v1 ||
            //         faces[i].v2 != entity_faces[i].v2 ||
            //         faces[i].v3 != entity_faces[i].v3)
            //     {
            //         DLOG(warning, "GPU-rendered face's normal differs from CPU-rendered face's normal: {" + std::to_string(faces[i].normal.x) + "," + std::to_string(faces[i].normal.y) + "," + std::to_string(faces[i].normal.z) + "} VS {" + std::to_string(entity_faces[i].normal.x) + "," + std::to_string(entity_faces[i].normal.y) + "," + std::to_string(entity_faces[i].normal.z) + "}")
            //     }
            //     glm::vec3 dn = faces[i].normal - entity_faces[i].normal;
            //     if (dn.x > 1e-6 || dn.y > 1e-6 || dn.z > 1e-6) {
            //         DLOG(warning, "GPU-rendered face's normal differs from CPU-rendered face's normal: {" + std::to_string(faces[i].normal.x) + "," + std::to_string(faces[i].normal.y) + "," + std::to_string(faces[i].normal.z) + "} VS {" + std::to_string(entity_faces[i].normal.x) + "," + std::to_string(entity_faces[i].normal.y) + "," + std::to_string(entity_faces[i].normal.z) + "}")
            //     }
            // }

        } else if (entities[i]->pre_render_mode & EntityPreRenderModeFlags::eprmf_cpu) {
            // Clear the buffers and set them to the correct size
            entity_faces.clear();
            entity_vertices.clear();
            entity_faces.resize(entities[i]->pre_render_faces);
            entity_vertices.resize(entities[i]->pre_render_vertices);

            // Determine the type of pre-rendering operation we need to do
            switch (entities[i]->pre_render_operation) {
                case EntityPreRenderOperation::epro_generate_triangle:
                    /* Call the generate triangle CPU function. */
                    cpu_pre_render_triangle(entity_faces, entity_vertices, (Triangle*) entities[i]);
                    break;

                case EntityPreRenderOperation::epro_generate_sphere:
                    /* Call the generate sphere CPU function. */
                    cpu_pre_render_sphere(entity_faces, entity_vertices, (Sphere*) entities[i]);
                    break;
                
                case EntityPreRenderOperation::epro_load_object_file:
                    /* Call the load object file CPU function. */
                    cpu_pre_render_object(entity_faces, entity_vertices, (Object*) entities[i]);
                    break;

                default:
                    DLOG(fatal, "Entity " + std::to_string(i) + " wants to be pre-rendered on the CPU using unsupported operation '" + entity_pre_render_operation_names[entities[i]->pre_render_operation] + "'.");

            }

            // Once done, use the internal function to copy them to the CPU
            this->transfer_entity(vk_entity_faces, faces_offset, vk_entity_vertices, vertex_offset, entity_faces, entity_vertices, suite);
            faces_offset += entities[i]->pre_render_faces;
            vertex_offset += entities[i]->pre_render_vertices;

        } else {
            DLOG(fatal, "Entity " + std::to_string(i) + " of type " + entity_type_names[entities[i]->type] + " with the Vulkan compute shader back-end.");
        }

        // Inserting is now handled by either the function itself or the CPU-specific function, so we're done!
    }

    // We're done! We pre-rendered all objects!
    DDEDENT;
    DRETURN;
}

/* Renders the internal list of vertices to a frame using the given camera position. */
void VulkanRenderer::render(Camera& cam) const {
    DENTER("VulkanRenderer::render");

    // Print some info
    DLOG(info, "Rendering for camera:");
    DINDENT;
    DLOG(auxillary, "Camera origin            : (" + std::to_string(cam.origin.x) + "," + std::to_string(cam.origin.y) + "," + std::to_string(cam.origin.z) + ")");
    DLOG(auxillary, "Camera horizontal        : (" + std::to_string(cam.horizontal.x) + "," + std::to_string(cam.horizontal.y) + "," + std::to_string(cam.horizontal.z) + ")");
    DLOG(auxillary, "Camera vertical          : (" + std::to_string(cam.vertical.x) + "," + std::to_string(cam.vertical.y) + "," + std::to_string(cam.vertical.z) + ")");
    DLOG(auxillary, "Camera lower_left_corner : (" + std::to_string(cam.lower_left_corner.x) + "," + std::to_string(cam.lower_left_corner.y) + "," + std::to_string(cam.lower_left_corner.z) + ")");
    DDEDENT;
    
    /* Step 1: Camera buffer initialization. */
    DLOG(info, "Transferring camera to GPU...");
    
    // First, allocate buffers for the frame and the camera data
    uint32_t width = cam.w(), height = cam.h();
    size_t camera_size = sizeof(GCameraData);
    size_t frame_size = width * height * sizeof(glm::vec4);
    Buffer camera = this->device_memory_pool->allocate_buffer(camera_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    Buffer frame = this->device_memory_pool->allocate_buffer(frame_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    // Next, get a staging buffer for the camera only
    Buffer camera_staging = this->stage_memory_pool->allocate_buffer(camera_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

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
    CommandBuffer staging_cb = (*this->memory_command_pool)[this->staging_cb_h];
    camera_staging.copyto(staging_cb, this->gpu->memory_queue(), camera);

    // Once that's done, deallocate the staging buffer for the camera.
    this->stage_memory_pool->deallocate(camera_staging);



    /* Step 3: Descriptor set initialization. */
    DLOG(info, "Creating descriptor set...");

    // Fetch the internal handles as buffers
    Buffer vk_entity_faces = this->device_memory_pool->deref_buffer(this->vk_entity_faces);
    Buffer vk_entity_vertices = this->device_memory_pool->deref_buffer(this->vk_entity_vertices);

    // Allocate a DescriptorSet & set all bindings
    DescriptorSet descriptor_set = this->descriptor_pool->allocate(*this->raytrace_dsl);
    descriptor_set.set(*this->gpu, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0, Tools::Array<Buffer>({ frame }));
    descriptor_set.set(*this->gpu, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, Tools::Array<Buffer>({ camera }));
    descriptor_set.set(*this->gpu, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2, Tools::Array<Buffer>({ vk_entity_faces }));
    descriptor_set.set(*this->gpu, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3, Tools::Array<Buffer>({ vk_entity_vertices }));



    /* Step 4: Render the frame. */
    DLOG(info, "Preparing pipeline...");
    
    // Initialize a new pipeline using the new layout
    DINDENT;
    Pipeline pipeline(
        *this->gpu,
        Shader(*this->gpu, Tools::get_executable_path() + "/shaders/raytracer_v3.spv"),
        Tools::Array<DescriptorSetLayout>({ *this->raytrace_dsl }),
        std::unordered_map<uint32_t, std::tuple<uint32_t, void*>>({ { 0, std::make_tuple(sizeof(uint32_t), (void*) &width) }, { 1, std::make_tuple(sizeof(uint32_t), (void*) &height) } })
    );
    DDEDENT;
    
    // Next, start recording the compute command buffer
    DLOG(info, "Recording command buffer...");
    CommandBuffer cb_compute = this->compute_command_pool->allocate();
    cb_compute.begin();
    pipeline.bind(cb_compute);
    descriptor_set.bind(cb_compute, pipeline.layout());
    vkCmdDispatch(cb_compute, (width / 32) + 1, (height / 32) + 1, 1);
    cb_compute.end();

    // With the buffer recorded, submit it to the given queue
    DLOG(info, "Rendering...");
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
    DLOG(info, "Retrieving frame...");
    
    // Get a staging buffer for the frame
    Buffer frame_staging = this->stage_memory_pool->allocate_buffer(frame_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    // Copy the contents of the frame buffer back to the CPU
    frame.copyto(staging_cb, this->gpu->memory_queue(), frame_staging);

    // Next, map the staging buffer's memory to host-accessible memory
    void* frame_staging_map;
    frame_staging.map(*this->gpu, &frame_staging_map);

    // Copy the vectors manually, to convert from vec4 to vec3
    for (uint32_t p = 0; p < width * height; p++) {
        cam.get_frame().d()[p] = ((glm::vec4*) frame_staging_map)[p];
    }
   
    // When done, flush and unmap
    frame_staging.flush(*this->gpu);
    frame_staging.unmap(*this->gpu);

    // Once we're done, we can clean the frame staging buffer
    this->stage_memory_pool->deallocate(frame_staging);
    


    /* Step 5: Cleanup. */
    DLOG(info, "Finishing up...");

    // Cleanup the command buffer
    this->compute_command_pool->deallocate(cb_compute);

    // Cleanup the descriptor set
    this->descriptor_pool->deallocate(descriptor_set);

    // Cleanup the GPU buffers
    this->device_memory_pool->deallocate(frame);
    this->device_memory_pool->deallocate(camera);

    // Done!
    DRETURN;
}



/* Swap operator for the VulkanRenderer class. */
void RayTracer::swap(VulkanRenderer& r1, VulkanRenderer& r2) {
    using std::swap;

    // Swap as renderer first
    swap((Renderer&) r1, (Renderer&) r2);

    swap(r1.instance, r2.instance);
    swap(r1.gpu, r2.gpu);

    swap(r1.device_memory_pool, r2.device_memory_pool);
    swap(r1.stage_memory_pool, r2.stage_memory_pool);
    swap(r1.descriptor_pool, r2.descriptor_pool);
    swap(r1.compute_command_pool, r2.compute_command_pool);
    swap(r1.memory_command_pool, r2.memory_command_pool);

    swap(r1.insert_dsl, r2.insert_dsl);
    swap(r1.raytrace_dsl, r2.raytrace_dsl);
    swap(r1.staging_cb_h, r2.staging_cb_h);

    swap(r1.vk_entity_faces, r2.vk_entity_faces);
    swap(r1.vk_entity_vertices, r2.vk_entity_vertices);

    // Done
}





/***** FACTORY METHOD *****/
Renderer* RayTracer::initialize_renderer() {
    DENTER("initialize_renderer");

    // Initialize a new object
    VulkanRenderer* result = new VulkanRenderer();

    // Done, return
    DRETURN (Renderer*) result;
}
