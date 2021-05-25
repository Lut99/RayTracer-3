/* INSTANCE.cpp
 *   by Lut99
 *
 * Created:
 *   30/04/2021, 14:03:33
 * Last edited:
 *   25/05/2021, 18:14:13
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Contains the Instance class, which wraps the Vulkan instance and
 *   manages the debugger.
**/

#include <CppDebugger.hpp>

#include "ErrorCodes.hpp"

#include "Instance.hpp"

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





/***** INSTANCE CLASS *****/
/* Constructor for the Instance class, which takes a list of extensions to enable at the instance and layers to enable. */
Instance::Instance(const Tools::Array<const char*>& extensions, const Tools::Array<const char*>& layers) :
    vk_debugger(nullptr),
    vk_destroy_debug_utils_messenger_method(nullptr),
    vk_extensions(extensions),
    vk_layers(layers)
{
    DENTER("Compute::Instance::Instance");
    DLOG(info, "Initializing Vulkan instance...");
    DINDENT;



    /* First, setup the instance. */
    DLOG(info, "Creating instance...");

    // Start by defining meta stuff
    VkApplicationInfo app_info;
    populate_application_info(app_info);

    // Next, setup the list of extensions
    VkInstanceCreateInfo instance_info;
    populate_instance_info(instance_info, app_info, this->vk_extensions, this->vk_layers);

    // Now, setup the instance
    VkResult vk_result;
    if ((vk_result = vkCreateInstance(&instance_info, nullptr, &this->vk_instance)) != VK_SUCCESS) {
        DLOG(fatal, "Could not create the Vulkan instance: " + vk_error_map[vk_result]);
    }

    // If we were successfull, print which extensions
    DINDENT;
    for (size_t i = 0; i < instance_extensions.size(); i++) {
        DLOG(info, std::string("Enabled extension '") + this->vk_extensions[i] + "'");
    }
    DDEDENT;



    /* Next, initialize the debugger - but only if debug is set and the layer is enabled. */
    #ifndef NDEBUG
    for (size_t i = 0; i < this->vk_layers.size(); i++) {
        if (this->vk_layers[i] == std::string("VK_LAYER_KHRONOS_validation")) {
            DLOG(info, "Initializing debug logger...");

            // First, we load the two extension functions needed using the dynamic loader
            PFN_vkCreateDebugUtilsMessengerEXT vk_create_debug_utils_messenger_method = (PFN_vkCreateDebugUtilsMessengerEXT) load_instance_method(this->vk_instance, "vkCreateDebugUtilsMessengerEXT");
            this->vk_destroy_debug_utils_messenger_method = (PFN_vkDestroyDebugUtilsMessengerEXT) load_instance_method(this->vk_instance, "vkDestroyDebugUtilsMessengerEXT");

            // Next, define the messenger
            VkDebugUtilsMessengerCreateInfoEXT debug_info;
            populate_debug_info(debug_info);

            // And with that, create it
            if ((vk_result = vk_create_debug_utils_messenger_method(this->vk_instance, &debug_info, nullptr, &this->vk_debugger)) != VK_SUCCESS) {
                DLOG(fatal, "Could not create the logger: " + vk_error_map[vk_result]);
            }

            // We're done searching
            break;
        }
    }
    #endif



    DDEDENT;
    DLEAVE;
}

/* Copy constructor for the Instance class. */
Instance::Instance(const Instance& other) :
    vk_debugger(other.vk_debugger),
    vk_destroy_debug_utils_messenger_method(other.vk_destroy_debug_utils_messenger_method),
    vk_extensions(other.vk_extensions),
    vk_layers(other.vk_layers)
{
    DENTER("Compute::Instance::Instance(copy)");

    // First, re-create the instance itself by defining the application info again
    VkApplicationInfo app_info;
    populate_application_info(app_info);

    // Next, setup the list of extensions
    VkInstanceCreateInfo instance_info;
    populate_instance_info(instance_info, app_info, this->vk_extensions, this->vk_layers);

    // Now, setup the instance
    VkResult vk_result;
    if ((vk_result = vkCreateInstance(&instance_info, nullptr, &this->vk_instance)) != VK_SUCCESS) {
        DLOG(fatal, "Could not create the Vulkan instance: " + vk_error_map[vk_result]);
    }



    #ifndef NDEBUG
    // Next, initialize the debugger, but only if the copied object had so as well
    if (this->vk_debugger != nullptr) {
        // First, we load the create extension function. The destroy is assumed to be copied from the other Instance.
        PFN_vkCreateDebugUtilsMessengerEXT vk_create_debug_utils_messenger_method = (PFN_vkCreateDebugUtilsMessengerEXT) load_instance_method(this->vk_instance, "vkCreateDebugUtilsMessengerEXT");

        // Next, define the messenger
        VkDebugUtilsMessengerCreateInfoEXT debug_info;
        populate_debug_info(debug_info);

        // And with that, create it
        if ((vk_result = vk_create_debug_utils_messenger_method(this->vk_instance, &debug_info, nullptr, &this->vk_debugger)) != VK_SUCCESS) {
            DLOG(fatal, "Could not create the logger: " + vk_error_map[vk_result]);
        }
    }
    #endif



    // Done
    DLEAVE;
}

/* Move constructor for the Instance class. */
Instance::Instance(Instance&& other) :
    vk_instance(other.vk_instance),
    vk_debugger(other.vk_debugger),
    vk_destroy_debug_utils_messenger_method(other.vk_destroy_debug_utils_messenger_method),
    vk_extensions(other.vk_extensions),
    vk_layers(other.vk_layers)
{
    // Set everything to nullptrs in the other function to avoid deallocation
    other.vk_instance = nullptr;
    other.vk_debugger = nullptr;
}

/* Destructor for the Instance class. */
Instance::~Instance() {
    DENTER("Compute::Instance::~Instance");
    DLOG(info, "Cleaning Vulkan instance...");
    DINDENT;

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

    DDEDENT;
    DLEAVE;
}



/* Swap operator for the Instance class. */
void Compute::swap(Instance& i1, Instance& i2) {
    DENTER("Compute::swap(Instance)");

    using std::swap;
    swap(i1.vk_instance, i2.vk_instance);
    swap(i1.vk_debugger, i2.vk_debugger);
    swap(i1.vk_destroy_debug_utils_messenger_method, i2.vk_destroy_debug_utils_messenger_method);
    swap(i1.vk_extensions, i2.vk_extensions),
    swap(i1.vk_layers, i2.vk_layers);

    DRETURN;
}
