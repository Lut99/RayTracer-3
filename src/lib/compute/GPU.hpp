/* GPU.hpp
 *   by Lut99
 *
 * Created:
 *   16/04/2021, 17:21:54
 * Last edited:
 *   03/05/2021, 13:35:56
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
