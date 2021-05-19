/* SEQUENTIAL RENDERER.hpp
 *   by Lut99
 *
 * Created:
 *   03/05/2021, 15:25:09
 * Last edited:
 *   19/05/2021, 17:52:34
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
        /* The pre-rendered list of (GPU-optimised) vertices, which we can send to the GPU. */
        Tools::Array<GFace> entity_faces;
        /* The pre-rendered list of (GPU-optimised) points referred to by the vertices, which we can send to the GPU. */
        Tools::Array<glm::vec4> entity_vertices;
        
        /* Helper function that merges the given newly pre-rendered faces & vertex buffers and inserts them in the global buffers. */
        void transfer_entity(Tools::Array<GFace>& faces_buffer, Tools::Array<glm::vec4>& vertex_buffer, const Tools::Array<GFace>& new_faces_buffer, const Tools::Array<glm::vec4>& new_vertex_buffer);

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
