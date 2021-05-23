/* MEMORY POOL.hpp
 *   by Lut99
 *
 * Created:
 *   25/04/2021, 11:36:35
 * Last edited:
 *   23/05/2021, 20:37:45
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Contains the MemoryPool class, which is in charge of a single pool of
 *   GPU memory that it can hand out to individual buffers.
**/

#ifndef COMPUTE_MEMORY_POOL_HPP
#define COMPUTE_MEMORY_POOL_HPP

#include <vulkan/vulkan.h>
#include <unordered_map>

#include "CommandPool.hpp"
#include "tools/Array.hpp"

#include "GPU.hpp"

namespace RayTracer::Compute {
    /* "Base" handle for all memory objects allocated in the pool. */
    using MemoryHandle = uint32_t;
    /* Handle for buffer objects, which is used to reference buffers. */
    using BufferHandle = MemoryHandle;
    /* Handle for image objects, which is used to reference images. */
    using ImageHandle = MemoryHandle;



    /* The Buffer class, which is a reference to a Buffer allocated by the MemoryPool. Do NOT change any fields in this class directly, as memory etc is managed by the memory pool. */
    class Buffer {
    private:
        /* Handle for this buffer object. */
        BufferHandle vk_handle;

        /* The actual VkBuffer object constructed. */
        VkBuffer vk_buffer;

        /* Describes the usage flags set for this buffer. */
        VkBufferUsageFlags vk_usage_flags;
        /* Describes the sharing mode for this buffer. */
        VkSharingMode vk_sharing_mode;
        /* Describes the create flags for this buffer. */
        VkBufferCreateFlags vk_create_flags;

        /* Reference to the large memory block where this buffer is allocated. */
        VkDeviceMemory vk_memory;
        /* The offset of the internal buffer. */
        VkDeviceSize vk_memory_offset;
        /* The size of the internal buffer. */
        VkDeviceSize vk_memory_size;
        /* The actual size of the internal buffer, as reported by memory_requirements.size. */
        VkDeviceSize vk_req_memory_size;
        /* The properties of the memory for this buffer. */
        VkMemoryPropertyFlags vk_memory_properties;

        /* Private constructor for the Buffer class, which takes the buffer, the buffer's size and the properties of the pool's memory. */
        Buffer(BufferHandle handle, VkBuffer buffer, VkBufferUsageFlags vk_usage_flags, VkSharingMode vk_sharing_mode, VkBufferCreateFlags vk_create_flags, VkDeviceMemory vk_memory, VkDeviceSize memory_offset, VkDeviceSize memory_size, VkDeviceSize req_memory_size, VkMemoryPropertyFlags memory_properties);
        
        /* Mark the MemoryPool as friend. */
        friend class MemoryPool;

    public:
        /* Sets the buffer using an intermediate staging buffer. The staging buffer is copied using the given command buffer on the given queue. */
        void set(const GPU& gpu, const Buffer& staging_buffer, const CommandBuffer& command_buffer, VkQueue vk_queue, void* data, uint32_t n_bytes) const;
        /* Gets n_bytes data from this buffer using an intermediate staging buffer. The buffers are copied over using the given command buffer on the given queue. The result is put in the given pointer. */
        void get(const GPU& gpu, const Buffer& staging_buffer, const CommandBuffer& command_buffer, VkQueue vk_queue, void* data, uint32_t n_bytes) const;

        /* Maps the buffer to host-memory so it can be written to. Only possible if the VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT is set for the memory of this buffer's pool. Note that the memory is NOT automatically unmapped if the Buffer object is destroyed. */
        void map(const GPU& gpu, void** mapped_memory) const;
        /* Flushes all unflushed memory operations done on mapped memory. If the memory of this buffer has VK_MEMORY_PROPERTY_HOST_COHERENT_BIT set, then nothing is done as the memory is already automatically flushed. */
        void flush(const GPU& gpu) const;
        /* Unmaps buffer's memory. */
        void unmap(const GPU& gpu) const;

        /* Copies this buffer's content to another given buffer on the given memory-enabled GPU queue. The given command pool (which must be a pool for the memory-enabled queue) is used to schedule the copy. Optionally waits until the queue is idle before returning. Note that the given buffer needn't come from the same memory pool. */
        inline void copyto(const CommandBuffer& command_buffer, VkQueue vk_queue, const Buffer& destination, bool wait_queue_idle = true) const { return copyto(command_buffer, vk_queue, destination, std::numeric_limits<VkDeviceSize>::max(), 0, wait_queue_idle); }
        /* Copies this buffer's content to another given buffer on the given memory-enabled GPU queue. The given command pool (which must be a pool for the memory-enabled queue) is used to schedule the copy. Optionally waits until the queue is idle before returning. Note that the given buffer needn't come from the same memory pool. */
        void copyto(const CommandBuffer& command_buffer, VkQueue vk_queue, const Buffer& destination, VkDeviceSize n_bytes, VkDeviceSize target_offset = 0, bool wait_queue_idle = true) const;

        /* Returns the size of the buffer, in bytes. */
        inline VkDeviceSize size() const { return this->vk_memory_size; }
        /* Returns the memory offset of the buffer, in bytes. */
        inline VkDeviceSize offset() const { return this->vk_memory_offset; }
        /* Explicit retrieval of the internal buffer object. */
        inline const VkBuffer& buffer() const { return this->vk_buffer; }
        /* Implicit retrieval of the internal buffer object. */
        inline operator VkBuffer() const { return this->vk_buffer; }
        /* Explicit retrieval of the internal handle. */
        inline BufferHandle handle() const { return this->vk_handle; }
        /* Implicit retrieval of the internal handle. */
        inline operator BufferHandle() const { return this->vk_handle; }

    };

    /* Swap operator for the Buffer class. */
    void swap(Buffer& b1, Buffer& b2);



    /* The Image class, which is a reference to an Image allocated by the MemoryPool. Do NOT change any fields in this class directly, as memory etc is managed by the memory pool. */
    class Image {
    private:
        /* Handle for this image object. */
        ImageHandle vk_handle;

        /* The actual VkBuffer object constructed. */
        VkImage vk_image;
        /* The size of the image as a Vulkan extent. */
        VkExtent2D vk_extent;
        /* The format of the image. */
        VkFormat vk_format;
        /* The layout of the image. */
        VkImageLayout vk_layout;

        /* Describes the usage flags set for this image. */
        VkImageUsageFlags vk_usage_flags;
        /* Describes the sharing mode for this image. */
        VkSharingMode vk_sharing_mode;
        /* Describes the create flags for this image. */
        VkImageCreateFlags vk_create_flags;

        /* Reference to the large memory block where this image is allocated. */
        VkDeviceMemory vk_memory;
        /* The offset of the internal image. */
        VkDeviceSize vk_memory_offset;
        /* The size of the internal image. */
        VkDeviceSize vk_memory_size;
        /* The actual size of the internal image, as reported by memory_requirements.size. */
        VkDeviceSize vk_req_memory_size;
        /* The properties of the memory for this image. */
        VkMemoryPropertyFlags vk_memory_properties;

        /* Private constructor for the Buffer class, which takes the buffer, the buffer's size and the properties of the pool's memory. */
        Image(ImageHandle handle, VkImage image, VkExtent2D vk_extent, VkFormat vk_format, VkImageLayout vk_layout, VkImageUsageFlags vk_usage_flags, VkSharingMode vk_sharing_mode, VkImageCreateFlags vk_create_flags, VkDeviceMemory vk_memory, VkDeviceSize memory_offset, VkDeviceSize memory_size, VkDeviceSize req_memory_size, VkMemoryPropertyFlags memory_properties);
        
        /* Mark the MemoryPool as friend. */
        friend class MemoryPool;

    public:
        // /* Sets the buffer using an intermediate staging buffer. The staging buffer is copied using the given command buffer on the given queue. */
        // void set(const GPU& gpu, const Buffer& staging_buffer, const CommandBuffer& command_buffer, VkQueue vk_queue, void* data, uint32_t n_bytes) const;
        // /* Gets n_bytes data from this buffer using an intermediate staging buffer. The buffers are copied over using the given command buffer on the given queue. The result is put in the given pointer. */
        // void get(const GPU& gpu, const Buffer& staging_buffer, const CommandBuffer& command_buffer, VkQueue vk_queue, void* data, uint32_t n_bytes) const;

        // /* Maps the buffer to host-memory so it can be written to. Only possible if the VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT is set for the memory of this buffer's pool. Note that the memory is NOT automatically unmapped if the Buffer object is destroyed. */
        // void map(const GPU& gpu, void** mapped_memory) const;
        // /* Flushes all unflushed memory operations done on mapped memory. If the memory of this buffer has VK_MEMORY_PROPERTY_HOST_COHERENT_BIT set, then nothing is done as the memory is already automatically flushed. */
        // void flush(const GPU& gpu) const;
        // /* Unmaps buffer's memory. */
        // void unmap(const GPU& gpu) const;

        // /* Copies this buffer's content to another given buffer on the given memory-enabled GPU queue. The given command pool (which must be a pool for the memory-enabled queue) is used to schedule the copy. Optionally waits until the queue is idle before returning. Note that the given buffer needn't come from the same memory pool. */
        // inline void copyto(const CommandBuffer& command_buffer, VkQueue vk_queue, const Buffer& destination, bool wait_queue_idle = true) const { return copyto(command_buffer, vk_queue, destination, std::numeric_limits<VkDeviceSize>::max(), wait_queue_idle); }
        // /* Copies this buffer's content to another given buffer on the given memory-enabled GPU queue. The given command pool (which must be a pool for the memory-enabled queue) is used to schedule the copy. Optionally waits until the queue is idle before returning. Note that the given buffer needn't come from the same memory pool. */
        // void copyto(const CommandBuffer& command_buffer, VkQueue vk_queue, const Buffer& destination, VkDeviceSize n_bytes, bool wait_queue_idle = true) const;

        /* Returns the extent of the image. */
        inline VkExtent2D extent() const { return this->vk_extent; }
        /* Returns the format of the image. */
        inline VkFormat format() const { return this->vk_format; }
        /* Returns the size of the image, in bytes. */
        inline VkDeviceSize size() const { return this->vk_memory_size; }
        /* Returns the memory offset of the buffer, in bytes. */
        inline VkDeviceSize offset() const { return this->vk_memory_offset; }
        /* Explicit retrieval of the internal buffer object. */
        inline VkImage image() const { return this->vk_image; }
        /* Implicit retrieval of the internal buffer object. */
        inline operator VkImage() const { return this->vk_image; }
        /* Explicit retrieval of the internal handle. */
        inline ImageHandle handle() const { return this->vk_handle; }
        /* Implicit retrieval of the internal handle. */
        inline operator ImageHandle() const { return this->vk_handle; }

    };

    /* Swap operator for the Buffer class. */
    void swap(Buffer& b1, Buffer& b2);



    /* The MemoryPool class serves as a memory manager for our GPU memory. */
    class MemoryPool {
    public:
        /* Immutable reference to the GPU object where this pool is linked to. */
        const GPU& gpu;

    private:
        /* Internal enum used to differentiate between memory types. */
        enum class MemoryBlockType {
            none = 0,
            buffer = 1,
            image = 2
        };

        /* Internal base struct used to represent used memory blocks. */
        struct UsedBlock {
            /* The type of this block. */
            MemoryBlockType type;
            /* Pointer to the start of the block. */
            VkDeviceSize start;
            /* Length of the free block, in bytes. */
            VkDeviceSize length;
            /* The actual size of the block, as reported by memsize.size. */
            VkDeviceSize req_length;

            /* Constructor for the UsedBlock struct, which requires at least a type. */
            UsedBlock(MemoryBlockType type): type(type) {}
        };
        /* Internal specialized struct used to represent memory blocks used for buffers. */
        struct BufferBlock: public UsedBlock {
            /* The actual VkBuffer object that is bound to this memory location. */
            VkBuffer vk_buffer;
            /* The usage flags of the internal buffer. */
            VkBufferUsageFlags vk_usage_flags;
            /* The create flags used to create this buffer. */
            VkBufferCreateFlags vk_create_flags;
            /* The sharing mode of the buffer. */
            VkSharingMode vk_sharing_mode;

            /* Constructor for the BufferBlock struct. */
            BufferBlock(): UsedBlock(MemoryBlockType::buffer) {}
        };
        /* Internal specialized struct used to represent memory blocks used for images. */
        struct ImageBlock: public UsedBlock {
            /* The actual VkImage object that is bound to this memory location. */
            VkImage vk_image;
            /* The size of the image, as a VkExtent3D. */
            VkExtent3D vk_extent;
            /* The format of the image. */
            VkFormat vk_format;
            /* The initial layout of the image. */
            VkImageLayout vk_layout;
            /* The usage flags of the internal buffer. */
            VkImageUsageFlags vk_usage_flags;
            /* The create flags used to create this buffer. */
            VkImageCreateFlags vk_create_flags;
            /* The sharing mode of the buffer. */
            VkSharingMode vk_sharing_mode;

            /* Constructor for the ImageBlock struct. */
            ImageBlock(): UsedBlock(MemoryBlockType::image) {}
        };
        /* Internal struct used to keep track of free memory blocks. */
        struct FreeBlock {
            /* Pointer to the start of the block. */
            VkDeviceSize start;
            /* Length of the free block, in bytes. */
            VkDeviceSize length;  
        };

        /* The allocated memory on the GPU. */
        VkDeviceMemory vk_memory;
        /* The type of memory we allocated for this. */
        uint32_t vk_memory_type;
        /* The total size of the allocated memory block. */
        VkDeviceSize vk_memory_size;
        /* The memory properties assigned to this buffer. */
        VkMemoryPropertyFlags vk_memory_properties;

        /* List of all allocated buffers in this pool. */
        std::unordered_map<MemoryHandle, UsedBlock*> vk_used_blocks;
        /* List of pointers to each free memory block in the allocated space. */
        Tools::Array<FreeBlock> vk_free_blocks;


        /* Private helper function that takes a BufferBlock, and uses it to initialize the given buffer. */
        inline static Buffer init_buffer(BufferHandle handle, BufferBlock* block, VkDeviceMemory vk_memory, VkMemoryPropertyFlags memory_properties) { return Buffer(handle, block->vk_buffer, block->vk_usage_flags, block->vk_sharing_mode, block->vk_create_flags, vk_memory, block->start, block->length, block->req_length, memory_properties); }
        /* Private helper function that takes a UsedBlock, and uses it to initialize the given buffer. */
        inline static Image init_image(ImageHandle handle, ImageBlock* block, VkDeviceMemory vk_memory, VkMemoryPropertyFlags memory_properties) { return Image(handle, block->vk_image, VkExtent2D({ block->vk_extent.width, block->vk_extent.height }), block->vk_format, block->vk_layout, block->vk_usage_flags, block->vk_sharing_mode, block->vk_create_flags, vk_memory, block->start, block->length, block->req_length, memory_properties); }

        /* Private helper function that actually performs memory allocation. Returns a reference to a UsedBlock that describes the block allocated. */
        MemoryHandle allocate_memory(MemoryBlockType type, VkDeviceSize n_bytes, const VkMemoryRequirements& mem_requirements);

    public:
        /* The null handle for the pool. */
        const static constexpr MemoryHandle NullHandle = 0;

        
        /* Constructor for the MemoryPool class, which takes a device to allocate on, the type of memory we will allocate on and the total size of the allocated block. */
        MemoryPool(const GPU& gpu, uint32_t memory_type, VkDeviceSize n_bytes, VkMemoryPropertyFlags memory_properties = 0);
        /* Copy constructor for the MemoryPool class, which is deleted. */
        MemoryPool(const MemoryPool& other);
        /* Move constructor for the MemoryPool class. */
        MemoryPool(MemoryPool&& other);
        /* Destructor for the MemoryPool class. */
        ~MemoryPool();

        /* Returns a reference to the internal buffer with the given handle. Always performs out-of-bounds checking. */
        inline Buffer deref_buffer(BufferHandle buffer) const { return init_buffer(buffer, (BufferBlock*) this->vk_used_blocks.at(buffer), this->vk_memory, this->vk_memory_properties); }
        /* Returns a reference to the internal image with the given handle. Always performs out-of-bounds checking. */
        inline Image deref_image(ImageHandle image) const { return init_image(image, (ImageBlock*) this->vk_used_blocks.at(image), this->vk_memory, this->vk_memory_properties); }

        /* Tries to get a new buffer from the pool of the given size and with the given flags. Applies extra checks if NDEBUG is not defined. */
        inline Buffer allocate_buffer(VkDeviceSize n_bytes, VkBufferUsageFlags usage_flags, VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE, VkBufferCreateFlags create_flags = 0) { return this->deref_buffer(this->allocate_buffer_h(n_bytes, usage_flags, sharing_mode, create_flags)); }
        /* Allocates a new buffer that has the same specifications as the given Buffer object. Note that the given Buffer needn't be allocated with the same pool as this one. */
        inline Buffer allocate_buffer(const Buffer& buffer) { return this->allocate_buffer(buffer.vk_memory_size, buffer.vk_usage_flags, buffer.vk_sharing_mode, buffer.vk_create_flags); }
        /* Tries to get a new buffer from the pool of the given size and with the given flags, returning only its handle. Applies extra checks if NDEBUG is not defined. */
        BufferHandle allocate_buffer_h(VkDeviceSize n_bytes, VkBufferUsageFlags usage_flags, VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE, VkBufferCreateFlags create_flags = 0);
        /* Allocates a new buffer that has the same specifications as the given Buffer object, but we return only its handle. Note that the given Buffer needn't be allocated with the same pool as this one. */
        inline BufferHandle allocate_buffer_h(const Buffer& buffer) { return this->allocate_buffer_h(buffer.vk_memory_size, buffer.vk_usage_flags, buffer.vk_sharing_mode, buffer.vk_create_flags); }
        /* Tries to get a new image from the pool of the given sizes and with the given flags. Applies extra checks if NDEBUG is not defined. */
        inline Image allocate_image(uint32_t width, uint32_t height, VkFormat image_format, VkImageLayout image_layout, VkImageUsageFlags usage_flags, VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE, VkImageCreateFlags create_flags = 0) { return this->deref_image(this->allocate_image_h(width, height, image_format, image_layout, usage_flags, sharing_mode, create_flags)); }
        /* Allocates a new image that has the same specifications as the given Image object. Note that the given Image needn't be allocated with the same pool as this one. */
        inline Image allocate_image(const Image& image) { return this->allocate_image(image.vk_extent.width, image.vk_extent.height, image.vk_format, image.vk_layout, image.vk_usage_flags, image.vk_sharing_mode, image.vk_format); }
        /* Tries to get a new image from the pool of the given sizes and with the given flags, returning only its handle. Applies extra checks if NDEBUG is not defined. */
        ImageHandle allocate_image_h(uint32_t width, uint32_t height, VkFormat image_format, VkImageLayout image_layout, VkImageUsageFlags usage_flags, VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE, VkImageCreateFlags create_flags = 0);
        /* Allocates a new image that has the same specifications as the given Image object, returning only its handle. Note that the given Image needn't be allocated with the same pool as this one. */
        inline ImageHandle allocate_image_h(const Image& image) { return this->allocate_image_h(image.vk_extent.width, image.vk_extent.height, image.vk_format, image.vk_layout, image.vk_usage_flags, image.vk_sharing_mode, image.vk_format); }
        /* Deallocates the buffer or image with the given handle. Does not throw an error if the handle doesn't exist, unless NDEBUG is not defined. */
        void deallocate(MemoryHandle handle);

        /* Defragements the entire pool, aligning all buffers next to each other in memory to create a maximally sized free block. Note that existing handles will remain valid. */
        void defrag();

        /* Copy assignment operator for the MemoryPool class. */
        inline MemoryPool& operator=(const MemoryPool& other) { return *this = MemoryPool(other); }
        /* Move assignment operator for the MemoryPool class. */
        inline MemoryPool& operator=(MemoryPool&& other) { if (this != &other) { swap(*this, other); } return *this; }
        /* Swap operator for the MemoryPool class. */
        friend void swap(MemoryPool& mp1, MemoryPool& mp2);

        /* Static function that helps users decide the best memory queue. */
        static uint32_t select_memory_type(const GPU& gpu, VkBufferUsageFlags usage_flags = 0, VkMemoryPropertyFlags memory_properties = 0, VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE, VkBufferCreateFlags create_flags = 0);
        /* Static function that helps users decide the best memory queue for images. */
        static uint32_t select_memory_type(const GPU& gpu, VkFormat format, VkImageLayout layout, VkImageUsageFlags usage_flags, VkMemoryPropertyFlags memory_properties, VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE, VkImageCreateFlags create_flags = 0);

    };

    /* Swap operator for the MemoryPool class. */
    void swap(MemoryPool& mp1, MemoryPool& mp2);

}

#endif
