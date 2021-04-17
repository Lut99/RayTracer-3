/* GPU.cpp
 *   by Lut99
 *
 * Created:
 *   16/04/2021, 17:21:49
 * Last edited:
 *   17/04/2021, 16:58:05
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


/***** DEBUG CALLBACK *****/
static VKAPI_ATTR VkBool32 VKAPI_CALL vk_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                                                  VkDebugUtilsMessageTypeFlagsEXT message_type,
                                                  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                  void* user_data)
{
    // Determine the logging severity
    CppDebugger::Severity severity;
    switch(message_severity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            severity = info;
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            severity = vulkan_warning;
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            severity = vulkan_error;
            break;
        default:
            severity = (CppDebugger::Severity) -1;
            break;
    }

    // Do not use the message type for now
    (void) message_type;

    // Log the message with the correct severity
    CppDebugger::Debugger* debugger = (CppDebugger::Debugger*) user_data;
    debugger->log(severity, pCallbackData->pMessage);
    return VK_FALSE;
}





/***** POPULATE FUNCTIONS *****/
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

/* Populates a VkDebugUtilsMessengerCreateInfoEXT struct with the hardcoded settings. */
static void populate_debug_info(VkDebugUtilsMessengerCreateInfoEXT& debug_info) {
    DENTER("populate_debug_info");

    // Set the struct to 0 and set its type
    debug_info = {};
    debug_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

    // Note which debug sevirities & message types we want to log
    debug_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debug_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

    // Define the callback with an optional data struct (the Debugger itself)
    debug_info.pfnUserCallback = vk_callback;
    debug_info.pUserData = (void*) &CppDebugger::debugger;

    // Done
    DRETURN;
}

/* Populates a list of VkDeviceQueueCreateInfo structs based on the given queue info of a device. */
static void populate_queue_infos(Tools::Array<VkDeviceQueueCreateInfo>& queue_infos, const DeviceQueueInfo& vk_physical_device_queue_info, const Tools::Array<Tools::Array<float>>& queue_priorities) {
    DENTER("populate_queue_infos");

    // Fetch the indices as a list
    Tools::Array<uint32_t> queue_indices = vk_physical_device_queue_info.queues();

    // Reserve enough space in the result array
    queue_infos.resize(queue_indices.size());
    
    // Loop to populate the create infos
    for (size_t i = 0; i < queue_indices.size(); i++) {
        // Set the meta properties of the struct
        queue_infos[i] = {};
        queue_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;

        // Next, tell the struct that we want one queue per family
        queue_infos[i].queueFamilyIndex = queue_indices[i];
        queue_infos[i].queueCount = 1;

        // Finally, give each queue the same priority
        DINDENT;
        DLOG(info, "Device queue priority: " + std::to_string(queue_priorities[i][0]));
        DDEDENT;
        queue_infos[i].pQueuePriorities = queue_priorities[i].rdata();
    }

    // Done
    DRETURN;
}

/* Populates a VkPhysicalDeviceFeatures struct with hardcoded settings. */
static void populate_device_features(VkPhysicalDeviceFeatures& device_features) {
    DENTER("populate_device_features");

    // None!
    device_features = {};

    DRETURN;
}

/* Populates a VkDeviceCreateInfo struct based on the given list of qeueu infos and the given device features. */
static void populate_device_info(VkDeviceCreateInfo& device_info, const Tools::Array<VkDeviceQueueCreateInfo>& queue_infos, const VkPhysicalDeviceFeatures& device_features, const Tools::Array<const char*>& device_extensions) {
    DENTER("populate_device_info");

    // Set the meta info first
    device_info = {};
    device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    // Next, pass the queue infos
    device_info.queueCreateInfoCount = static_cast<uint32_t>(queue_infos.size());
    device_info.pQueueCreateInfos = queue_infos.rdata();

    // Pass the device features
    device_info.pEnabledFeatures = &device_features;

    // Finally, add the extensions we want to enable to the device
    device_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
    device_info.ppEnabledExtensionNames = device_extensions.rdata();

    // Done!
    DRETURN;
}





/***** DYNAMIC LOADER FUNCTIONS *****/
/* Loads a given method ('s address) from the given instance. */
static PFN_vkVoidFunction load_instance_method(VkInstance vk_instance, const char* method_name) {
    DENTER("load_instance_method");

    // Fetch the function pointer
    PFN_vkVoidFunction to_return = vkGetInstanceProcAddr(vk_instance, method_name);
    if (to_return == nullptr) {
        DLOG(fatal, "Could not load function '" + std::string(method_name) + "'.");
    }

    // Otherwise, log success and return
    DINDENT;
    DLOG(info, "Loaded function '" + std::string(method_name) + "'.");
    DDEDENT;
    DRETURN to_return;
}





/***** GPU SELECTION HELPER FUNCTIONS *****/
/* Given a physical device, checks if it supports the required list of extensions. */
static bool gpu_supports_extensions(VkPhysicalDevice vk_physical_device, const Tools::Array<const char*>& device_extensions) {
    DENTER("gpu_supports_extensions");

    // Get a list of all extensions supported on this device
    uint32_t n_supported_extensions = 0;
    VkResult vk_result;
    if ((vk_result = vkEnumerateDeviceExtensionProperties(vk_physical_device, nullptr, &n_supported_extensions, nullptr)) != VK_SUCCESS) {
        DLOG(warning, "Could not get the number of supported extensions on the GPU.");
        DRETURN false;
    }
    Tools::Array<VkExtensionProperties> supported_extensions(n_supported_extensions);
    if ((vk_result = vkEnumerateDeviceExtensionProperties(vk_physical_device, nullptr, &n_supported_extensions, supported_extensions.wdata(n_supported_extensions))) != VK_SUCCESS) {
        DLOG(warning, "Could not get the number of supported extensions on the GPU.");
        DRETURN false;
    }

    // Now simply make sure that all extensions in the list appear in the retrieved list
    for (size_t i = 0; i < device_extensions.size(); i++) {
        const char* extension = device_extensions[i];

        // Try to find it in the list of supported extensions
        bool found = false;
        for (size_t j = 0; j < supported_extensions.size(); j++) {
            if (strcmp(extension, supported_extensions[j].extensionName) != 0) {
                // Dope, continue
                found = true;
                break;
            }
        }
        if (!found) {
            // This extension is missing!
            DRETURN false;
        }
    }

    // However, if all extensions passed the test, we're done
    DRETURN true;
}

/* Given a physical device, checks if it meets our needs. */
static bool is_suitable_gpu(VkPhysicalDevice vk_physical_device, const Tools::Array<const char*>& device_extensions) {
    DENTER("is_suitable_gpu");

    // First, we get a list of supported queues on this device
    DeviceQueueInfo queue_info(vk_physical_device);

    // Next, check if the device supports the extensions we want
    bool supports_extensions = gpu_supports_extensions(vk_physical_device, device_extensions);

    // With those two, return it the GPU is suitable
    DRETURN queue_info.can_compute() && supports_extensions;
}

/* Selects a suitable GPU from the ones that support Vulkan. */
static VkPhysicalDevice select_gpu(VkInstance vk_instance, const Tools::Array<const char*>& device_extensions) {
    DENTER("select_gpu");

    // Get how many Vulkan-capable devices are out there
    uint32_t n_available_devices = 0;
    VkResult vk_result;
    if ((vk_result = vkEnumeratePhysicalDevices(vk_instance, &n_available_devices, nullptr)) != VK_SUCCESS) {
        DLOG(fatal, "Could not get the number of available GPUs: " + vk_error_map[vk_result]);
    }
    if (n_available_devices == 0) {
        DLOG(fatal, "No Vulkan-compatible GPUs found.");
    }

    // If there are GPUs, then we fetch a list
    Tools::Array<VkPhysicalDevice> available_devices(n_available_devices);
    if ((vk_result = vkEnumeratePhysicalDevices(vk_instance, &n_available_devices, available_devices.wdata(n_available_devices))) != VK_SUCCESS) {
        DLOG(fatal, "Could not get list of available GPUs: " + vk_error_map[vk_result]);
    }

    // Iterate through each of them to find a suitable one
    for (size_t i = 0; i < available_devices.size(); i++) {
        // Just take the first suitable one
        if (is_suitable_gpu(available_devices[i], device_extensions)) {
            DRETURN available_devices[i];
        }
    }

    // Otherwise, we didn't find any
    DLOG(fatal, "Could not find a supported GPU.");
    DRETURN nullptr;
}





/***** DEVICEQUEUEINFO CLASS *****/
/* Default constructor for the DeviceQueueInfo class, which initializes this to "nothing supported". */
DeviceQueueInfo::DeviceQueueInfo() {
    DENTER("Compute::DeviceQueueInfo::DeviceQueueInfo");

    // We mark it as didn't find
    this->supports_compute = false;

    // Done!
    DRETURN;
}

/* Constructor for the DeviceQueueInfo class, which takes the device to extract the properties from. */
DeviceQueueInfo::DeviceQueueInfo(VkPhysicalDevice vk_physical_device)
{
    DENTER("Compute::DeviceQueueInfo::DeviceQueueInfo(gpu)");

    // First, get a list of all the queues supported by the GPU
    uint32_t n_supported_queues;
    vkGetPhysicalDeviceQueueFamilyProperties(vk_physical_device, &n_supported_queues, nullptr);
    Array<VkQueueFamilyProperties> supported_queues(n_supported_queues);
    vkGetPhysicalDeviceQueueFamilyProperties(vk_physical_device, &n_supported_queues, supported_queues.wdata(n_supported_queues));

    // Loop through the queues to find the compute queue
    for (size_t i = 0; i < supported_queues.size(); i++) {
        // If the current queue is the compute queue, we're done
        if (supported_queues[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            this->compute_index = (uint32_t) i;
            this->supports_compute = true;
            DRETURN;
        }
    }

    // Otherwise, we mark it as didn't find
    this->supports_compute = false;

    // Done!
    DRETURN;
}





/***** GPU CLASS *****/
/* Constructor for the GPU class, which takes a list of required extensions to enable on the GPU. Does not enable validation layers. */
GPU::GPU(const Tools::Array<const char*>& instance_extensions, const Tools::Array<const char*>& device_extensions, const Tools::Array<const char*>& required_layers) {
    DENTER("Compute::GPU::GPU");
    DLOG(info, "Initializing GPU object...");
    DINDENT;


    
    DLOG(info, "Initializing Vulkan instance...");

    // Start by defining meta stuff
    VkApplicationInfo app_info;
    populate_application_info(app_info);

    // Next, setup the list of extensions
    VkInstanceCreateInfo instance_info;
    populate_instance_info(instance_info, app_info, instance_extensions, required_layers);

    // Now, setup the instance
    VkResult vk_result;
    if ((vk_result = vkCreateInstance(&instance_info, nullptr, &this->vk_instance)) != VK_SUCCESS) {
        DLOG(fatal, "Could not create the Vulkan instance: " + vk_error_map[vk_result]);
    }

    // If we were successfull, print which extensions
    DINDENT;
    for (size_t i = 0; i < instance_extensions.size(); i++) {
        DLOG(auxillary, std::string("Enabled extension '") + instance_extensions[i] + "'");
    }
    DDEDENT;

    

    // Next up is initializing the debugger - but only if the proper layer is present
    this->vk_debugger = nullptr;
    this->vk_destroy_debug_utils_messenger_method = nullptr;
    for (size_t i = 0; i < required_layers.size(); i++) {
        if (required_layers[i] == std::string("VK_LAYER_KHRONOS_validation")) {
            DLOG(info, "Initializing Vulkan debugger...");

            // First, we load the two extension functions needed using the dynamic loader
            PFN_vkCreateDebugUtilsMessengerEXT vk_create_debug_utils_messenger_method = (PFN_vkCreateDebugUtilsMessengerEXT) load_instance_method(this->vk_instance, "vkCreateDebugUtilsMessengerEXT");
            this->vk_destroy_debug_utils_messenger_method = (PFN_vkDestroyDebugUtilsMessengerEXT) load_instance_method(this->vk_instance, "vkDestroyDebugUtilsMessengerEXT");

            // Next, define the messenger
            VkDebugUtilsMessengerCreateInfoEXT debug_info;
            populate_debug_info(debug_info);

            // And with that, create it
            if ((vk_result = vk_create_debug_utils_messenger_method(this->vk_instance, &debug_info, nullptr, &this->vk_debugger)) != VK_SUCCESS) {
                DLOG(fatal, "Could not create the Vulkan debugger: " + vk_error_map[vk_result]);
            }

            // We're done searching
            break;
        }
    }
    


    // Then, we move to getting a phsysical device that supports our desired properties
    DLOG(info, "Choosing physical device...");

    // Start by selecting a proper one
    this->vk_physical_device = select_gpu(this->vk_instance, device_extensions);

    // Next, get some of its properties, like the name & queue info
    vkGetPhysicalDeviceProperties(this->vk_physical_device, &this->vk_physical_device_properties);
    this->vk_physical_device_queue_info = DeviceQueueInfo(this->vk_physical_device);
    DINDENT;
    DLOG(auxillary, std::string("Selected GPU: '") + this->vk_physical_device_properties.deviceName + "'");
    DDEDENT;

    

    // With the physical device, create the logical device
    DLOG(info, "Initializing logical device...");

    // Collect the qeuues we want to use for this device(s) by creating a list of queue create infos.
    Tools::Array<VkDeviceQueueCreateInfo> queue_infos;
    const Tools::Array<Tools::Array<float>> queue_priorities({ Tools::Array<float>({ 1.0f }) });
    populate_queue_infos(queue_infos, this->vk_physical_device_queue_info, queue_priorities);

    // Next, populate the list of features we like from our device.
    VkPhysicalDeviceFeatures device_features;
    populate_device_features(device_features);

    // Then, use the queue indices and the features to populate the create info for the device itself
    VkDeviceCreateInfo device_info;
    populate_device_info(device_info, queue_infos, device_features, device_extensions);

    // With the device info ready, create it
    if ((vk_result = vkCreateDevice(this->vk_physical_device, &device_info, nullptr, &this->vk_device)) != VK_SUCCESS) {
        DLOG(fatal, "Could not create the logical device: " + vk_error_map[vk_result]);
    }



    // As a quick next step, fetch the relevant device queues
    DLOG(info, "Fetching device queues...");
    vkGetDeviceQueue(this->vk_device, this->vk_physical_device_queue_info.compute(), 0, &this->vk_compute_queue);



    // We're done initializing!
    DDEDENT;
    DLOG(info, "GPU initialized.");
    DLEAVE;
}

/* Move constructor for the GPU class. */
GPU::GPU(GPU&& other) :
    vk_instance(other.vk_instance),
    vk_debugger(other.vk_debugger),
    vk_destroy_debug_utils_messenger_method(other.vk_destroy_debug_utils_messenger_method),
    vk_physical_device(other.vk_physical_device),
    vk_physical_device_properties(other.vk_physical_device_properties),
    vk_physical_device_queue_info(other.vk_physical_device_queue_info),
    vk_device(other.vk_device),
    vk_compute_queue(other.vk_compute_queue)
{
    DENTER("Compute::GPU::GPU(move)");

    // Set all deallocated objects to nullptrs in the other object, to prevent it from doing so
    other.vk_instance = nullptr;
    other.vk_debugger = nullptr;
    other.vk_device = nullptr;

    DLEAVE;
}

/* Destructor for the GPU class. */
GPU::~GPU() {
    DENTER("Compute::GPU::~GPU");
    DLOG(info, "Cleaning GPU...");
    DINDENT;

    // Destroy them in reverse order: first, the logical device
    if (this->vk_device != nullptr) {
        DLOG(info, "Cleaning logical device...");
        vkDestroyDevice(this->vk_device, nullptr);
    }

    // Physical devices need no destructing
    
    // Destroy the debugger
    if (this->vk_debugger != nullptr) {
        DLOG(info, "Cleaning Vulkan debugger...");
        this->vk_destroy_debug_utils_messenger_method(this->vk_instance, this->vk_debugger, nullptr);
    }

    // Finally, destroy the instance
    if (this->vk_instance != nullptr) {
        DLOG(info, "Cleaning Vulkan instance...");
        vkDestroyInstance(this->vk_instance, nullptr);
    }

    // We're done here
    DDEDENT;
    DLOG(info, "Cleaned GPU.");
    DLEAVE;
}



/* Move assignment operator for the GPU class. */
GPU& GPU::operator=(GPU&& other) {
    if (this != &other) { swap(*this, other); }
    return *this;
}

/* Swap operator for the GPU class. */
void Compute::swap(GPU& g1, GPU& g2) {
    using std::swap;

    // Swap all operators
    swap(g1.vk_instance, g2.vk_instance);
    swap(g1.vk_debugger, g2.vk_debugger);
    swap(g1.vk_destroy_debug_utils_messenger_method, g2.vk_destroy_debug_utils_messenger_method);
    swap(g1.vk_physical_device, g2.vk_physical_device);
    swap(g1.vk_physical_device_properties, g2.vk_physical_device_properties);
    swap(g1.vk_physical_device_queue_info, g2.vk_physical_device_queue_info);
    swap(g1.vk_device, g2.vk_device);
    swap(g1.vk_compute_queue, g2.vk_compute_queue);
}
