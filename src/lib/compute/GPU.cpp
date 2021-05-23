/* GPU.cpp
 *   by Lut99
 *
 * Created:
 *   16/04/2021, 17:21:49
 * Last edited:
 *   23/05/2021, 21:17:19
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


/***** POPULATE FUNCTIONS *****/
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
            if (strcmp(extension, supported_extensions[j].extensionName) == 0) {
                // Dope, continue
                found = true;
                break;
            }
        }
        if (!found) {
            // This extension is missing!
            DLOG(warning, "GPU does not support extension '" + std::string(extension) + "'");
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
    DRETURN queue_info.can_compute() && queue_info.can_memory() && supports_extensions;
}

/* Selects a suitable GPU from the ones that support Vulkan. */
static VkPhysicalDevice select_gpu(const Instance& instance, const Tools::Array<const char*>& device_extensions) {
    DENTER("select_gpu");

    // Get how many Vulkan-capable devices are out there
    uint32_t n_available_devices = 0;
    VkResult vk_result;
    if ((vk_result = vkEnumeratePhysicalDevices(instance, &n_available_devices, nullptr)) != VK_SUCCESS) {
        DLOG(fatal, "Could not get the number of available GPUs: " + vk_error_map[vk_result]);
    }
    if (n_available_devices == 0) {
        DLOG(fatal, "No Vulkan-compatible GPUs found.");
    }

    // If there are GPUs, then we fetch a list
    Tools::Array<VkPhysicalDevice> available_devices(n_available_devices);
    if ((vk_result = vkEnumeratePhysicalDevices(instance, &n_available_devices, available_devices.wdata(n_available_devices))) != VK_SUCCESS) {
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
    this->supports_memory = false;
    this->supports_presentation = false;

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
    this->supports_compute = false;
    this->supports_memory = false;
    this->supports_presentation = false;
    for (size_t i = 0; i < supported_queues.size(); i++) {
        // If the current queue is the compute queue, mark that it's supported
        if (supported_queues[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            this->compute_index = (uint32_t) i;
            this->supports_compute = true;
        }
        
        // If the current queue is the memory queue, mark that it's supported
        if (supported_queues[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
            // Only note its existance if it's a) the first queue we see or b) a queue that is not a compute queue
            if (!this->supports_memory || !(supported_queues[i].queueFlags & VK_QUEUE_COMPUTE_BIT)) {
                this->memory_index = (uint32_t) i;
                this->supports_memory = true;
            }
        }
    }

    // Done!
    DRETURN;
}



/* Given a vulkan physical device and a surface, checks the device's capabilities of presenting to that surface. Updates the internal presentation index if it's found a queue. */
void DeviceQueueInfo::check_present(VkPhysicalDevice vk_physical_device, VkSurfaceKHR vk_surface) {
    DENTER("Compute::DeviceQeuueInfo::check_present");

    // First, get up-to-date queue support of the device
    uint32_t n_supported_queues;
    vkGetPhysicalDeviceQueueFamilyProperties(vk_physical_device, &n_supported_queues, nullptr);
    Array<VkQueueFamilyProperties> supported_queues(n_supported_queues);
    vkGetPhysicalDeviceQueueFamilyProperties(vk_physical_device, &n_supported_queues, supported_queues.wdata(n_supported_queues));

    // Next, check if we can find a queue that supports presenting, preferrably a unique one.
    this->supports_presentation = false;
    for (size_t i = 0; i < supported_queues.size(); i++) {
        // Query the support of this queue
        VkBool32 supports_presenting;
        vkGetPhysicalDeviceSurfaceSupportKHR(vk_physical_device, i, vk_surface, &supports_presenting);

        // If it supports presenting, then set it if a) it's the first or b) it's different from any of the other found queues
        if (supports_presenting == VK_TRUE && (!this->supports_presentation || (this->supports_compute && this->presentation_index != this->compute_index && this->supports_memory && this->presentation_index != this->memory_index))) {
            this->supports_presentation = true;
            this->presentation_index = static_cast<uint32_t>(i);
        }
    }

    // // If we found a queue that we like, then dive deeper into the formats available
    // if (this->supports_presentation) {
        
    // }

    // Done, updated!
    DRETURN;
}



/* Returns an Array with all the queue indices. The order is compute. */
Tools::Array<uint32_t> DeviceQueueInfo::queues() const {
    DENTER("Compute::DeviceQueueInfo::queues");

    // Add each of the queues, but only if they differ from what is already there
    Tools::Array<uint32_t> result;
    if (this->supports_compute) {
        result.push_back(this->compute_index);
    }
    if (this->supports_memory && this->compute_index != this->memory_index) {
        result.push_back(this->memory_index);
    }
    if (this->supports_presentation && this->compute_index != this->presentation_index && this->memory_index != this->presentation_index) {
        result.push_back(this->presentation_index);
    }

    // Done already
    DRETURN result;
}





/***** SWAPCHAININFO CLASS *****/
/* Default constructor for the SwapchainInfo class, which initializes this to "nothing supported". */
SwapchainInfo::SwapchainInfo() :
    vk_capabilities({})
{}

/* Constructor for the SwapchainInfo class, which takes a VkPhysicalDevice and a VkSurfaceKHR to populate itself appropriately. */
SwapchainInfo::SwapchainInfo(VkPhysicalDevice vk_physical_device, VkSurfaceKHR vk_surface) {
    DENTER("SwapchainInfo::SwapchainInfo(gpu)");

    // First, getch the capabilities of the device/surface pair
    VkResult vk_result;
    if ((vk_result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk_physical_device, vk_surface, &this->vk_capabilities)) != VK_SUCCESS) {
        DLOG(fatal, "Could not get physical device surface capabilities: " + vk_error_map[vk_result]);
    }

    // Next, get the list of formats
    uint32_t n_formats;
    vkGetPhysicalDeviceSurfaceFormatsKHR(vk_physical_device, vk_surface, &n_formats, nullptr);
    if (n_formats > 0) {
        this->vk_formats.reserve(n_formats);
        vkGetPhysicalDeviceSurfaceFormatsKHR(vk_physical_device, vk_surface, &n_formats, this->vk_formats.wdata(n_formats));
    }

    // Finally, get the list of present modes
    uint32_t n_modes;
    vkGetPhysicalDeviceSurfacePresentModesKHR(vk_physical_device, vk_surface, &n_modes, nullptr);
    if (n_modes > 0) {
        this->vk_present_modes.reserve(n_modes);
        vkGetPhysicalDeviceSurfacePresentModesKHR(vk_physical_device, vk_surface, &n_modes, this->vk_present_modes.wdata(n_modes));
    }

    DLEAVE;
}





/***** GPU CLASS *****/
/* Constructor for the GPU class, which takes a list of required extensions to enable on the GPU. */
GPU::GPU(const Instance& instance, const Tools::Array<const char*>& extensions) :
    instance(instance),
    vk_extensions(extensions)
{
    DENTER("Compute::GPU::GPU");
    DLOG(info, "Initializing GPU object...");
    DINDENT;
    


    // Then, we move to getting a phsysical device that supports our desired properties
    DLOG(info, "Choosing physical device...");

    // Start by selecting a proper one
    this->vk_physical_device = select_gpu(this->instance, this->vk_extensions);

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
    const Tools::Array<Tools::Array<float>> queue_priorities({ Tools::Array<float>({ 1.0f }), Tools::Array<float>({ 1.0f }) });
    populate_queue_infos(queue_infos, this->vk_physical_device_queue_info, queue_priorities);

    // Next, populate the list of features we like from our device.
    VkPhysicalDeviceFeatures device_features;
    populate_device_features(device_features);

    // Then, use the queue indices and the features to populate the create info for the device itself
    VkDeviceCreateInfo device_info;
    populate_device_info(device_info, queue_infos, device_features, this->vk_extensions);

    // With the device info ready, create it
    VkResult vk_result;
    if ((vk_result = vkCreateDevice(this->vk_physical_device, &device_info, nullptr, &this->vk_device)) != VK_SUCCESS) {
        DLOG(fatal, "Could not create the logical device: " + vk_error_map[vk_result]);
    }



    #ifndef NDEBUG
    // For debugging purposes, print the extensions enabled
    DINDENT;
    for (size_t i = 0; i < extensions.size(); i++) {
        DLOG(info, "Enabled extension '" + std::string(extensions[i]) + "'");
    }
    DDEDENT;
    #endif



    // As a quick next step, fetch the relevant device queues
    DLOG(info, "Fetching device queues...");
    vkGetDeviceQueue(this->vk_device, this->vk_physical_device_queue_info.compute(), 0, &this->vk_compute_queue);
    vkGetDeviceQueue(this->vk_device, this->vk_physical_device_queue_info.memory(), 0, &this->vk_memory_queue);



    // We're done initializing!
    DDEDENT;
    DLEAVE;
}

/* Copy constructor for the GPU class. */
GPU::GPU(const GPU& other) :
    instance(other.instance),
    vk_physical_device(other.vk_physical_device),
    vk_physical_device_properties(other.vk_physical_device_properties),
    vk_physical_device_queue_info(other.vk_physical_device_queue_info),
    vk_swapchain_info(other.vk_swapchain_info),
    vk_extensions(other.vk_extensions)
{
    DENTER("Compute::GPU::GPU(copy)");

    // The physical device can be copied literally; just create a new instance of the logical device
    // Collect the qeuues we want to use for this device(s) by creating a list of queue create infos.
    Tools::Array<VkDeviceQueueCreateInfo> queue_infos;
    const Tools::Array<Tools::Array<float>> queue_priorities({ Tools::Array<float>({ 1.0f }), Tools::Array<float>({ 1.0f }) });
    populate_queue_infos(queue_infos, this->vk_physical_device_queue_info, queue_priorities);

    // Next, populate the list of features we like from our device.
    VkPhysicalDeviceFeatures device_features;
    populate_device_features(device_features);

    // Then, use the queue indices and the features to populate the create info for the device itself
    VkDeviceCreateInfo device_info;
    populate_device_info(device_info, queue_infos, device_features, this->vk_extensions);

    // With the device info ready, create it
    VkResult vk_result;
    if ((vk_result = vkCreateDevice(this->vk_physical_device, &device_info, nullptr, &this->vk_device)) != VK_SUCCESS) {
        DLOG(fatal, "Could not create the logical device: " + vk_error_map[vk_result]);
    }

    // Re-fetch the relevant device queues to make sure they are accurate
    vkGetDeviceQueue(this->vk_device, this->vk_physical_device_queue_info.compute(), 0, &this->vk_compute_queue);
    if (this->vk_physical_device_queue_info.n_queues() > 1) {
        vkGetDeviceQueue(this->vk_device, this->vk_physical_device_queue_info.memory(), 0, &this->vk_memory_queue);
    }

    DLEAVE;
}

/* Move constructor for the GPU class. */
GPU::GPU(GPU&& other) :
    instance(other.instance),
    vk_physical_device(other.vk_physical_device),
    vk_physical_device_properties(other.vk_physical_device_properties),
    vk_physical_device_queue_info(other.vk_physical_device_queue_info),
    vk_device(other.vk_device),
    vk_compute_queue(other.vk_compute_queue),
    vk_memory_queue(other.vk_memory_queue),
    vk_presentation_queue(other.vk_presentation_queue),
    vk_swapchain_info(other.vk_swapchain_info),
    vk_extensions(other.vk_extensions)
{
    // Set the device to a nullptr so the now useless object doesn't destruct it
    other.vk_device = nullptr;
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

    // We're done here
    DDEDENT;
    DLEAVE;
}



/* Updates the internal queue info on whether or not the GPU can present to the given surface. */
void GPU::check_present(VkSurfaceKHR vk_surface) {
    DENTER("Compute::GPU::check_present");

    // First, call the queue's update function
    this->vk_physical_device_queue_info.check_present(this->vk_physical_device, vk_surface);

    // Next, throw an error if cannot present
    if (!this->vk_physical_device_queue_info.can_present()) {
        DLOG(fatal, "GPU '" + this->name() + "' has no queue that can present to the given surface.");
    }

    // With the queue prepared, load the swapchain info
    this->vk_swapchain_info = SwapchainInfo(this->vk_physical_device, vk_surface);
    // Verify that the device supports the required modes & formats
    if (this->vk_swapchain_info.formats().empty() || this->vk_swapchain_info.present_modes().empty()) {
        DLOG(fatal, "GPU '" + this->name() + "' does not support the format & present mode required for the given surface.");
    }

    // If it is valid, then fetch the queue
    vkGetDeviceQueue(this->vk_device, this->vk_physical_device_queue_info.presentation(), 0, &this->vk_presentation_queue);

    // Done
    DRETURN;
}



/* Swap operator for the GPU class. */
void Compute::swap(GPU& g1, GPU& g2) {
    DENTER("Compute::swap(GPU)");

    #ifndef NDEBUG
    // Check if the instances are actually the same
    if (g1.instance != g2.instance) {
        DLOG(fatal, "Cannot swap gpus with different instances");
    }
    #endif

    // Swap all operators
    using std::swap;
    swap(g1.vk_physical_device, g2.vk_physical_device);
    swap(g1.vk_physical_device_properties, g2.vk_physical_device_properties);
    swap(g1.vk_physical_device_queue_info, g2.vk_physical_device_queue_info);
    swap(g1.vk_device, g2.vk_device);
    swap(g1.vk_compute_queue, g2.vk_compute_queue);
    swap(g1.vk_memory_queue, g2.vk_memory_queue);
    swap(g1.vk_presentation_queue, g2.vk_presentation_queue);
    swap(g1.vk_swapchain_info, g2.vk_swapchain_info);
    swap(g1.vk_extensions, g2.vk_extensions);

    // Done
    DRETURN;
}
