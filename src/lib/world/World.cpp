/* WORLD.cpp
 *   by Lut99
 *
 * Created:
 *   29/04/2021, 17:04:50
 * Last edited:
 *   29/04/2021, 17:57:40
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Contains the World class, which describes and manages all objects,
 *   lights, w/e in the to-be-rendered 3D world. Does not manage cameras,
 *   though.
**/

#include <limits>
#include <CppDebugger.hpp>

#include "compute/ErrorCodes.hpp"

#include "World.hpp"

using namespace std;
using namespace RayTracer;
using namespace RayTracer::Compute;
using namespace CppDebugger::SeverityValues;


/***** WORLD CLASS (get it?) *****/
/* Constructor for the World class, which takes a GPU used to render all objects, a (device-local) memory pool used to allocate GPU buffers and a CommandPool used to allocate a buffer for memory transfers. */
World::World(const Compute::GPU& gpu, Compute::MemoryPool& mpool, Compute::CommandPool& cpool, size_t init_n_vertices) :
    gpu(gpu),
    pool(&mpool),
    cmd_buffer(cpool[cpool.allocate()]),
    gpu_object_buffer(MemoryPool::NullHandle),
    gpu_vertex_buffer(MemoryPool::NullHandle),
    gpu_normal_buffer(MemoryPool::NullHandle),
    gpu_color_buffer(MemoryPool::NullHandle),
    object_bind_index(numeric_limits<uint32_t>::max()),
    vertex_bind_index(numeric_limits<uint32_t>::max()),
    normal_bind_index(numeric_limits<uint32_t>::max()),
    color_bind_index(numeric_limits<uint32_t>::max())
{
    DENTER("World::World");

    // Reserve enough space in the CPU buffers
    this->cpu_vertex_buffer.resize(init_n_vertices);
    this->cpu_normal_buffer.resize(init_n_vertices);
    this->cpu_color_buffer.resize(init_n_vertices);

    // Don't reserve the GPU buffers yet, as we keep that until draw() is called

    // Also don't touch the object array so far

    // Done!
    DLEAVE;
}

/* Copy constructor for the World class. */
World::World(const World& other) :
    gpu(other.gpu),
    pool(other.pool),
    cmd_buffer(other.cmd_buffer),
    cpu_object_buffer(other.cpu_object_buffer),
    cpu_vertex_buffer(other.cpu_vertex_buffer),
    cpu_normal_buffer(other.cpu_normal_buffer),
    cpu_color_buffer(other.cpu_color_buffer),
    gpu_object_buffer(other.gpu_object_buffer),
    gpu_vertex_buffer(other.gpu_vertex_buffer),
    gpu_normal_buffer(other.gpu_normal_buffer),
    gpu_color_buffer(other.gpu_color_buffer),
    object_bind_index(other.object_bind_index),
    vertex_bind_index(other.vertex_bind_index),
    normal_bind_index(other.normal_bind_index),
    color_bind_index(other.color_bind_index)
{
    DENTER("World::World(copy)");

    // We first allocate nwe GPU buffers
    if (other.gpu_object_buffer != MemoryPool::NullHandle) {
        this->gpu_object_buffer = this->pool->allocate((*(other.pool))[other.gpu_object_buffer].size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    }
    if (other.gpu_vertex_buffer != MemoryPool::NullHandle) {
        this->gpu_vertex_buffer = this->pool->allocate((*(other.pool))[other.gpu_vertex_buffer].size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    }
    if (other.gpu_normal_buffer != MemoryPool::NullHandle) {
        this->gpu_normal_buffer = this->pool->allocate((*(other.pool))[other.gpu_normal_buffer].size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    }
    if (other.gpu_color_buffer != MemoryPool::NullHandle) {
        this->gpu_color_buffer = this->pool->allocate((*(other.pool))[other.gpu_color_buffer].size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    }

    // Next, we copy all elements from their old buffers to the new
    // Note that we only wait for the queue to become idle in the last one, to overlap scheduling and copying
    MemoryPool& pool = *(this->pool);
    if (this->gpu_object_buffer != MemoryPool::NullHandle) {
        pool[other.gpu_object_buffer].copyto(this->gpu, this->cmd_buffer, pool[this->gpu_object_buffer], false);
    }
    if (this->gpu_vertex_buffer != MemoryPool::NullHandle) {
        pool[other.gpu_vertex_buffer].copyto(this->gpu, this->cmd_buffer, pool[this->gpu_vertex_buffer], false);
    }
    if (this->gpu_normal_buffer != MemoryPool::NullHandle) {
        pool[other.gpu_normal_buffer].copyto(this->gpu, this->cmd_buffer, pool[this->gpu_normal_buffer], false);
    }
    if (this->gpu_color_buffer != MemoryPool::NullHandle) {
        pool[other.gpu_color_buffer].copyto(this->gpu, this->cmd_buffer, pool[this->gpu_color_buffer], false);
    }

    // Wait until all copy operations are ready
    VkResult vk_result;
    if ((vk_result = vkQueueWaitIdle(this->gpu.memory_queue())) != VK_SUCCESS) {
        DLOG(fatal, "Could not wait for memory queue: " + vk_error_map[vk_result]);
    }



    // Before we quit, don't forget to copy all objects
    this->objects.reserve(other.objects.capacity());
    for (size_t i = 0; i < other.objects.size(); i++) {
        this->objects.push_back(other.objects[i]->copy());
    }

    // Done!
    DLEAVE;
}

/* Move constructor for the World class. */
World::World(World&& other) :
    gpu(other.gpu),
    pool(other.pool),
    cmd_buffer(other.cmd_buffer),
    cpu_object_buffer(other.cpu_object_buffer),
    cpu_vertex_buffer(other.cpu_vertex_buffer),
    cpu_normal_buffer(other.cpu_normal_buffer),
    cpu_color_buffer(other.cpu_color_buffer),
    gpu_object_buffer(other.gpu_object_buffer),
    gpu_vertex_buffer(other.gpu_vertex_buffer),
    gpu_normal_buffer(other.gpu_normal_buffer),
    gpu_color_buffer(other.gpu_color_buffer),
    object_bind_index(other.object_bind_index),
    vertex_bind_index(other.vertex_bind_index),
    normal_bind_index(other.normal_bind_index),
    color_bind_index(other.color_bind_index),
    objects(other.objects)
{
    // Set all deallocateable memory to nullptrs etc to avoid them being deallocated
    this->gpu_object_buffer = MemoryPool::NullHandle;
    this->gpu_vertex_buffer = MemoryPool::NullHandle;
    this->gpu_normal_buffer = MemoryPool::NullHandle;
    this->gpu_color_buffer = MemoryPool::NullHandle;
    this->objects.clear();
}

/* Destructor for the World class. */
World::~World() {
    DENTER("World::~World");
    DLOG(info, "Cleaning World...");
    DINDENT;



    if (this->gpu_object_buffer != MemoryPool::NullHandle || this->gpu_vertex_buffer != MemoryPool::NullHandle || this->gpu_normal_buffer != MemoryPool::NullHandle || this->gpu_color_buffer != MemoryPool::NullHandle) {
        DLOG(info, "Cleaning GPU-side buffers...");
    }
    if (this->gpu_object_buffer != MemoryPool::NullHandle) {
        this->pool->deallocate(this->gpu_object_buffer);
    }
    if (this->gpu_vertex_buffer != MemoryPool::NullHandle) {
        this->pool->deallocate(this->gpu_vertex_buffer);
    }
    if (this->gpu_normal_buffer != MemoryPool::NullHandle) {
        this->pool->deallocate(this->gpu_normal_buffer);
    }
    if (this->gpu_color_buffer != MemoryPool::NullHandle) {
        this->pool->deallocate(this->gpu_color_buffer);
    }



    if (this->objects.size() > 0) {
        DLOG(info, "Cleaning objects...");
        for (size_t i = 0; i < this->objects.size(); i++) {
            // Delete it first
            delete this->objects[i];
        }
    }

    

    DDEDENT;
    DLEAVE;
}



/* Adds a given object to the World, rendering it when the time comes. The world also manages it in terms of cleaning memory 'n' stuff. */
void World::add(RenderObject* object) {
    DENTER("World::add");

    

    DRETURN;
}



/* Creates new bindings for all the internal buffers. */
void World::set_layout(Compute::DescriptorSetLayout& descriptor_set_layout) {
    DENTER("World::set_layout");

    

    DRETURN;
}

/* Writes buffers to the bindings set in set_layout. Will throw errors if no binding has been set first. */
void World::set_bindings(Compute::DescriptorSet& descriptor_set) const {
    DENTER("World::set_bindings");

    

    DRETURN;
}



/* Updates all objects in this World. */
void World::update() {
    DENTER("World::update");

    

    DRETURN;
}

/* Draws all objects by rendering them to vertices and storing them internally. Will not yet sync with the GPU. */
void World::draw() {
    DENTER("World::draw");

    

    DRETURN;
}



/* Syncs the World object with the GPU, readying the data there for rendering. The given memory pool is used to allocate a staging buffer. */
void World::sync(const Compute::MemoryPool& mpool) {
    DENTER("World::sync");

    

    DRETURN;
}



/* Reserves space for the given number of objects in the World class. New objects will be default initialized. */
void World::resize(size_t new_size) {
    DENTER("World::resize");

    // Simply reserve all vertice-related arrays
    this->cpu_vertex_buffer.resize(new_size);
    this->cpu_normal_buffer.resize(new_size);
    this->cpu_color_buffer.resize(new_size);

    // Done
    DLEAVE;
}



/* Swap operator for the World class. */
void RayTracer::swap(World& w1, World& w2) {
    DENTER("RayTracer::swap(World)");

    // Make sure the GPUs overlap
    #ifndef NDEBUG
    // If the GPU is not the same, then initialize to all nullptrs and everything
    if (w1.gpu != w2.gpu) {
        DLOG(fatal, "Cannot swap worlds with different GPUs");
    }
    #endif

    // Swap all elements
    using std::swap;
    swap(w1.pool, w2.pool);
    swap(w1.cmd_buffer, w2.cmd_buffer);
    swap(w1.cpu_object_buffer, w2.cpu_object_buffer);
    swap(w1.cpu_vertex_buffer, w2.cpu_vertex_buffer);
    swap(w1.cpu_normal_buffer, w2.cpu_normal_buffer);
    swap(w1.cpu_color_buffer, w2.cpu_color_buffer);
    swap(w1.gpu_object_buffer, w2.gpu_object_buffer);
    swap(w1.gpu_vertex_buffer, w2.gpu_vertex_buffer);
    swap(w1.gpu_normal_buffer, w2.gpu_normal_buffer);
    swap(w1.gpu_color_buffer, w2.gpu_color_buffer);
    swap(w1.object_bind_index, w2.object_bind_index);
    swap(w1.vertex_bind_index, w2.vertex_bind_index);
    swap(w1.normal_bind_index, w2.normal_bind_index);
    swap(w1.color_bind_index, w2.color_bind_index);
    swap(w1.objects, w2.objects);

    // Done
    DRETURN;
}
