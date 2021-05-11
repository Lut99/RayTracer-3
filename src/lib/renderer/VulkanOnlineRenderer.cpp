/* VULKAN ONLINE RENDERER.cpp
 *   by Lut99
 *
 * Created:
 *   09/05/2021, 18:30:34
 * Last edited:
 *   11/05/2021, 21:15:47
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

#include "compute/ErrorCodes.hpp"
#include "compute/Swapchain.hpp"

#include "VulkanOnlineRenderer.hpp"

using namespace std;
using namespace RayTracer;
using namespace RayTracer::Compute;
using namespace CppDebugger::SeverityValues;


/***** VULKANONLINERENDERER CLASS *****/
/* Constructor for the VulkanOnlineRenderer class, which takes nothing to be compatible. Note that it does not rely on the parent constructor, since we want to start the vulkan instance & GPU differently. */
VulkanOnlineRenderer::VulkanOnlineRenderer() {
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
void VulkanOnlineRenderer::render(Camera& camera) const {
    DENTER("VulkanOnlineRenderer::render");

    /* Step 1: Initialize the window we'll be rendering to and get a surface. */
    DLOG(info, "Creating GLFW window...");
    GLFWwindow* glfw_window = glfwCreateWindow(camera.w(), camera.h(), "RayTracer-3", NULL, NULL);
    VkSurfaceKHR glfw_surface;
    VkResult vk_result;
    if ((vk_result = glfwCreateWindowSurface(*this->instance, glfw_window, nullptr, &glfw_surface)) != VK_SUCCESS) {
        DLOG(fatal, "Could not get GLFW window surface: " + vk_error_map[vk_result]);
    }

    // Also update the GPU info for its presentation queue support
    this->gpu->check_present(glfw_surface);



    /* Step 2: Prepare the initial swapchain. */
    Swapchain* swapchain = new Swapchain(*this->gpu, glfw_window, glfw_surface);



    /* Step Y: Game loop */
    DLOG(info, "Entering game loop...");
    while (!glfwWindowShouldClose(glfw_window)) {
        glfwPollEvents();
    }



    /* Step Z: Cleanup. */
    DLOG(info, "Finalizing...");

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
