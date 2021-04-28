/* FRAME.hpp
 *   by Lut99
 *
 * Created:
 *   28/04/2021, 14:28:35
 * Last edited:
 *   28/04/2021, 20:59:29
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Contains the Frame class, which is a wrapper around CPU & Vulkan
 *   buffers to provide a conceptually easy frame to fill with colour
 *   pixels on the GPU. Also contains code to easily write to PNG, using
 *   the LodePNG library.
**/

#ifndef FRAME_FRAME_HPP
#define FRAME_FRAME_HPP

#include "glm/glm.hpp"
#include "compute/GPU.hpp"
#include "compute/CommandPool.hpp"
#include "compute/MemoryPool.hpp"
#include "compute/DescriptorSetLayout.hpp"
#include "compute/DescriptorPool.hpp"

namespace RayTracer {
    /* The Frame class, which represents a single image to be rendered by the RayTracer. */
    class Frame {
    public:
        /* Constant reference to the GPU where this frame is rendered to. */
        const Compute::GPU& gpu;

        /* The datatype used to define the array. */
        using pixel_t = glm::vec4;

    private:
        /* Reference to the memory pool where we allocate buffers from. */
        Compute::MemoryPool* pool;

        /* The width, in pixels, of the frame. */
        uint32_t width;
        /* The height, in pixels, of the frame. */
        uint32_t height;

        /* Command buffer used for memory operations. */
        Compute::CommandBuffer cmd_buffer;
        /* The CPU-side buffer of the Frame. Is allocated the entire lifetime of the Frame. */
        pixel_t* cpu_buffer;
        /* The GPU-side buffer of the Frame. Is allocated the entire lifetime of the Frame. */
        Compute::BufferHandle gpu_buffer;

        /* Temporary storage for the binding set in set_layout and used in set_binding. */
        uint32_t bind_index;

    public:
        /* Constructor for the Frame class, which takes the GPU where the frame lives, a memory pool to allocate the GPU-side buffer from, a command pool to allocate a command buffer for memory operations from (the pool itself will not stay referenced) and the size of the image (in pixels). */
        Frame(const Compute::GPU& gpu, Compute::MemoryPool& mpool, Compute::CommandPool& cpool, uint32_t width, uint32_t height);
        /* Alternative constructor for the Frame, which takes the GPU where the frame lives, a memory pool to allocate the GPU-side buffer from, a command buffer for memory operations from and the size of the image (in pixels). */
        Frame(const Compute::GPU& gpu, Compute::MemoryPool& mpool, const Compute::CommandBuffer& cmd_buffer, uint32_t width, uint32_t height);
        /* Copy constructor for the Frame class. */
        Frame(const Frame& other);
        /* Move constructor for the Frame class. */
        Frame(Frame&& other);
        /* Desctructor for the Frame class. */
        ~Frame();

        /* Given a DescriptorSetLayout, adds a new binding for the storage buffer used internally in the Frame. */
        void set_layout(Compute::DescriptorSetLayout& descriptor_set_layout);
        /* Given a DescriptorSet, writes the internal buffer to it. */
        void set_binding(Compute::DescriptorSet& descriptor_set) const;

        /* Synchronizes the CPU buffer with the GPU buffer. Since we assume that only the GPU does writing, this means it's copied from GPU memory to CPU memory. The given memory pool is used to create a staging buffer for this event. */
        void sync(Compute::MemoryPool& staging_pool);
        
        /* Writes the internal frame to disk as a PNG. Assumes that the CPU buffer is synchronized with the GPU one. */
        void to_png(const std::string& path) const;
        /* Writes the internal frame to disk as a PPM. Assumes that the CPU buffer is synchronized with the GPU one. */
        void to_ppm(const std::string& path) const;

        /* Returns the width of the frame. */
        inline uint32_t w() const { return this->width; }
        /* Returns the height of the frame. */
        inline uint32_t h() const { return this->height; }
        
        /* Copy assignment operator for the Frame class. */
        inline Frame& operator=(const Frame& other) { return *this = Frame(other); }
        /* Move assignment operator for the Frame class. */
        inline Frame& operator=(Frame&& other) { if (this != &other) { swap(*this, other); } return *this; }
        /* Swap operator for the Frame class. */
        friend void swap(Frame& f1, Frame& f2);

    };

    /* Swap operator for the Frame class. */
    void swap(Frame& f1, Frame& f2);
}

#endif
