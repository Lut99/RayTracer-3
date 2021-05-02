/* CAMERA.hpp
 *   by Lut99
 *
 * Created:
 *   28/04/2021, 20:08:29
 * Last edited:
 *   02/05/2021, 17:28:53
 * Auto updated?
 *   Yes
 *
 * Description:
 *   The camera class, which computes the required camera matrices
 *   per-frame and can optionally move the camera in between frames.
**/

#ifndef CAMERA_CAMERA_HPP
#define CAMERA_CAMERA_HPP

#include "compute/MemoryPool.hpp"
#include "compute/DescriptorPool.hpp"
#include "compute/DescriptorSetLayout.hpp"
#include "compute/CommandPool.hpp"
#include "compute/Pipeline.hpp"

#include "frame/Frame.hpp"

namespace RayTracer {
    /* The CameraData struct, which describes what is send to the GPU and how. */
    struct CameraData {
        /* Vector placing the origin (middle) of the camera viewport in the world. */
        glm::vec4 origin;
        /* Vector determining the horizontal line of the camera viewport in the game world, and also its conceptual size. */
        glm::vec4 horizontal;
        /* Vector determining the vertical line of the camera viewport in the game world, and also its conceptual size. */
        glm::vec4 vertical;
        /* Vector describing the lower left corner of the viewport. Shortcut based on the other three. */
        glm::vec4 lower_left_corner;
    };



    /* The Camera class, which computes the required camera matrices for each frame. */
    class Camera {
    public:
        /* Constant reference to the GPU where this camera renders on. */
        const Compute::GPU& gpu;

    private:
        /* Reference to the memory pool where we allocate buffers from. */
        Compute::MemoryPool* pool;

        /* The internal Frame object that the camera uses to obtain the rendered view. */
        Frame* frame;

        /* Command buffer used to move data around. */
        Compute::CommandBuffer cmd_buffer;
        /* CPU-side buffer used to populate the GPU-side one. */
        CameraData* cpu_buffer;
        /* Handle to the uniform buffer used to describe the camera matrices. */
        Compute::BufferHandle gpu_buffer;

        /* Temporary storage for the binding index of the camera buffer. */
        uint32_t bind_index;

    public:
        /* Constructor for the Camera class, */
        Camera(const Compute::GPU& gpu, Compute::MemoryPool& mpool, Compute::CommandPool& cpool);
        /* Copy constructor for the Camera class. */
        Camera(const Camera& other);
        /* Move constructor for the Camera class. */
        Camera(Camera&& other);
        /* Destructor for the Camera class. */
        ~Camera();

        /* Computes new camera matrices for the given position and orientation. */
        void update(uint32_t width, uint32_t height, float focal_length, float viewport_width, float viewport_height /* TBD */);

        /* Adds new bindings for all relevant Camera-managed objects to the given DescriptorSetLayout. */
        void set_layout(Compute::DescriptorSetLayout& descriptor_set_layout);
        /* Writes buffers to the bindings set in set_layout. Will throw errors if no binding has been set first. */
        void set_bindings(Compute::DescriptorSet& descriptor_set) const;

        /* Synchronizes the Camera with the GPU buffers, so they can be used during rendering. */
        void sync(Compute::MemoryPool& mpool);
        
        /* Returns the result of a render as a constant reference to the internal frame. Note that the queue where it was rendered should really be idle before you call this. */
        const Frame& get_frame(Compute::MemoryPool& staging_pool) const;

        /* Returns the width (in pixels) of the current camera frame. Will probably segfault if not set. */
        inline uint32_t w() const { return this->frame->w(); }
        /* Returns the height (in pixels) of the current camera frame. Will probably segfault if not set. */
        inline uint32_t h() const { return this->frame->h(); }

        /* Copy assignment operator for the Camera class. */
        inline Camera& operator=(const Camera& other) { return *this = Camera(other); }
        /* Move assignment operator for the Camera class. */
        inline Camera& operator=(Camera&& other) { if (this != &other) { swap(*this, other); } return *this; }
        /* Swap operator for the Camera class. */
        friend void swap(Camera& c1, Camera& c2);

    };

    /* Swap operator for the Camera class. */
    void swap(Camera& c1, Camera& c2);
}

#endif
