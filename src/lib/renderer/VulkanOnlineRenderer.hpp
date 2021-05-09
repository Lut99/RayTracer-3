/* VULKAN ONLINE RENDERER.hpp
 *   by Lut99
 *
 * Created:
 *   09/05/2021, 18:30:37
 * Last edited:
 *   09/05/2021, 18:34:27
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Derived class of the VulkanRenderer class, which renders to a
 *   swapchain in real-time instead of to images. Therefore, the call to
 *   this render is blocking, returning an empty frame once the user closes
 *   the window.
**/

#ifndef RENDERER_VULKAN_ONLINE_RENDERER_HPP
#define RENDERER_VULKAN_ONLINE_RENDERER_HPP

#include "glm/glm.hpp"

#include "VulkanRenderer.hpp"

namespace RayTracer {
    /* The VulkanOnlineRenderer class, which renders to a window in real-time instead of a picture. */
    class VulkanOnlineRenderer: public VulkanRenderer {
    private:
        

    public:
        /* Constructor for the VulkanOnlineRenderer class, which takes nothing to be compatible. */
        VulkanOnlineRenderer();
        /* Copy constructor for the VulkanOnlineRenderer class. */
        VulkanOnlineRenderer(const VulkanOnlineRenderer& other);
        /* Move constructor for the VulkanOnlineRenderer class. */
        VulkanOnlineRenderer(VulkanOnlineRenderer&& other);
        /* Destructor for the VulkanOnlineRenderer class. */
        virtual ~VulkanOnlineRenderer();

        /* Renders the internal list of vertices to a window using the given camera position. Renders the entire simulation, including update steps, and returns a blackened frame since the output is unreachable and simultaneously not interesting. */
        virtual void render(Camera& camera) const;

        /* Copy assignment operator for the VulkanOnlineRenderer class. */
        virtual VulkanOnlineRenderer& operator=(const VulkanOnlineRenderer& other) { return *this = VulkanOnlineRenderer(other); }
        /* Move assignment operator for the VulkanOnlineRenderer class. */
        virtual VulkanOnlineRenderer& operator=(VulkanOnlineRenderer&& other) { if (this != &other) { swap(*this, other); } return *this; }
        /* Swap operator for the VulkanOnlineRenderer class. */
        friend void swap(VulkanOnlineRenderer& r1, VulkanOnlineRenderer& r2);

    };

    /* Swap operator for the VulkanOnlineRenderer class. */
    void swap(VulkanOnlineRenderer& r1, VulkanOnlineRenderer& r2);
}

#endif
