/* INSTANCE.hpp
 *   by Lut99
 *
 * Created:
 *   30/04/2021, 14:03:39
 * Last edited:
 *   30/04/2021, 14:32:06
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Contains the Instance class, which wraps the Vulkan instance and
 *   manages the debugger.
**/

#ifndef COMPUTE_INSTANCE_HPP
#define COMPUTE_INSTANCE_HPP

#include <vulkan/vulkan.h>

#include "tools/Array.hpp"

namespace RayTracer::Compute {
    /* The Vulkan instance extensions we want to be enabled. */
    static const Tools::Array<const char*> instance_extensions({
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME
    });
    /* The Vulkan validation layers we want to be enabled. */
    static const Tools::Array<const char*> debug_layers({
        "VK_LAYER_KHRONOS_validation" 
    });


    
    /* The Instance class, which wraps and manages the Vulkan instance and the Vulkan debug logger. */
    class Instance {
    private:
        /* The VkInstance that we use, among other things. */
        VkInstance vk_instance;

        /* The debug messenger of Vulkan. */
        VkDebugUtilsMessengerEXT vk_debugger;
        /* The function needed to destroy the Vulkan debug messenger. */
        PFN_vkDestroyDebugUtilsMessengerEXT vk_destroy_debug_utils_messenger_method;
    
    public:
        /* Constructor for the Instance class, which takes a list of extensions to enable at the instance and layers to enable. */
        Instance(const Tools::Array<const char*>& extensions = instance_extensions, const Tools::Array<const char*>& layers = debug_layers);
        /* Copy constructor for the Instance class, which is deleted. */
        Instance(const Instance& other);
        /* Move constructor for the Instance class. */
        Instance(Instance&& other);
        /* Destructor for the Instance class. */
        ~Instance();

        /* Explicitly returns the internal VkInstance object. */
        inline VkInstance instance() const { return this->vk_instance; }
        /* Implicitly returns the internal VkInstance object. */
        inline operator VkInstance() const { return this->vk_instance; }

        /* Copy assignment operator for the Instance class, which is deleted. */
        Instance& operator=(const Instance& other) = delete;
        /* Move assignment operator for the Instance class. */
        inline Instance& operator=(Instance&& other) { if (this != &other) { swap(*this, other); } return *this; }
        /* Swap operator for the Instance class. */
        friend void swap(Instance& i1, Instance& i2);

    };

    /* Swap operator for the Instance class. */
    void swap(Instance& i1, Instance& i2);



    /* Global Vulkan instance. Should be declared elsewhere. */
    extern const Instance vk_instance;
}

#endif
