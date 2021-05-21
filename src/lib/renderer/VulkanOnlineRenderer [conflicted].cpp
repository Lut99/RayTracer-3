/* VULKAN ONLINE RENDERER.cpp
 *   by Lut99
 *
 * Created:
 *   09/05/2021, 18:30:34
 * Last edited:
 *   21/05/2021, 17:04:21
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Derived class of the VulkanRenderer class, which renders to a
 *   swapchain in real-time instead of to images. Therefore, the call to
 *   this render is blocking, returning an empty frame once the user closes
 *   the window.
**/

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <CppDebugger.hpp>

#include "compute/Pipeline.hpp"
#include "compute/Swapchain.hpp"
#include "compute/ErrorCodes.hpp"
#include "tools/Common.hpp"

#include "VulkanOnlineRenderer.hpp"

using namespace std;
using namespace RayTracer;
using namespace RayTracer::Compute;
using namespace CppDebugger::SeverityValues;


/***** POPULATE FUNCTIONS *****/
/* Populates a given VkPresentInfoKHR struct. */
static void populate_present_info(VkPresentInfoKHR& present_info, const Swapchain& swapchain, uint32_t image_index) {
    DENTER("populate_present_info");

    // First, set the default stuff
    present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    present_info.

    DRETURN;
}

/* Populates a given VkSemaphoreCreateInfo struct. */
static void populate_semaphore_info(VkSemaphoreCreateInfo& semaphore_info) {
    DENTER("populate_semaphore_info");

    // First, set the default stuff
    semaphore_info = {};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    // And that's it already!
    DRETURN;
}

/* Populates a given VkFenceCreateInfo struct. */
static void populate_fence_info(VkFenceCreateInfo& fence_info) {
    DENTER("populate_fence_info");

    // First, set the default stuff
    fence_info = {};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    // And that's it already!
    DRETURN;
}





/***** VULKANONLINERENDERER CLASS *****/
/* Constructor for the VulkanOnlineRenderer class, which takes nothing to be compatible. Note that it does not rely on the parent constructor, since we want to start the vulkan instance & GPU differently. */
VulkanOnlineRenderer::VulkanOnlineRenderer() :
    VulkanRenderer(false)
{
    DENTER("VulkanOnlineRenderer::VulkanOnlineRenderer");
    DLOG(info, "Initializing Vulkan-based online renderer...");
    DINDENT;
    
    // First, prepare the GLFW library. Note that this is global, and is thus tricky to use in multiple ways I guess
    DLOG(info, "Initializing GLFW...");
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);



    // Next, setup the Vulkan stuff
    // We first collect a list of GLFW extensions
    uint32_t n_extensions = 0;
    const char** raw_extensions = glfwGetRequiredInstanceExtensions(&n_extensions);

    // Use them to populate the instance
    this->instance = new Instance(instance_extensions + Tools::Array<const char*>(raw_extensions, n_extensions));

    // Next, create the GPU in swapchain mode
    this->gpu = new GPU(*this->instance, device_extensions + Tools::Array<const char*>({ VK_KHR_SWAPCHAIN_EXTENSION_NAME }));

    // Before we continue, select suitable memory types for each pool.
    uint32_t device_memory_type = MemoryPool::select_memory_type(*this->gpu, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    uint32_t stage_memory_type = MemoryPool::select_memory_type(*this->gpu, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    // Next, initialize all  pools
    this->device_memory_pool = new MemoryPool(*this->gpu, device_memory_type, VulkanOnlineRenderer::device_memory_size, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    this->stage_memory_pool = new MemoryPool(*this->gpu, stage_memory_type, VulkanOnlineRenderer::stage_memory_size, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    this->descriptor_pool = new DescriptorPool(
        *this->gpu,
        Tools::Array<std::tuple<VkDescriptorType, uint32_t>>({
            std::make_tuple(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1),
            std::make_tuple(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 4)
        }),
        VulkanOnlineRenderer::max_descriptor_sets
    );

    this->compute_command_pool = new CommandPool(*this->gpu, this->gpu->queue_info().compute());
    this->memory_command_pool = new CommandPool(*this->gpu, this->gpu->queue_info().memory(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    // Initialize the descriptor set layout for the raytrace call
    this->raytrace_dsl = new DescriptorSetLayout(*this->gpu);
    this->raytrace_dsl->add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT);
    this->raytrace_dsl->add_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT);
    this->raytrace_dsl->add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT);
    this->raytrace_dsl->add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT);
    this->raytrace_dsl->finalize();

    // Also allocate the command buffer(s)
    this->staging_cb_h = this->memory_command_pool->allocate_h();



    DDEDENT;
    DLEAVE;
}

/* Copy constructor for the VulkanOnlineRenderer class. */
VulkanOnlineRenderer::VulkanOnlineRenderer(const VulkanOnlineRenderer& other) :
    VulkanRenderer(other)
{}

/* Move constructor for the VulkanOnlineRenderer class. */
VulkanOnlineRenderer::VulkanOnlineRenderer(VulkanOnlineRenderer&& other) :
    VulkanRenderer(std::move(other))
{}

/* Destructor for the VulkanOnlineRenderer class. */
VulkanOnlineRenderer::~VulkanOnlineRenderer() {
    DENTER("VulkanOnlineRenderer::~VulkanOnlineRenderer");
    DLOG(info, "Cleaning online renderer stuff...");
    DINDENT;
    
    DLOG(info, "Terminating GLFW library...");
    glfwTerminate();

    DDEDENT;
    DLEAVE;
}



/* Renders the internal list of vertices to a window using the given camera position. Renders the entire simulation, including update steps, and returns a blackened frame since the output is unreachable and simultaneously not interesting. */
void VulkanOnlineRenderer::render(Camera& cam) const {
    DENTER("VulkanOnlineRenderer::render");
    uint32_t width = cam.w(), height = cam.h();

    /* Step 1: Initialize the window we'll be rendering to and get a surface. */
    DLOG(info, "Creating GLFW window...");
    GLFWwindow* glfw_window = glfwCreateWindow(width, height, "RayTracer-3", NULL, NULL);
    VkSurfaceKHR glfw_surface;
    VkResult vk_result;
    if ((vk_result = glfwCreateWindowSurface(*this->instance, glfw_window, nullptr, &glfw_surface)) != VK_SUCCESS) {
        DLOG(fatal, "Could not get GLFW window surface: " + vk_error_map[vk_result]);
    }

    // Also update the GPU info for its presentation queue support
    this->gpu->check_present(glfw_surface);



    /* Step 2: Prepare the initial swapchain. */
    Swapchain* swapchain = new Swapchain(*this->gpu, glfw_window, glfw_surface);



    /* Step 3: Allocate buffers & prepare the pipeline. */
    DLOG(info, "Preparing memory structures...");

    // First, allocate the camera buffer and a matching staging buffer
    size_t camera_size = sizeof(GCameraData);
    Buffer camera = this->device_memory_pool->allocate_buffer(camera_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    Buffer staging = this->stage_memory_pool->allocate_buffer(camera_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    // Next, prepare the output frame buffer
    size_t frame_size = width * height * sizeof(glm::vec4);
    Buffer frame = this->device_memory_pool->allocate_buffer(frame_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    // Fetch the internal faces & vertex handles as buffers
    Buffer vk_entity_faces = this->device_memory_pool->deref_buffer(this->vk_entity_faces);
    Buffer vk_entity_vertices = this->device_memory_pool->deref_buffer(this->vk_entity_vertices);

    // Fetch the internal command buffer handle
    CommandBuffer staging_cb = (*this->memory_command_pool)[this->staging_cb_h];

    // Create the pipeline
    Pipeline pipeline(
        *this->gpu,
        Shader(*this->gpu, Tools::get_executable_path() + "/shaders/raytracer_v3.spv"),
        Tools::Array<DescriptorSetLayout>({ *this->raytrace_dsl }),
        std::unordered_map<uint32_t, std::tuple<uint32_t, void*>>({ { 0, std::make_tuple(sizeof(uint32_t), (void*) &width) }, { 1, std::make_tuple(sizeof(uint32_t), (void*) &height) } })
    );

    

    /* Step 4: Synchronization structres. */
    DLOG(info, "Preparing synchronization structures...");
    Tools::Array<VkSemaphore> image_ready_semaphores;
    Tools::Array<VkSemaphore> render_ready_semaphores;
    Tools::Array<VkFence> image_in_flight_fences;
    image_ready_semaphores.resize(VulkanOnlineRenderer::max_frames_in_flight);
    render_ready_semaphores.resize(VulkanOnlineRenderer::max_frames_in_flight);
    image_in_flight_fences.resize(VulkanOnlineRenderer::max_frames_in_flight);

    // Prepare the only create infos we'll use for now
    VkSemaphoreCreateInfo semaphore_info;
    populate_semaphore_info(semaphore_info);
    VkFenceCreateInfo fence_info;
    populate_fence_info(fence_info);

    // Allocate two semaphores for each frame in flight
    VkResult vk_result;
    for (uint32_t i = 0; i < VulkanOnlineRenderer::max_frames_in_flight; i++) {
        // First, allocate the image ready one
        if ((vk_result = vkCreateSemaphore(*this->gpu, &semaphore_info, nullptr, &image_ready_semaphores[i])) != VK_SUCCESS) {
            DLOG(fatal, "Could not create image ready semaphore " + std::to_string(i) + ": " + vk_error_map[vk_result]);
        }

        // Next, allocate the render ready one
        if ((vk_result = vkCreateSemaphore(*this->gpu, &semaphore_info, nullptr, &render_ready_semaphores[i])) != VK_SUCCESS) {
            DLOG(fatal, "Could not create render ready semaphore " + std::to_string(i) + ": " + vk_error_map[vk_result]);
        }

        // Finally, allocate a fence
        if ((vk_result = vkCreateFence(*this->gpu, &fence_info, nullptr, &image_in_flight_fences[i])) != VK_SUCCESS) {
            DLOG(fatal, "Could not create fence " + std::to_string(i) + ": " + vk_error_map[vk_result]);
        }
    }
    


    /* Step 5: Game loop */
    DLOG(info, "Entering game loop...");
    uint32_t swapchain_i = 0;
    while (!glfwWindowShouldClose(glfw_window)) {
        // Let's handle the window events
        glfwPollEvents();

        // Try to get a new swapchain image
        uint32_t swapchain_index;
        if ((vk_result = vkAcquireNextImageKHR(*this->gpu, *swapchain, UINT64_MAX, image_ready_semaphores[swapchain_i], VK_NULL_HANDLE, &swapchain_index)) != VK_SUCCESS) {
            DLOG(fatal, "Could not get next swapchain image: " + vk_error_map[vk_result]);
        }

        

        // Then, prepare a new frame by populatin the CPU camera buffer
        GCameraData camera_data({ cam.origin, cam.horizontal, cam.vertical, cam.lower_left_corner });
        camera.set(*this->gpu, staging, staging_cb, this->gpu->memory_queue(), (void*) &camera_data, camera_size);

        // Prepare the descriptor set, with all bindings
        DescriptorSet descriptor_set = this->descriptor_pool->allocate(*this->raytrace_dsl);
        descriptor_set.set(*this->gpu, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0, Tools::Array<Buffer>({ frame }));
        descriptor_set.set(*this->gpu, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, Tools::Array<Buffer>({ camera }));
        descriptor_set.set(*this->gpu, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2, Tools::Array<Buffer>({ vk_entity_faces }));
        descriptor_set.set(*this->gpu, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3, Tools::Array<Buffer>({ vk_entity_vertices }));



        // With that all set, begin recording the command buffer
        CommandBuffer compute_cb = this->compute_command_pool->allocate();
        compute_cb.begin();

        // First, we dispatch the compute shader with its resources
        pipeline.bind(compute_cb);
        descriptor_set.bind(compute_cb, pipeline.layout());
        vkCmdDispatch(compute_cb, (width / 32) + 1, (height / 32) + 1, 1);

        // Next, we add a memory barrier
        VkMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        vkCmdPipelineBarrier(compute_cb, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_DEPENDENCY_DEVICE_GROUP_BIT, 1, &barrier, 0, nullptr, 0, nullptr);

        // Then, we schedule a copy to the swapchain image
        

        // We're done recording
        compute_cb.end();



        // Prepare submitting the command buffer, by adding a semaphore to its submit info
        VkSubmitInfo submit_info = compute_cb.get_submit_info();
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &vk_semaphore;

        // Submit it and don't wait
        if ((vk_result = vkQueueSubmit(gpu->compute_queue(), 1, &submit_info, VK_NULL_HANDLE)) != VK_SUCCESS) {
            DLOG(fatal, "Could not submit compute shader to queue: " + vk_error_map[vk_result]);
        }



        // Copy the resulting buffer 



        // Now, prepare presenting the resulting frame
        VkPresentInfoKHR present_info;
        populate_present_info(present_info, swapchain, image_index);



        // Once done, advance to the next frame in flight
        swapchain_i = (swapchain_i + 1) % VulkanOnlineRenderer::max_frames_in_flight;
    }

    // Before we continue, wait until the device is idle tho
    vkDeviceWaitIdle(*this->gpu);



    /* Step Z: Cleanup. */
    DLOG(info, "Finalizing...");

    // Destroy the semaphores

    // Destroy the buffers

    // Destroy the swapchain
    delete swapchain;

    // Destroy the GLFW stuff
    vkDestroySurfaceKHR(*this->instance, glfw_surface, nullptr);
    glfwDestroyWindow(glfw_window);

    DRETURN;
}



/* Swap operator for the VulkanOnlineRenderer class. */
void RayTracer::swap(VulkanOnlineRenderer& r1, VulkanOnlineRenderer& r2) {
    using std::swap;

    // Swap as VulkanRenderer first
    swap((VulkanRenderer&) r1, (VulkanRenderer&) r2);

    // Done
}





/***** FACTORY METHOD *****/
Renderer* RayTracer::initialize_renderer() {
    DENTER("initialize_renderer");

    // Initialize a new object
    VulkanOnlineRenderer* result = new VulkanOnlineRenderer();

    // Done, return
    DRETURN (Renderer*) result;
}
