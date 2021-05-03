/* RENDERER.hpp
 *   by Lut99
 *
 * Created:
 *   30/04/2021, 13:21:29
 * Last edited:
 *   03/05/2021, 15:50:58
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Implements the rendering process. More accurately, does two things:
 *   first, it takes a list of RenderEntities and returns a list of
 *   vertices, normals and colours that can be rendered. Then, it takes
 *   those lists and utiliized RayTracing to render those vertices to an
 *   image.
**/

#ifndef RENDERER_RENDERER_HPP
#define RENDERER_RENDERER_HPP

#include "glm/glm.hpp"

#include "camera/Camera.hpp"

#include "entities/RenderEntity.hpp"

#include "tools/Array.hpp"

#include "Vertex.hpp"

namespace RayTracer {
    /* The Renderer baseclass, which can be used to render a list of RenderEntities to a frame. Derived classes can determine if the renderer uses Vulkan, CUDA, the CPU, w/e. */
    class Renderer {
    protected:
        /* The pre-rendered list of (CPU-optimised) vertices, which we can send to the GPU. */
        Tools::Array<CVertex> entity_cvertices;
        /* The pre-rendered list of (CPU-optimised) points referred to by the vertices, which we can send to the GPU. */
        Tools::Array<glm::vec3> entity_cpoints;
        /* The pre-rendered list of (GPU-optimised) vertices, which we can send to the GPU. */
        Tools::Array<GVertex> entity_gvertices;
        /* The pre-rendered list of (GPU-optimised) points referred to by the vertices, which we can send to the GPU. */
        Tools::Array<glm::vec4> entity_gpoints;
        
        /* Protected constructor for the Renderer Baseclass, which is used by derived classes to initialize the base elements. */
        Renderer();

        /* Given a list of vertices pre-rendered from an entity, injects them into the list of points and indexed list of CVertices. */
        static void insert_cvertices(Tools::Array<CVertex>& gvertices, Tools::Array<glm::vec3>& points, const Array<Vertex>& vertices);
        /* Given a list of vertices pre-rendered from an entity, injects them into the list of points and indexed list of GVertices. */
        static void insert_gvertices(Tools::Array<GVertex>& gvertices, Tools::Array<glm::vec4>& points, const Array<Vertex>& vertices);

    public:
        /* Copy constructor for the Renderer baseclass. */
        Renderer(const Renderer& other);
        /* Move constructor for the Renderer baseclass. */
        Renderer(Renderer&& other);
        /* Virtual destructor for the Renderer baseclass. */
        virtual ~Renderer();

        /* Uses the chosen backend to perform pre-rendering. The resulting list of vertices, indices, w/e is stored internally, ready to be used by render(). */
        virtual void prerender(const Tools::Array<ECS::RenderEntity*>& entities) = 0;
        /* Uses the chosen backend to render the internal list of vertices, indices, w/e using the given Camera object. The resulting frame can then be retrieved from the Camera. */
        virtual void render(Camera& camera) const = 0;

        /* Swap operator for the Renderer baseclass. */
        friend void swap(Renderer& r1, Renderer& r2);

    };

    /* Swap operator for the Renderer baseclass. */
    void swap(Renderer& r1, Renderer& r2);



    /* Factory method for the Renderer class. Is implemented by the chosen backend, to avoid changing the frontend too. */
    extern Renderer* initialize_renderer();
}

#endif
