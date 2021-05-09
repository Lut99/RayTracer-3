/* VULKAN ONLINE RENDERER.cpp
 *   by Lut99
 *
 * Created:
 *   09/05/2021, 18:30:34
 * Last edited:
 *   09/05/2021, 20:33:43
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

#include "VulkanOnlineRenderer.hpp"

using namespace std;
using namespace RayTracer;
using namespace RayTracer::Compute;
using namespace CppDebugger::SeverityValues;


/***** HELPER FUNCTIONS *****/
/* Initializes the GLFW library, while simultaneously returning the vulkan extensions required as an Array. */
static Tools::Array<const char*> setup_glfw() {
    DENTER("setup_glfw");
    DLOG(info, "Initializing GLFW library...");

    // Initialize GLFW
    glfwInit();

    // Next, use glfwGetRequiredInstanceExtensions to get the Vulkan extensions we'll need
    uint32_t n_extensions = 0;
    const char** raw_extensions = glfwGetRequiredInstanceExtensions(&n_extensions);

    // Put them in an array and return
    DRETURN Tools::Array<const char*>(raw_extensions, n_extensions);
}





/***** VULKANONLINERENDERER CLASS *****/
/* Constructor for the VulkanOnlineRenderer class, which takes nothing to be compatible. */
VulkanOnlineRenderer::VulkanOnlineRenderer() :
    VulkanRenderer(setup_glfw())
{
    DENTER("VulkanOnlineRenderer::VulkanOnlineRenderer");
    DLOG(info, "Initializing Vulkan-based online renderer...");
    DINDENT;
    
    // First, set some general window hints
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    DDEDENT;
    DLEAVE;
}

/* Copy constructor for the VulkanOnlineRenderer class. */
VulkanOnlineRenderer::VulkanOnlineRenderer(const VulkanOnlineRenderer& other) :
    VulkanRenderer(other)
{
    DENTER("VulkanOnlineRenderer::VulkanOnlineRenderer(copy)");
    
    

    DLEAVE;
}

/* Move constructor for the VulkanOnlineRenderer class. */
VulkanOnlineRenderer::VulkanOnlineRenderer(VulkanOnlineRenderer&& other) :
    VulkanRenderer(other)
{
    
}

/* Destructor for the VulkanOnlineRenderer class. */
VulkanOnlineRenderer::~VulkanOnlineRenderer() {
    DENTER("VulkanOnlineRenderer::~VulkanOnlineRenderer");
    DLOG(info, "Cleaning online renderer...");
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
    if ((vk_result = glfwCreateWindowSurface(*this->instance, glfw_window, nullptr, &glfw_surface)) != vk_result) {
        DLOG(fatal, "Could not get GLFW window surface: " + vk_error_map[vk_result]);
    }



    /* Step Y: Game loop */
    DLOG(info, "Entering game loop...");
    while (!glfwWindowShouldClose(glfw_window)) {
        glfwPollEvents();
    }



    /* Step Z: Cleanup. */
    DLOG(info, "Finalizing...");

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
