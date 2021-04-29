/* CAMERA.cpp
 *   by Lut99
 *
 * Created:
 *   28/04/2021, 20:08:23
 * Last edited:
 *   29/04/2021, 15:43:11
 * Auto updated?
 *   Yes
 *
 * Description:
 *   The camera class, which computes the required camera matrices
 *   per-frame and can optionally move the camera in between frames.
**/

#include <limits>
#include <CppDebugger.hpp>

#include "compute/ErrorCodes.hpp"

#include "Camera.hpp"

using namespace std;
using namespace RayTracer;
using namespace RayTracer::Compute;
using namespace CppDebugger::SeverityValues;


/***** CAMERA CLASS *****/
/* Constructor for the Camera class, */
Camera::Camera(const Compute::GPU& gpu, Compute::MemoryPool& mpool, Compute::CommandPool& cpool) :
    gpu(gpu),
    pool(&mpool),
    frame(nullptr),
    cmd_buffer(cpool[cpool.allocate()]),
    bind_index(numeric_limits<uint32_t>::max())
{
    DENTER("Camera::Camera");

    // Setup the camera CPU-side buffer by allocating it
    this->cpu_buffer = new CameraData;
    
    // Next, setup the GPU-side buffer
    this->gpu_buffer = mpool.allocate(sizeof(CameraData), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    // Done!
    DLEAVE;
}

/* Copy constructor for the Camera class. */
Camera::Camera(const Camera& other) :
    gpu(other.gpu),
    pool(other.pool),
    frame(new Frame(*other.frame)),
    cmd_buffer(other.cmd_buffer),
    bind_index(other.bind_index)
{
    DENTER("Camera::Camera(copy)");

    // First, allocate a new CPU buffer
    this->cpu_buffer = new CameraData;
    // Copy the old buffer to this one
    memcpy(this->cpu_buffer, other.cpu_buffer, sizeof(CameraData));

    // Next, allocate a new GPU buffer
    this->gpu_buffer = this->pool->allocate(sizeof(CameraData), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    // Use the command buffer to copy the data to the new GPU buffer
    MemoryPool& pool = *(this->pool);
    pool[other.gpu_buffer].copyto(this->gpu, this->cmd_buffer, pool[this->gpu_buffer]);

    // Done
    DLEAVE;
}

/* Move constructor for the Camera class. */
Camera::Camera(Camera&& other)  :
    gpu(other.gpu),
    pool(other.pool),
    frame(other.frame),
    cmd_buffer(other.cmd_buffer),
    cpu_buffer(other.cpu_buffer),
    gpu_buffer(other.gpu_buffer),
    bind_index(other.bind_index)
{
    // Set the other pointers w/e to null to avoid deallocation
    other.frame = nullptr;
    other.cpu_buffer = nullptr;
    other.gpu_buffer = MemoryPool::NullHandle;
}

/* Destructor for the Camera class. */
Camera::~Camera() {
    DENTER("Camera::~Camera");

    if (this->gpu_buffer != MemoryPool::NullHandle) {
        this->pool->deallocate(this->gpu_buffer);
    }

    if (this->cpu_buffer != nullptr) {
        delete this->cpu_buffer;
    }

    if (this->frame != nullptr) {
        delete this->frame;
    }

    DLEAVE;
}



/* Computes new camera matrices for the given position and orientation. */
void Camera::update(uint32_t width, uint32_t height, float focal_length, float viewport_width, float viewport_height /* TBD */) {
    DENTER("Camera::update");

    // If there already exists a frame, destroy it
    if (this->frame != nullptr) {
        delete this->frame;
    }

    // First, initialize the frame struct
    this->frame = new Frame(this->gpu, *(this->pool), this->cmd_buffer, width, height);
    
    // Next, compute all new vectors
    glm::vec3 origin = glm::vec3(0.0, 0.0, 0.0);
    glm::vec3 horizontal = glm::vec3(viewport_width, 0.0, 0.0);
    glm::vec3 vertical = glm::vec3(0.0, viewport_height, 0.0);
    glm::vec3 lower_left_corner = origin - horizontal / glm::vec3(2.0) - vertical / glm::vec3(2.0) - glm::vec3(0.0, 0.0, focal_length);

    // Update them in the internal vector
    this->cpu_buffer->origin = glm::vec4(origin, 0.0);
    this->cpu_buffer->horizontal = glm::vec4(horizontal, 0.0);
    this->cpu_buffer->vertical = glm::vec4(vertical, 0.0);
    this->cpu_buffer->lower_left_corner = glm::vec4(lower_left_corner, 0.0);

    // Done
    DRETURN;
}



/* Adds new bindings for all relevant Camera-managed objects to the given DescriptorSetLayout. */
void Camera::set_layout(Compute::DescriptorSetLayout& descriptor_set_layout) {
    DENTER("Camera::set_layout");

    // Make sure the frame exists
    if (this->frame == nullptr) {
        DLOG(fatal, "Cannot set layout for unupdated Camera.");
    }

    // Set the camera buffer layout first
    this->bind_index = descriptor_set_layout.add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT);

    // Do the frame as well
    this->frame->set_layout(descriptor_set_layout);

    // DOne
    DRETURN;
}

/* Writes buffers to the bindings set in set_layout. Will throw errors if no binding has been set first. */
void Camera::set_bindings(Compute::DescriptorSet& descriptor_set) const {
    DENTER("Camera::set_bindings");

    // Make sure set_layout has been called first
    if (this->bind_index == numeric_limits<uint32_t>::max()) {
        DLOG(fatal, "Cannot write to binding without having defined it in the layout.");
    }

    // Set the camera buffer layout first
    descriptor_set.set(this->gpu, this->bind_index, Tools::Array<Buffer>({ (*(this->pool))[this->gpu_buffer] }));

    // Do the frame as well
    this->frame->set_binding(descriptor_set);

    // DOne
    DRETURN;
}



/* Renders a single frame for the camera, using the given World object. To retrieve the frame, call get_frame() when the queue is guaranteed to be idle. */
void Camera::render(Compute::MemoryPool& mpool, Compute::CommandPool& cpool, const Compute::Pipeline& pipeline, VkQueue vk_compute_queue, Compute::DescriptorSet& descriptor_set /* TBD */, bool wait_queue_idle) {
    DENTER("Camera::render");

    // First, update camera stuff; push the CPU buffer to the GPU
    DLOG(info, "Syncing camera with GPU...");
    BufferHandle staging_buffer_h = mpool.allocate(sizeof(CameraData), VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    Buffer staging_buffer = mpool[staging_buffer_h];

    // Map the staging buffer and copy the CPU buffer to it
    void* mapped_memory;
    staging_buffer.map(this->gpu, &mapped_memory);
    memcpy(mapped_memory, this->cpu_buffer, sizeof(CameraData));
    staging_buffer.flush(this->gpu);
    staging_buffer.unmap(this->gpu);

    // Use the internal command buffer to schedule the copy
    staging_buffer.copyto(this->gpu, this->cmd_buffer, (*(this->pool))[this->gpu_buffer]);

    // Once done, deallocate the staging buffer
    mpool.deallocate(staging_buffer_h);

    // Next, also in the update camera corner, update the camera bindings
    this->set_bindings(descriptor_set);
    
    

    // Next, we can begin the render phase; record the command buffer
    DLOG(info, "Recording command buffer...");
    CommandBuffer cb_compute = cpool[cpool.allocate()];
    cb_compute.begin();
    pipeline.bind(cb_compute);
    descriptor_set.bind(cb_compute, pipeline.layout());
    vkCmdDispatch(cb_compute, (this->frame->w() / 32) + 1, (this->frame->h() / 32) + 1, 1);
    cb_compute.end();


    
    // With the buffer recorded, submit it to the given queue
    DLOG(info, "Launching pipeline...");
    VkResult vk_result;
    VkSubmitInfo submit_info = cb_compute.get_submit_info();
    if ((vk_result = vkQueueSubmit(vk_compute_queue, 1, &submit_info, VK_NULL_HANDLE)) != VK_SUCCESS) {
        DLOG(fatal, "Could not submit command buffer to queue: " + vk_error_map[vk_result]);
    }



    // If we're told to wait, then wait until the queue is idle; otherwise, continue
    if (wait_queue_idle) {
        DLOG(info, "Waiting for queue to become idle...");
        if ((vk_result = vkQueueWaitIdle(vk_compute_queue)) != VK_SUCCESS) {
            DLOG(fatal, "Could not wait for queue to become idle: " + vk_error_map[vk_result]);
        }
    }

    // Done
    DRETURN;
}

/* Returns the result of a render as a constant reference to the internal frame. Note that the queue where it was rendered should really be idle before you call this. */
const Frame& Camera::get_frame(Compute::MemoryPool& staging_pool) const {
    DENTER("Camera::get_frame");

    // Synchronize the frame with the GPU
    this->frame->sync(staging_pool);

    // Return it
    DRETURN *(this->frame);
}



/* Swap operator for the Camera class. */
void RayTracer::swap(Camera& c1, Camera& c2) {
    DENTER("RayTracer::swap(Camera)");

    #ifndef NDEBUG
    // If the GPU is not the same, then initialize to all nullptrs and everything
    if (c1.gpu != c2.gpu) {
        DLOG(fatal, "Cannot swap cameras with different GPUs");
    }
    #endif

    using std::swap;

    // Swap all fields
    swap(c1.pool, c2.pool);
    swap(c1.frame, c2.frame);
    swap(c1.cmd_buffer, c2.cmd_buffer);
    swap(c1.cpu_buffer, c2.cpu_buffer);
    swap(c1.gpu_buffer, c2.gpu_buffer);
    swap(c1.bind_index, c2.bind_index);

    // Done 
    DRETURN;
}
