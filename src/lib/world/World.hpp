/* WORLD.hpp
 *   by Lut99
 *
 * Created:
 *   29/04/2021, 17:04:56
 * Last edited:
 *   29/04/2021, 17:57:03
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Contains the World class, which describes and manages all objects,
 *   lights, w/e in the to-be-rendered 3D world. Does not manage cameras,
 *   though.
**/

#ifndef WORLD_WORLD_HPP
#define WORLD_WORLD_HPP

#include "compute/GPU.hpp"
#include "compute/MemoryPool.hpp"
#include "compute/DescriptorPool.hpp"
#include "compute/DescriptorSetLayout.hpp"
#include "compute/CommandPool.hpp"

#include "tools/Array.hpp"

#include "RenderObject.hpp"

namespace RayTracer {
    /* The World class, which manages all RenderObjects and handles their data transfers to the GPU. */
    class World {
    public:
        /* Constant reference to the GPU where this camera renders on. */
        const Compute::GPU& gpu;

    private:
        /* Reference to the memory pool where we allocate buffers from. */
        Compute::MemoryPool* pool;

        /* Command buffer used to move data around. */
        Compute::CommandBuffer cmd_buffer;
        /* CPU-side buffer used to describe all RenderObjects. */
        Tools::Array<RenderObjectData> cpu_object_buffer;
        /* CPU-side buffer used to describe el vertices. */
        Tools::Array<glm::vec4> cpu_vertex_buffer;
        /* CPU-side buffer used to describe el normals. */
        Tools::Array<glm::vec4> cpu_normal_buffer;
        /* CPU-side buffer used to describe el colors. */
        Tools::Array<glm::vec4> cpu_color_buffer;
        /* Handle to the GPU-side buffer used to describe all RenderObjects. */
        Compute::BufferHandle gpu_object_buffer;
        /* Handle to the GPU-side buffer used to describe el vertices. */
        Compute::BufferHandle gpu_vertex_buffer;
        /* Handle to the GPU-side buffer used to describe el normals. */
        Compute::BufferHandle gpu_normal_buffer;
        /* Handle to the GPU-side buffer used to describe el colors. */
        Compute::BufferHandle gpu_color_buffer;

        /* Temporary storage for the binding index of the object buffer. */
        uint32_t object_bind_index;
        /* Temporary storage for the binding index of the vertex buffer. */
        uint32_t vertex_bind_index;
        /* Temporary storage for the binding index of the normal buffer. */
        uint32_t normal_bind_index;
        /* Temporary storage for the binding index of the color buffer. */
        uint32_t color_bind_index;

        /* List of objects registered in this world class. */
        Tools::Array<RenderObject*> objects;
    
    public:
        /* Constructor for the World class, which takes a GPU used to render all objects, a (device-local) memory pool used to allocate GPU buffers and a CommandPool used to allocate a buffer for memory transfers. */
        World(const Compute::GPU& gpu, Compute::MemoryPool& mpool, Compute::CommandPool& cpool, size_t init_n_vertices = 4096);
        /* Copy constructor for the World class. */
        World(const World& other);
        /* Move constructor for the World class. */
        World(World&& other);
        /* Destructor for the World class. */
        ~World();

        /* Adds a given object to the World, rendering it when the time comes. The world also manages it in terms of cleaning memory 'n' stuff. */
        void add(RenderObject* object);

        /* Creates new bindings for all the internal buffers. */
        void set_layout(Compute::DescriptorSetLayout& descriptor_set_layout);
        /* Writes buffers to the bindings set in set_layout. Will throw errors if no binding has been set first. */
        void set_bindings(Compute::DescriptorSet& descriptor_set) const;

        /* Updates all objects in this World. */
        void update();
        /* Draws all objects by rendering them to vertices and storing them internally. Will not yet sync with the GPU. */
        void draw();
        
        /* Syncs the World object with the GPU, readying the data there for rendering. The given memory pool is used to allocate a staging buffer. */
        void sync(const Compute::MemoryPool& mpool);

        /* Reserves space for the given number of objects in the World class. New objects will be default initialized. */
        void resize(size_t new_size);

        /* Returns the current number of vertices in the World class. */
        inline size_t size() const { return this->cpu_normal_buffer.size(); }
        /* Returns the maximum number of vertices in the World class we currently have memory reserved for. */
        inline size_t capacity() const { return this->cpu_normal_buffer.capacity(); }

        /* Copy assignment operator for the World class. */
        inline World& operator=(const World& other) { return *this = World(other); }
        /* Move assignment operator for the World class. */
        inline World& operator=(World&& other) { if (this != &other) { swap(*this, other); } return *this; }
        /* Swap operator for the World class. */
        friend void swap(World& w1, World& w2);

    };

    /* Swap operator for the World class. */
    void swap(World& w1, World& w2);
}

#endif
