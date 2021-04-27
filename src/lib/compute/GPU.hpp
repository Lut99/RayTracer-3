/* GPU.hpp
 *   by Lut99
 *
 * Created:
 *   16/04/2021, 17:21:54
 * Last edited:
 *   27/04/2021, 14:09:02
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Main library that is an abstraction of a Vulkan GPU, which binds
 *   itself to a GPU we choose and then interfaces us with the Vulkan
 *   library.
**/

#ifndef COMPUTE_GPU_HPP
#define COMPUTE_GPU_HPP

#include <vulkan/vulkan.h>

#include "tools/Array.hpp"

namespace RayTracer::Compute {
    /* The DeviceQueueInfo class, which describes the queues that a GPU supports. */
    class DeviceQueueInfo {
    private:
        /* The index of the compute family queue on the device. */
        uint32_t compute_index;
        /* Boolean that keeps track of whether or not the compute queue is supported. */
        bool supports_compute;

        /* The index of the memory family queue on the device. */
        uint32_t memory_index;
        /* Boolean that keeps track of whether or not the memory queue is supported. */
        bool supports_memory;

    public:
        /* Default constructor for the DeviceQueueInfo class, which initializes this to "nothing supported". */
        DeviceQueueInfo();
        /* Constructor for the DeviceQueueInfo class, which takes the index of the compute queue and a boolean indicating whether it exists or not. */
        DeviceQueueInfo(VkPhysicalDevice vk_physical_device);

        /* Returns the queue index of the GPU's compute queue. Note that the value returned by this is undefined if can_compute == false. */
        inline uint32_t compute() const { return this->compute_index; }
        /* Returns whether or not the compute queue is supported by this device. */
        inline bool can_compute() const { return this->supports_compute; }

        /* Returns the queue index of the GPU's memory queue. Note that the value returned by this is undefined if can_memory == false. Also note that this may be the same as the compute queue. */
        inline uint32_t memory() const { return this->memory_index; }
        /* Returns whether or not the compute queue is supported by this device. */
        inline bool can_memory() const { return this->supports_memory; }

        /* Returns an Array with all the queue indices. The order is compute. */
        inline Tools::Array<uint32_t> queues() const { return this->compute_index != this->memory_index ? Tools::Array<uint32_t>({ this->compute_index, this->memory_index }) : Tools::Array<uint32_t>({ this->compute_index }); }
        /* Returns the number of queues in the class. */
        inline uint32_t n_queues() const { return this->compute_index != this->memory_index ? 2 : 1; }

    };

    /* The GPU class, which is the main interface to our Vulkan compute library implementation. */
    class GPU {
    private:
        /* The VkInstance that we use, among other things. */
        VkInstance vk_instance;

        /* The debug messenger of Vulkan. */
        VkDebugUtilsMessengerEXT vk_debugger;
        /* The function needed to destroy the Vulkan debug messenger. */
        PFN_vkDestroyDebugUtilsMessengerEXT vk_destroy_debug_utils_messenger_method;

        /* The physical GPU this class references. */
        VkPhysicalDevice vk_physical_device;
        /* The properties of said device, like its name. */
        VkPhysicalDeviceProperties vk_physical_device_properties;
        /* The queue information for this device. */
        DeviceQueueInfo vk_physical_device_queue_info;

        /* The logical device this class references. */
        VkDevice vk_device;
        /* The compute queue on the device. */
        VkQueue vk_compute_queue;
        /* The memory queue on the device. Might be the same. */
        VkQueue vk_memory_queue;

    public:
        /* Constructor for the GPU class, which takes a list of required extensions to enable on the GPU and optionally a list of validation layers to add. */
        GPU(const Tools::Array<const char*>& instance_extensions = Tools::Array<const char*>(), const Tools::Array<const char*>& device_extensions = Tools::Array<const char*>(), const Tools::Array<const char*>& required_layers = Tools::Array<const char*>());
        /* Copy constructor for the GPU class. However, this is deleted, as it makes no sense to copy a GPU. */
        GPU(const GPU& other) = delete;
        /* Move constructor for the GPU class. */
        GPU(GPU&& other);
        /* Destructor for the GPU class. */
        ~GPU();

        

        /* Explicitly provides (read-only) access to the internal vk_instance object. */
        inline VkInstance instance() const { return this->vk_instance; }
        /* Implicitly provides (read-only) access to the internal vk_instance object. */
        inline operator VkInstance() const { return this->vk_instance; }

        /* Returns the name of the chosen GPU. */
        inline std::string name() const { return std::string(this->vk_physical_device_properties.deviceName); }
        /* Returns the queue information of the chosen GPU. */
        inline DeviceQueueInfo queue_info() const { return this->vk_physical_device_queue_info; }
        /* Explicitly provides (read-only) access to the internal vk_physical_device object. */
        inline VkPhysicalDevice physical_device() const { return this->vk_physical_device; }
        /* Implicitly provides (read-only) access to the internal vk_physical_device object. */
        inline operator VkPhysicalDevice() const { return this->vk_physical_device; }

        /* Explicitly provides (read-only) access to the internal vk_device object. */
        inline VkDevice device() const { return this->vk_device; }
        /* Implicitly provides (read-only) access to the internal vk_device object. */
        inline operator VkDevice() const { return this->vk_device; }

        /* Explicitly provides (read-only) access to the internal vk_compute_queue object. */
        inline VkQueue compute_queue() const { return this->vk_compute_queue; }
        /* Explicitly provides (read-only) access to the internal vk_memory_queue object. */
        inline VkQueue memory_queue() const { return this->vk_memory_queue; }

        /* Copy assignment operator for the GPU class, which is also deleted. */
        GPU& operator=(const GPU& other) = delete;
        /* Move assignment operator for the GPU class. */
        GPU& operator=(GPU&& other);
        /* Swap operator for the GPU class. */
        friend void swap(GPU& g1, GPU& g2);
  
    };

    /* Swap operator for the GPU class. */
    void swap(GPU& g1, GPU& g2);
}

#endif
