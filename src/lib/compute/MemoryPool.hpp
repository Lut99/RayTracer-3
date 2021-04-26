/* MEMORY POOL.hpp
 *   by Lut99
 *
 * Created:
 *   25/04/2021, 11:36:35
 * Last edited:
 *   26/04/2021, 17:29:33
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

#include "tools/Array.hpp"

#include "GPU.hpp"

namespace RayTracer::Compute {
    /* Handle for a Buffer, which is used to reference buffers. */
    using BufferHandle = uint32_t;

    /* The Buffer class, which is a reference to a Buffer allocated by the MemoryPool. Do NOT change any fields in this class directly, as memory etc is managed by the memory pool. */
    class Buffer {
    private:
        /* The actual VkBuffer object constructed. */
        VkBuffer vk_buffer;
        /* Reference to the large memory block where this buffer is allocated. */
        VkDeviceMemory vk_memory;
        /* The size of the internal buffer. */
        VkDeviceSize vk_buffer_size;
        /* The properties of the memory for this buffer. */
        VkMemoryPropertyFlags vk_memory_properties;

        /* Private constructor for the Buffer class, which takes the buffer, the buffer's size and the properties of the pool's memory. */
        Buffer(VkBuffer buffer, VkDeviceMemory vk_memory, VkDeviceSize buffer_size, VkMemoryPropertyFlags memory_properties);
        
        /* Mark the MemoryPool as friend. */
        friend class MemoryPool;

    public:
        /* Directly sets the value of this buffer. Only possible if the VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT is set for the memory of this buffer's pool. */
        void set(const GPU& gpu, void* data, size_t n_bytes);

        /* Returns the size of the buffer, in bytes. */
        VkDeviceSize size() const { return this->vk_buffer_size; }
        /* Explicit retrieval of the internal buffer object. */
        inline VkBuffer buffer() const { return this->vk_buffer; }
        /* Implicit retrieval of the internal buffer object. */
        inline operator VkBuffer() const { return this->vk_buffer; }

    };

    /* Swap operator for the Buffer class. */
    void swap(Buffer& b1, Buffer& b2);



    /* The MemoryPool class serves as a memory manager for our GPU memory. */
    class MemoryPool {
    private:
        /* Internal struct used to represent used memory blocks. */
        struct UsedBlock {
            /* Pointer to the start of the block. */
            VkDeviceSize start;
            /* Length of the free block, in bytes. */
            VkDeviceSize length;

            /* The actual VkBuffer object that is bound to this memory location. */
            VkBuffer vk_buffer;

            /* The usage flags of the internal buffer. */
            VkBufferUsageFlags vk_usage_flags;
            /* The create flags used to create this buffer. */
            VkBufferCreateFlags vk_create_flags;
            /* The sharing mode of the buffer. */
            VkSharingMode vk_sharing_mode;
        };
        /* Internal struct used to keep track of free memory blocks. */
        struct FreeBlock {
            /* Pointer to the start of the block. */
            VkDeviceSize start;
            /* Length of the free block, in bytes. */
            VkDeviceSize length;  
        };

        /* Immutable reference to the GPU object where this pool is linked to. */
        const GPU& gpu;

        /* The allocated memory on the GPU. */
        VkDeviceMemory vk_memory;
        /* The type of memory we allocated for this. */
        uint32_t vk_memory_type;
        /* The total size of the allocated memory block. */
        VkDeviceSize vk_memory_size;
        /* The memory properties assigned to this buffer. */
        VkMemoryPropertyFlags vk_memory_properties;

        /* List of all allocated buffers in this pool. */
        std::unordered_map<BufferHandle, UsedBlock> vk_used_blocks;
        /* List of pointers to each free memory block in the allocated space. */
        Tools::Array<FreeBlock> vk_free_blocks;


        /* Private helper function that takes a UsedBlock, and uses it to initialize the given buffer. */
        inline static Buffer init_buffer(const UsedBlock& used_block, VkDeviceMemory vk_memory, VkMemoryPropertyFlags memory_properties) { return Buffer(used_block.vk_buffer, vk_memory, used_block.length, memory_properties); }

    public:
        /* Constructor for the MemoryPool class, which takes a device to allocate on, the type of memory we will allocate on and the total size of the allocated block. */
        MemoryPool(const GPU& gpu, uint32_t memory_type, VkDeviceSize n_bytes, VkMemoryPropertyFlags memory_properties = 0);
        /* Copy constructor for the MemoryPool class, which is deleted. */
        MemoryPool(const MemoryPool& other) = delete;
        /* Move constructor for the MemoryPool class. */
        MemoryPool(MemoryPool&& other);
        /* Destructor for the MemoryPool class. */
        ~MemoryPool();

        /* Returns a reference to the internal buffer object with the given handle. Does not perform out-of-bounds checking. */
        inline Buffer operator[](BufferHandle buffer) const { return init_buffer(this->vk_used_blocks.at(buffer), this->vk_memory, this->vk_memory_properties); }
        /* Returns a reference to the internal buffer with the given handle. Always performs out-of-bounds checking. */
        Buffer at(BufferHandle buffer) const;

        /* Tries to get a new buffer from the pool of the given size and with the given flags. Applies extra checks if NDEBUG is not defined. */
        BufferHandle allocate(VkDeviceSize n_bytes, VkBufferUsageFlags usage_flags, VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE, VkBufferCreateFlags create_flags = 0);
        /* Deallocates the buffer with the given handle. Does not throw an error if the handle doesn't exist, unless NDEBUG is not defined. */
        void deallocate(BufferHandle buffer);

        /* Defragements the entire pool, aligning all buffers next to each other in memory to create a maximally sized free block. Note that existing handles will remain valid. */
        void defrag();

        /* Copy constructor for the MemoryPool class, which is deleted. */
        MemoryPool& operator=(const MemoryPool& other) = delete;
        /* Move constructor for the MemoryPool class. */
        MemoryPool& operator=(MemoryPool&& other);
        /* Swap operator for the MemoryPool class. */
        friend void swap(MemoryPool& mp1, MemoryPool& mp2);

        /* Static function that helps users decide the best memory queue. */
        static uint32_t select_memory_type(const GPU& gpu, VkBufferUsageFlags usage_flags, VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE, VkBufferCreateFlags create_flags = 0);

    };

    /* Swap operator for the MemoryPool class. */
    void swap(MemoryPool& mp1, MemoryPool& mp2);

}

#endif
