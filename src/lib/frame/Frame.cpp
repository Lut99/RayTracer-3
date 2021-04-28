/* FRAME.cpp
 *   by Lut99
 *
 * Created:
 *   28/04/2021, 14:28:32
 * Last edited:
 *   28/04/2021, 17:29:42
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Contains the Frame class, which is a wrapper around CPU & Vulkan
 *   buffers to provide a conceptually easy frame to fill with colour
 *   pixels on the GPU. Also contains code to easily write to PNG, using
 *   the LodePNG library.
**/

#include <fstream>
#include <cstring>
#include <cerrno>
#include <CppDebugger.hpp>

#include "LodePNG.hpp"

#include "Frame.hpp"

using namespace std;
using namespace RayTracer;
using namespace RayTracer::Compute;
using namespace CppDebugger::SeverityValues;


/***** MACROS *****/
/* Computes the size of the frame, in bytes. */
#define FRAME_SIZE(WIDTH, HEIGHT) \
    ((WIDTH) * (HEIGHT) * sizeof(pixel_t))





/***** FRAME CLASS *****/
/* Constructor for the Frame class, which takes the GPU where the frame lives, a memory pool to allocate the GPU-side buffer from, a command pool to allocate a command buffer for memory operations from (the pool itself will not stay referenced) and the size of the image (in pixels). */
Frame::Frame(const Compute::GPU& gpu, Compute::MemoryPool& mpool, Compute::CommandPool& cpool, uint32_t width, uint32_t height) :
    gpu(gpu),
    pool(&mpool),
    width(width),
    height(height),
    cmd_buffer(cpool[cpool.allocate()])
{
    DENTER("Frame::Frame");

    // Before we begin, make sure that the gpu is consistent across all pools
    if (gpu != mpool.gpu || gpu != cpool.gpu) {
        DLOG(fatal, "All pools must have the same GPU as passed (got " + (gpu != mpool.gpu ? mpool.gpu.name() : cpool.gpu.name()) + ", expected " + gpu.name());
    }

    // We start by allocating the CPU memory
    this->cpu_buffer = new pixel_t[FRAME_SIZE(this->width, this->height)];

    // Next, allocate a buffer on the GPU. Note that we'll only read (CPU-side, then) from this one.
    this->gpu_buffer = this->pool->allocate(FRAME_SIZE(this->width, this->height), VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    // Done!
    DLEAVE;
}

/* Copy constructor for the Frame class. */
Frame::Frame(const Frame& other) :
    gpu(other.gpu),
    pool(other.pool),
    width(other.width),
    height(other.height),
    cmd_buffer(other.cmd_buffer)
{
    DENTER("Frame::Frame(copy)");

    // Allocate a new CPU-side buffer
    this->cpu_buffer = new pixel_t[FRAME_SIZE(this->width, this->height)];
    // Copy the entire memory in one go
    memcpy(this->cpu_buffer, other.cpu_buffer, FRAME_SIZE(this->width, this->height));

    // Using the internal pool, we'll allocate an additional buffer. Note that this one's also a destination to facilitate copying the data.
    this->gpu_buffer = this->pool->allocate(FRAME_SIZE(this->width, this->height), VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    // Copy the data
    MemoryPool& pool = *(this->pool);
    pool[other.gpu_buffer].copyto(this->gpu, this->cmd_buffer, pool[this->gpu_buffer]);

    DLEAVE;
}

/* Move constructor for the Frame class. */
Frame::Frame(Frame&& other) :
    gpu(other.gpu),
    pool(other.pool),
    width(other.width),
    height(other.height),
    cmd_buffer(other.cmd_buffer),
    cpu_buffer(other.cpu_buffer),
    gpu_buffer(other.gpu_buffer)
{
    // Set fields to nullptrs etc to avoid deallocation
    other.cpu_buffer = nullptr;
    other.gpu_buffer = MemoryPool::NullHandle;
}

/* Desctructor for the Frame class. */
Frame::~Frame() {
    DENTER("Frame::~Frame");

    // We can deallocate our gpu-side buffer if we still have it
    if (this->gpu_buffer != MemoryPool::NullHandle) {
        this->pool->deallocate(this->gpu_buffer);
    }

    // Possibly deallocate the cpu_buffer, if it isn't stolen
    if (this->cpu_buffer != nullptr) {
        delete[] this->cpu_buffer;
    }

    DLEAVE;
}



/* Given a DescriptorSetLayout, adds a new binding for the storage buffer used internally in the Frame. */
void Frame::add_binding(Compute::DescriptorSetLayout& descriptor_set_layout) {
    DENTER("Frame::add_binding");

    // Simply call the add_binding with the correct parameters
    descriptor_set_layout.add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT);
    
    // Done already!
    DRETURN;
}

/* Given a DescriptorSet, writes the internal buffer to it. The given index is the binding created using add_binding(). */
void Frame::set_descriptor(Compute::DescriptorSet& descriptor_set, uint32_t binding_index) {
    DENTER("Frame::set_descriptor");

    // Simply call the add_binding with the correct parameters
    descriptor_set.set(this->gpu, binding_index, Tools::Array<Buffer>({ (*(this->pool))[this->gpu_buffer] }));
    
    // Done already!
    DRETURN;
}



/* Synchronizes the CPU buffer with the GPU buffer. Since we assume that only the GPU does writing, this means it's copied from GPU memory to CPU memory. The given memory pool is used to create a staging buffer for this event. */
void Frame::sync(Compute::MemoryPool& staging_pool) {
    DENTER("Frame::sync");

    // Fetch an instance of the internal buffer
    Buffer frame_buffer = (*(this->pool))[this->gpu_buffer];

    // Create a staging buffer
    BufferHandle staging_buffer_h = staging_pool.allocate(FRAME_SIZE(this->width, this->height), VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    Buffer staging_buffer = staging_pool[staging_buffer_h];

    // Copy one buffer to the other, using the internal memory queue
    frame_buffer.copyto(this->gpu, this->cmd_buffer, staging_buffer);

    // Map the staging buffer to host memory to access it
    pixel_t* staging_map;
    staging_buffer.map(this->gpu, (void**) &staging_map);

    // Copy the memory to the CPU buffer
    memcpy(this->cpu_buffer, staging_map, FRAME_SIZE(this->width, this->height));

    // Once done, unmap & deallocate the staging buffer
    staging_buffer.unmap(this->gpu);
    staging_pool.deallocate(staging_buffer_h);

    // Done!
    DRETURN;
}



/* Writes the internal frame to disk as a PNG. Assumes that the CPU buffer is synchronized with the GPU one. */
void Frame::to_png(const std::string& path) {
    DENTER("Frame::to_png");

    // Use the internal CPU buffer to construct a 0-255, four channel vector
    vector<unsigned char> raw_image;
    raw_image.resize(4 * this->width * this->height);
    for (uint32_t i = 0; i < this->width * this->height; i++) {
        glm::vec4& pixel = this->cpu_buffer[i];

        // Convert to 255
        raw_image[4 * i    ] = (unsigned char) (255.0 * pixel.r);
        raw_image[4 * i + 1] = (unsigned char) (255.0 * pixel.g);
        raw_image[4 * i + 2] = (unsigned char) (255.0 * pixel.b);
        raw_image[4 * i + 3] = 255;
    }

    // With the raw image created, we can send it to the file using lodepng
    unsigned result = lodepng::encode(path.c_str(), raw_image, this->width, this->height);
    if (result != 0) {
        DLOG(fatal, "Could not write to PNG: " + std::string(lodepng_error_text(result)));
    }
    
    // Done
    DRETURN;
}

/* Writes the internal frame to disk as a PPM. Assumes that the CPU buffer is synchronized with the GPU one. */
void Frame::to_ppm(const std::string& path) {
    DENTER("Frame::to_ppm");

    // Open a file handle
    std::ofstream h(path, ios::binary);
    if (!h.is_open()) {
        #ifdef _WIN32
        char buffer[BUFSIZ];
        strerror_s(buffer, BUFSIZ, errno);
        #else
        char* buffer = strerror(errno);
        #endif
        DLOG(fatal, "Could not open '" + path + "': " + buffer);
    }

    // Otherwise, write the PPM header
    h << "P6" << endl;
    h << "# Image rendered by the RayTracer-3" << endl;
    h << this->width << " " << this->height << endl;
    h << "255" << endl;

    // Now we write the data
    for (size_t i = 0; i < this->width * this->height; i++) {
        // Convert the floats to bytes
        glm::vec4& pixel = this->cpu_buffer[i];
        unsigned char r = (unsigned char) (255.0 / pixel.r);
        unsigned char g = (unsigned char) (255.0 / pixel.g);
        unsigned char b = (unsigned char) (255.0 / pixel.b);

        // Write to the file
        h.write((const char*) &r, sizeof(unsigned char));
        h.write((const char*) &g, sizeof(unsigned char));
        h.write((const char*) &b, sizeof(unsigned char));
    }

    // Alrighty! Looks good!
    h.close();
    DRETURN;
}



/* Swap operator for the Frame class. */
void RayTracer::swap(Frame& f1, Frame& f2) {
    DENTER("RayTracer::swap(Frame)");

    #ifndef NDEBUG
    // If the GPU is not the same, then initialize to all nullptrs and everything
    if (f1.gpu != f2.gpu) {
        DLOG(fatal, "Cannot swap frames with different GPUs");
    }
    #endif

    using std::swap;

    // Simply swap all fields
    swap(f1.pool, f2.pool);
    swap(f1.width, f2.width);
    swap(f1.height, f2.height);
    swap(f1.cmd_buffer, f2.cmd_buffer);
    swap(f1.cpu_buffer, f2.cpu_buffer);
    swap(f1.gpu_buffer, f2.gpu_buffer);
}
