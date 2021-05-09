/* SWAPCHAIN.hpp
 *   by Lut99
 *
 * Created:
 *   09/05/2021, 18:40:10
 * Last edited:
 *   09/05/2021, 18:41:50
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Contains the wrapper around Vulkan's swapchain. Not only does it
 *   manage those resources, but also all memory-related buffer and image
 *   management to make it easy to manage those.
**/

#ifndef COMPUTE_SWAPCHAIN_HPP
#define COMPUTE_SWAPCHAIN_HPP

#include <vulkan/vulkan.h>

#include "GPU.hpp"

namespace RayTracer::Compute {
    /* The Swapchain class, which wraps and manages the swapchain and all images related to it. */
    class Swapchain {
    public:
        /* Immutable reference to the GPU where we got the swapchain from. */
        const GPU& gpu;

    private:
        

    public:
        

    };
}

#endif
