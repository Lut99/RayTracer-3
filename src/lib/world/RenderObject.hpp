/* RENDER OBJECT.hpp
 *   by Lut99
 *
 * Created:
 *   29/04/2021, 15:51:24
 * Last edited:
 *   29/04/2021, 17:02:50
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Contains the RenderObject class, which is the baseclass for all
 *   objects rendered with the RayTracer.
**/

#ifndef WORLD_RENDER_OBJECT_HPP
#define WORLD_RENDER_OBJECT_HPP

#include <cstdint>

#include "glm/glm.hpp"

namespace RayTracer {
    /* The maximum number of vertices allowed per object. */
    const constexpr uint32_t max_object_vertices = 4096;
    /* The maximum number of vertices allowed in the world. */
    const constexpr uint32_t max_world_vertices = 4096000;



    /* The RenderObjectData struct, which describes what data is send to the GPU and how. */
    struct RenderObjectData {
        /* The vectors describing the bounding box for this RenderObject. */
        glm::vec4 aabb[2];
        /* The number of indices to the vertex, normal and color buffers used to describe this RenderObject. */
        uint32_t n_vertices;
        /* The indices to the vertex, normal and color buffers, which describe which vertices are part of this object. */
        uint32_t indices[max_object_vertices];

    };



    /* The RenderObject class, which forms the baseclass for everything rendered with the RayTracer. */
    class RenderObject {
    private:
        /* Local cache for the object's vertices. Interesting if they are known beforehand, to save compute time in between updates. */
        glm::vec4* cvertices;
        /* Local cache for the object's normals. Interesting if they are known beforehand, to save compute time in between updates. */
        glm::vec4* cnormals;
        /* Local cache for the object's colors. Interesting if they are known beforehand, to save compute time in between updates. */
        glm::vec4* ccolors;
        /* The number of vertices described in the previous arrays. */
        uint32_t n_cached_vertices;
        /* Marks if the caches are valid or not (i.e., an update() has been called). */
        bool cache_valid;

    protected:
        /* Protected constructor of the RenderObject class. */
        RenderObject(uint32_t n_cached_vertices = 1024);

        /* The actual update function, defined by the derived classes. */
        virtual void __update() = 0;
        /* The actual draw function, which generates the required vertices ed in the given buffers. The buffers are guaranteed to support at least n_cached_vertices vertices. */
        virtual void __draw(glm::vec4* vertices, glm::vec4* normals, glm::vec4* colors) const = 0;

    public:
        /* Copy constructor for the RenderObject class. */
        RenderObject(const RenderObject& other);
        /* Move constructor for the RenderObject class. */
        RenderObject(RenderObject&& other);
        /* (Virtual) destructor for the RenderObject class. */
        virtual ~RenderObject();

        /* Updates the render object, i.e., lets the derived class update itself accordingly. */
        void update();
        /* Draws the RenderObjectData, but stores it in internal caches instead of pushing it to the global array. */
        void draw_cached();
        /* Draws the RenderObjectData to the given vertex, normal and color buffers. Also computes the indices and stores them in the returned RenderObjectData. If cached is true, then the internal caches are used to possibly accelerate the draw. */
        void draw(RenderObjectData* object_data, uint32_t& n_vertices, glm::vec4* vertices, glm::vec4* normals, glm::vec4* colors, bool cached = true) const;

        /* Polymorphic copying function for the RenderObject class. */
        virtual RenderObject* copy() const = 0;
        /* Swap operator for the RenderObject class (useful for abstracting it away). */
        friend void swap(RenderObject& ro1, RenderObject& ro2);

    };

    /* Swap operator for the RenderObject class (useful for abstracting it away). */
    void swap(RenderObject& ro1, RenderObject& ro2);

}

#endif
