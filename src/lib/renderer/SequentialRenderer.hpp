/* SEQUENTIAL RENDERER.hpp
 *   by Lut99
 *
 * Created:
 *   03/05/2021, 15:25:09
 * Last edited:
 *   03/05/2021, 15:34:44
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Simplest implementation of the Renderer class, which just renders a
 *   frame sequentially on the CPU, no fancy strings attached.
**/

#ifndef RENDERER_SEQUENTIAL_RENDERER_HPP
#define RENDERER_SEQUENTIAL_RENDERER_HPP

#include "glm/glm.hpp"

#include "Vertex.hpp"
#include "Renderer.hpp"

namespace RayTracer {
    /* The SequentialRenderer class, which implements the standard Renderer as simple as possible. */
    class SequentialRenderer: public Renderer {
    private:

    public:
        /* Constructor for the SequentialRenderer class. */
        SequentialRenderer();
        
        /* Pre-renders the given list of RenderEntities, accelerated using Vulkan compute shaders. */
        virtual void prerender(const Tools::Array<ECS::RenderEntity*>& entities);
        /* Renders the internal list of vertices to a frame using the given camera position. */
        virtual void render(Camera& camera) const;

    };
}

#endif
