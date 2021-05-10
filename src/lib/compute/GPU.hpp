/* GPU.hpp
 *   by Lut99
 *
 * Created:
 *   16/04/2021, 17:21:54
 * Last edited:
 *   10/05/2021, 14:21:06
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

#include "Instance.hpp"

#include "tools/Array.hpp"

namespace RayTracer::Compute {
    /* The Vulkan device extensions we want to be enabled. */
    const Tools::Array<const char*> device_extensions({
        // Nothing lmao
    });



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

        /* The index of the presentation family queue on the device. */
        uint32_t presentation_index;
        /* Boolean that keeps track of whether or not the memory queue is supported. */
        bool supports_presentation;

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

        /* Returns the queue index of the GPU's presentation queue. Note that the value returned by this is undefined if can_present == false. Also not that this may be the same as any of the other queues. */
        inline uint32_t presentation() const { return this->presentation_index; }
        /* Returns whether or not the presentation queue is supported by this device. */
        inline bool can_present() const { return this->supports_presentation; }
        /* Given a vulkan physical device and a surface, checks the device's capabilities of presenting to that surface. Updates the internal presentation index if it's found a queue. */
        void check_present(VkPhysicalDevice vk_physical_device, VkSurfaceKHR vk_surface);

        /* Returns an Array with all the queue indices. The order is compute. */
        Tools::Array<uint32_t> queues() const;
        /* Returns the number of queues in the class. */
        inline uint32_t n_queues() const { return this->compute_index == this->memory_index && this->compute_index == this->memory_index ? 1 : (this->compute_index == this->memory_index || this->compute_index == this->memory_index ? 2 : 3); }

    };



    /* The SwapchainInfo class, which describes the kind of swapchains the GPU supports. */
    class SwapchainInfo {
    private:
        /* The capabilities of this device for the given surface. */
        VkSurfaceCapabilitiesKHR vk_capabilities;
        /* The formats supported by this device for this surface. */
        Tools::Array<VkSurfaceFormatKHR> vk_formats;
        /* The present modes supported by this device for this surface. */
        Tools::Array<VkPresentModeKHR> vk_present_modes;

    public:
        /* Default constructor for the SwapchainInfo class, which initializes this to "nothing supported". */
        SwapchainInfo();
        /* Constructor for the SwapchainInfo class, which takes a VkPhysicalDevice and a VkSurfaceKHR to populate itself appropriately. */
        SwapchainInfo(VkPhysicalDevice vk_physical_device, VkSurfaceKHR vk_surface);

        /* Returns the capabilities of this device as a constant reference. */
        inline const VkSurfaceCapabilitiesKHR& capabilities() const { return this->vk_capabilities; }
        /* Returns a constant reference to the list of surface formats. */
        inline const Tools::Array<VkSurfaceFormatKHR>& formats() const { return this->vk_formats; }
        /* Returns a constant reference to the list of present modes. */
        inline const Tools::Array<VkPresentModeKHR>& present_modes() const { return this->vk_present_modes; }

    };



    /* The GPU class, which is the main interface to our Vulkan compute library implementation. */
    class GPU {
    public:
        /* Constant reference to the instance where this GPU is declared with. */
        const Instance& instance;

    private:
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
        /* The presentation queue on the device. Might be the same. */
        VkQueue vk_presentation_queue;
        
        /* The swapchain support of this device. */
        SwapchainInfo vk_swapchain_info;

        /* The extensions enabled on the device. */
        Tools::Array<const char*> vk_extensions;

    public:
        /* Constructor for the GPU class, which takes a list of required extensions to enable on the GPU. */
        GPU(const Instance& instance, const Tools::Array<const char*>& extensions = device_extensions);
        /* Copy constructor for the GPU class. */
        GPU(const GPU& other);
        /* Move constructor for the GPU class. */
        GPU(GPU&& other);
        /* Destructor for the GPU class. */
        ~GPU();

        /* Allows the GPU to be compared with another GPU class. */
        inline bool operator==(const GPU& other) const { return this->vk_device == other.vk_device; }
        /* Allows the GPU to be (negated) compared with another GPU class. */
        inline bool operator!=(const GPU& other) const { return this->vk_device != other.vk_device; }

        /* Updates the internal queue info on whether or not the GPU can present to the given surface. */
       void check_present(VkSurfaceKHR vk_surface);

        /* Returns the name of the chosen GPU. */
        inline std::string name() const { return std::string(this->vk_physical_device_properties.deviceName); }
        /* Returns the queue information of the chosen GPU. */
        inline DeviceQueueInfo queue_info() const { return this->vk_physical_device_queue_info; }
        /* Returns the swapchain information of the chosen GPU. */
        inline SwapchainInfo swapchain_info() const { return this->vk_swapchain_info; }
        
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
        /* Explicitly provides (read-only) access to the internal vk_memory_queue object. */
        inline VkQueue present_queue() const { return this->vk_presentation_queue; }

        /* Copy assignment operator for the GPU class. */
        inline GPU& operator=(const GPU& other) { return *this = GPU(other); }
        /* Move assignment operator for the GPU class. */
        inline GPU& operator=(GPU&& other) { if (this != &other) { swap(*this, other); } return *this; }
        /* Swap operator for the GPU class. */
        friend void swap(GPU& g1, GPU& g2);
  
    };

    /* Swap operator for the GPU class. */
    void swap(GPU& g1, GPU& g2);
}

#endif
