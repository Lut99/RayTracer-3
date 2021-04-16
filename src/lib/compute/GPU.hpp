/* GPU.hpp
 *   by Lut99
 *
 * Created:
 *   16/04/2021, 17:21:54
 * Last edited:
 *   16/04/2021, 19:52:26
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
    /* The GPU class, which is the main interface to our Vulkan compute library implementation. */
    class GPU {
    private:
        /* The VkInstance that we use, among other things. */
        VkInstance vk_instance;

    public:
        /* Constructor for the GPU class, which takes a list of required extensions to enable on the GPU and optionally a list of validation layers to add. */
        GPU(const Tools::Array<const char*>& required_extensions, const Tools::Array<const char*>& required_layers = Tools::Array<const char*>());
        /* Copy constructor for the GPU class. However, this is deleted, as it makes no sense to copy a GPU. */
        GPU(const GPU& other) = delete;
        /* Move constructor for the GPU class. */
        GPU(GPU&& other);
        /* Destructor for the GPU class. */
        ~GPU();



        /* Explicitly provides (read-only) access to the intern vk_instance object. */
        inline VkInstance instance() const { return this->vk_instance; }
        /* Implicitly provides (read-only) access to the intern vk_instance object. */
        inline operator VkInstance() const { return this->vk_instance; }

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
