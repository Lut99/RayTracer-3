/* SWAPCHAIN.hpp
 *   by Lut99
 *
 * Created:
 *   09/05/2021, 18:40:10
 * Last edited:
 *   21/05/2021, 17:59:59
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

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "GPU.hpp"

namespace RayTracer::Compute {
    /* The Swapchain class, which wraps and manages the swapchain and all images related to it. */
    class Swapchain {
    public:
        /* Immutable reference to the GPU where we got the swapchain from. */
        const GPU& gpu;

    private:
        /* The internal VkSwapchainKHR object that we wrap. */
        VkSwapchainKHR vk_swapchain;

        /* The surface to which this swapchain is bound. */
        VkSurfaceKHR vk_surface;
        /* The chosen format for this swapchain. */
        VkSurfaceFormatKHR vk_surface_format;
        /* The chosen presentation mode for this swapchain. */
        VkPresentModeKHR vk_surface_present_mode;
        /* The current size of the swapchain frames. */
        VkExtent2D vk_surface_extent;
        
        /* The number of images that are actually created in the swapchain. */
        uint32_t vk_actual_image_count;
        /* The number of images that we want to have in the swapchain. */
        uint32_t vk_desired_image_count;

        /* The images part of the swapchain. Will be variable in size. */
        Tools::Array<VkImage> vk_swapchain_images;
        // /* Images views to the images created with the swapchain. */
        // Tools::Array<VkImageView> vk_swapchain_views;

    public:
        /* Constructor for the Swapchain class, which takes the GPU where it will be constructed and the window to which it shall present. */
        Swapchain(const Compute::GPU& gpu, GLFWwindow* glfw_window, VkSurfaceKHR vk_surface);
        /* Copy constructor for the Swapchain class. */
        Swapchain(const Swapchain& other);
        /* Move constructor for the Swapchain class. */
        Swapchain(Swapchain&& other);
        /* Destructor for the Swapchain class. */
        ~Swapchain();

        /* Returns a the internal image at the given location. */
        inline VkImage operator[](uint32_t index) { return this->vk_swapchain_images[index]; }

        /* Returns the number of images in the swapchain. */
        inline uint32_t size() const { return this->vk_actual_image_count; }
        /* Returns the actual extent of the swapchain. */
        inline VkExtent2D extent() const { return this->vk_surface_extent; }
        /* Returns the format of the swapchain images. */
        inline VkFormat format() const { return this->vk_surface_format.format; }

        /* Expliticly returns the internal VkSwapchainKHR object. */
        inline const VkSwapchainKHR& swapchain() const { return this->vk_swapchain; }
        /* Implicitly returns the internal VkSwapchainKHR object. */
        inline operator VkSwapchainKHR() const { return this->vk_swapchain; }

        /* Copy assignment operator for the Swapchain class. */
        inline Swapchain& operator=(const Swapchain& other) { return *this = Swapchain(other); }
        /* Move assignment operator for the Swapchain class. */
        inline Swapchain& operator=(Swapchain&& other) { if (this != &other) { swap(*this, other); } return *this; }
        /* Swap operator for the Swapchain class. */
        friend void swap(Swapchain& s1, Swapchain& s2);

    };

    /* Swap operator for the Swapchain class. */
    void swap(Swapchain& s1, Swapchain& s2);
}

#endif
