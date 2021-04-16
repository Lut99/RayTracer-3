/* GPU.cpp
 *   by Lut99
 *
 * Created:
 *   16/04/2021, 17:21:49
 * Last edited:
 *   16/04/2021, 19:52:04
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Main library that is an abstraction of a Vulkan GPU, which binds
 *   itself to a GPU we choose and then interfaces us with the Vulkan
 *   library.
**/

#include <CppDebugger.hpp>

#include "compute/ErrorCodes.hpp"

#include "GPU.hpp"

using namespace std;
using namespace RayTracer;
using namespace RayTracer::Compute;
using namespace CppDebugger::SeverityValues;


/***** HELPER FUNCTIONS *****/
/* Populates a VkApplicationInfo struct with the application info we hardcoded here. */
static void populate_application_info(VkApplicationInfo& app_info) {
    DENTER("populate_application_info");

    // Set the struct to 0 and set its type
    app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;

    // Next, set the name & version info of the application
    app_info.pApplicationName = "RayTracer-3";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);

    // We're not using any well-known engine, so tell Vulkan we're not
    app_info.pEngineName = "No Engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);

    // Finally, define the minimum API level to use
    app_info.apiVersion = VK_API_VERSION_1_0;

    // Done
    DRETURN;
}

/* Populates a VkInstanceCreateInfo struct with the given list of extensions and layers. If you don't want to enable either of those, pass empty lists. */
static void populate_instance_info(VkInstanceCreateInfo& instance_info, const VkApplicationInfo& app_info, const Tools::Array<const char*>& extensions, const Tools::Array<const char*>& layers) {
    DENTER("populate_instance_info");

    // Set the struct to 0 and set its type
    instance_info = {};
    instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    // Link the application info to this instance
    instance_info.pApplicationInfo = &app_info;

    // Set the extensions
    instance_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    instance_info.ppEnabledExtensionNames = extensions.size() > 0 ? extensions.rdata() : nullptr;

    // Set the layers
    instance_info.enabledLayerCount = static_cast<uint32_t>(layers.size());
    instance_info.ppEnabledLayerNames = layers.size() > 0 ? layers.rdata() : nullptr;

    // Done
    DRETURN;
}



/***** GPU CLASS *****/
/* Constructor for the GPU class, which takes a list of required extensions to enable on the GPU. Does not enable validation layers. */
GPU::GPU(const Tools::Array<const char*>& required_extensions, const Tools::Array<const char*>& required_layers) {
    DENTER("Compute::GPU::GPU");
    DLOG(info, "Initializing GPU object...");
    DINDENT;


    
    DLOG(info, "Initializing Vulkan instance...");

    // Start by defining meta stuff
    VkApplicationInfo app_info;
    populate_application_info(app_info);

    // Next, setup the list of extensions
    VkInstanceCreateInfo instance_info;
    populate_instance_info(instance_info, app_info, required_extensions, required_layers);

    // Now, setup the instance
    VkResult vk_result;
    if ((vk_result = vkCreateInstance(&instance_info, nullptr, &this->vk_instance)) != VK_SUCCESS) {
        DLOG(fatal, "Could not create the Vulkan instance: " + vk_error_map[vk_result]);
    }

    

    // Next up, 
    
}

/* Move constructor for the GPU class. */
GPU::GPU(GPU&& other) {
    
}

/* Destructor for the GPU class. */
GPU::~GPU() {
    
}



/* Move assignment operator for the GPU class. */
GPU& GPU::operator=(GPU&& other) {
    if (this != &other) { swap(*this, other); }
    return *this;
}

/* Swap operator for the GPU class. */
void Compute::swap(GPU& g1, GPU& g2) {
    using std::swap;


}
