/* MEMORY POOL.cpp
 *   by Lut99
 *
 * Created:
 *   25/04/2021, 11:36:42
 * Last edited:
 *   03/05/2021, 13:54:32
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Contains the MemoryPool class, which is in charge of a single pool of
 *   GPU memory that it can hand out to individual buffers.
**/

#include <limits>
#include <CppDebugger.hpp>

#include "ErrorCodes.hpp"

#include "MemoryPool.hpp"

using namespace std;
using namespace RayTracer;
using namespace RayTracer::Compute;
using namespace CppDebugger::SeverityValues;


/***** POPULATE FUNCTIONS *****/
/* Populates a given VKBufferCreateInfo struct. */
void populate_buffer_info(VkBufferCreateInfo& buffer_info, VkDeviceSize n_bytes, VkBufferUsageFlags usage_flags, VkSharingMode sharing_mode, VkBufferCreateFlags create_flags) {
    DENTER("populate_buffer_info");

    // Only set to default
    buffer_info = {};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;

    // Set the parameters passed
    buffer_info.size = n_bytes;
    buffer_info.usage = usage_flags;
    buffer_info.sharingMode = sharing_mode;
    buffer_info.flags = create_flags;

    // Done
    DRETURN;
}

/* Populates a given VkMemoryAllocateInfo struct. */
void populate_allocate_info(VkMemoryAllocateInfo& allocate_info, uint32_t memory_type, VkDeviceSize n_bytes) {
    DENTER("populate_allocate_info");

    // Set to default & define the stryct type
    allocate_info = {};
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

    // Set the allocation size of the memory
    allocate_info.allocationSize = n_bytes;
    // Set the type of the memory to allocate
    allocate_info.memoryTypeIndex = memory_type;

    // Done
    DRETURN;
}

/* Populates a given VkMappedMemoryRange struct. */
void populate_memory_range(VkMappedMemoryRange& memory_range, VkDeviceMemory vk_memory, VkDeviceSize vk_memory_offset, VkDeviceSize vk_memory_size) {
    DENTER("populate_memory_range");

    // Set to default
    memory_range = {};
    memory_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;

    // Tell it the memory and which part of the device memory is used
    memory_range.memory = vk_memory;
    memory_range.offset = vk_memory_offset;
    memory_range.size = vk_memory_size;

    // Done, return
    DRETURN;
}





/***** BUFFER CLASS *****/
/* Private constructor for the Buffer class, which takes the buffer, the buffer's size and the properties of the pool's memory. */
Buffer::Buffer(VkBuffer buffer, VkBufferUsageFlags vk_usage_flags, VkDeviceMemory vk_memory, VkDeviceSize memory_offset, VkDeviceSize memory_size, VkDeviceSize req_memory_size, VkMemoryPropertyFlags memory_properties) :
    vk_buffer(buffer),
    vk_usage_flags(vk_usage_flags),
    vk_memory(vk_memory),
    vk_memory_offset(memory_offset),
    vk_memory_size(memory_size),
    vk_req_memory_size(req_memory_size),
    vk_memory_properties(memory_properties)
{}



/* Maps the buffer to host-memory so it can be written to. Only possible if the VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT is set for the memory of this buffer's pool. Note that the memory is NOT automatically unmapped if the Buffer object is destroyed. */
void  Buffer::map(const GPU& gpu, void** mapped_memory) const {
    DENTER("Compute::Buffer::map");

    // If this buffer does not have the host bit set, then we stop immediatement
    if (!(this->vk_memory_properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
        DLOG(fatal, "Cannot map a buffer that is not visible by the CPU.");
    }

    // Now, we map the memory to a bit of host-side memory
    VkResult vk_result;
    if ((vk_result = vkMapMemory(gpu, this->vk_memory, this->vk_memory_offset, this->vk_req_memory_size, 0, mapped_memory)) != VK_SUCCESS) {
        DLOG(fatal, "Could not map buffer memory to CPU-memory: " + vk_error_map[vk_result]);
    }

    // Done
    DRETURN;
}

/* Flushes all unflushed memory operations done on mapped memory. If the memory of this buffer has VK_MEMORY_PROPERTY_HOST_COHERENT_BIT set, then nothing is done as the memory is already automatically flushed. */
void  Buffer::flush(const GPU& gpu) const {
    DENTER("Compute::Buffer::flush");

    // If this buffer is coherent, quite immediately
    if (!(this->vk_memory_properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
        DRETURN;
    }

    // Prepare the call to the flush function
    VkMappedMemoryRange memory_range;
    populate_memory_range(memory_range, this->vk_memory, this->vk_memory_offset, this->vk_req_memory_size);

    // Do the flush call
    VkResult vk_result;
    if ((vk_result = vkFlushMappedMemoryRanges(gpu, 1, &memory_range)) != VK_SUCCESS) {
        DLOG(fatal, "Could not flush mapped buffer memory: " + vk_error_map[vk_result]);
    }

    // Done
    DRETURN;
}

/* Unmaps buffer's memory. */
void  Buffer::unmap(const GPU& gpu) const {
    DENTER("Compute::Buffer::unmap");

    // Simply call unmap, done
    vkUnmapMemory(gpu, this->vk_memory);

    DRETURN;
}



/* Copies this buffer's content to another given buffer. The given command pool (which must be a pool for the memory-enabled queue) is used to schedule the copy. Note that the given buffer needn't come from the same memory pool. */
void Buffer::copyto(VkQueue vk_queue, CommandBuffer& command_buffer, const Buffer& destination, bool wait_queue_idle) const {
    DENTER("Compute::Buffer::copyto");

    // First, check if the destination buffer is large enough
    if (destination.vk_memory_size < this->vk_memory_size) {
        DLOG(fatal, "Cannot copy " + std::to_string(this->vk_memory_size) + " bytes to buffer of only " + std::to_string(destination.vk_memory_size) + " bytes.");
    }

    // Next, make sure they have the required flags
    if (!(this->vk_usage_flags & VK_BUFFER_USAGE_TRANSFER_SRC_BIT)) {
        DLOG(fatal, "Source buffer does not have VK_BUFFER_USAGE_TRANSFER_SRC_BIT-flag set.");
    }
    if (!(destination.vk_usage_flags & VK_BUFFER_USAGE_TRANSFER_DST_BIT)) {
        DLOG(fatal, "Destination buffer does not have VK_BUFFER_USAGE_TRANSFER_DST_BIT-flag set.");
    }

    // Next, start recording the given command buffer, and we'll tell Vulkan we use this recording only once
    command_buffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    // We schedule the copy by populating a struct
    VkBufferCopy copy_region{};
    copy_region.srcOffset = 0;
    copy_region.dstOffset = 0;
    copy_region.size = this->vk_memory_size;
    vkCmdCopyBuffer(command_buffer, this->vk_buffer, destination.vk_buffer, 1, &copy_region);

    // Since that's all, submit the queue. Note that we only return once the copy is 
    command_buffer.end(vk_queue, wait_queue_idle);

    DRETURN;
}





/***** MEMORYPOOL CLASS *****/
/* Constructor for the MemoryPool class, which takes a device to allocate on, the type of memory we will allocate on and the total size of the allocated block. */
MemoryPool::MemoryPool(const GPU& gpu, uint32_t memory_type, VkDeviceSize n_bytes, VkMemoryPropertyFlags memory_properties) :
    gpu(gpu),
    vk_memory_type(memory_type),
    vk_memory_size(n_bytes),
    vk_memory_properties(memory_properties),
    vk_free_blocks({ MemoryPool::FreeBlock({ 0, this->vk_memory_size }) })
{
    DENTER("Compute::MemoryPool::MemoryPool");
    DLOG(info, "Initializing MemoryPool...");
    DINDENT;



    #ifndef NDEBUG
    DLOG(info, "Validating memory requirements...");

    // Get the memory properties of the GPU
    VkPhysicalDeviceMemoryProperties gpu_properties;
    vkGetPhysicalDeviceMemoryProperties(gpu, &gpu_properties);

    // Check if the chosen memory type has the desired properties
    if (memory_type >= 32) {
        DLOG(fatal, "Memory type is out of range (0 <= memory_type < 32)");
    }
    if ((gpu_properties.memoryTypes[memory_type].propertyFlags & this->vk_memory_properties) != this->vk_memory_properties) {
        DLOG(fatal, "Chosen memory type with index " + std::to_string(memory_type) + " does not support the specified memory properties.");
    }

    #endif
    


    // Allocate the block of memory that we shall use
    DLOG(info, "Allocating memory on device '" + gpu.name() + "'...");
    VkMemoryAllocateInfo allocate_info;
    populate_allocate_info(allocate_info, memory_type, n_bytes);

    // Do the allocation
    VkResult vk_result;
    if ((vk_result = vkAllocateMemory(this->gpu, &allocate_info, nullptr, &this->vk_memory)) != VK_SUCCESS) {
        DLOG(fatal, "Could not allocate memory on device: " + vk_error_map[vk_result]);
    }



    DDEDENT;
    DLEAVE;
}

/* Copy constructor for the MemoryPool class, which is deleted. */
MemoryPool::MemoryPool(const MemoryPool& other) :
    gpu(other.gpu),
    vk_memory_type(other.vk_memory_type),
    vk_memory_size(other.vk_memory_size),
    vk_memory_properties(other.vk_memory_properties),
    vk_free_blocks({ MemoryPool::FreeBlock({ 0, this->vk_memory_size }) })
{
    DENTER("MemoryPool::MemoryPool(copy)");

    // Allocate a new block of memory that we shall use, with the proper size
    VkMemoryAllocateInfo allocate_info;
    populate_allocate_info(allocate_info, this->vk_memory_type, this->vk_memory_size);

    // Do the allocation
    VkResult vk_result;
    if ((vk_result = vkAllocateMemory(this->gpu, &allocate_info, nullptr, &this->vk_memory)) != VK_SUCCESS) {
        DLOG(fatal, "Could not allocate memory on device: " + vk_error_map[vk_result]);
    }

    // Do not copy handles with us, as that doesn't really make a whole lotta sense

    DLEAVE;
}

/* Move constructor for the MemoryPool class. */
MemoryPool::MemoryPool(MemoryPool&& other):
    gpu(other.gpu),
    vk_memory(other.vk_memory),
    vk_memory_type(other.vk_memory_type),
    vk_memory_size(other.vk_memory_size),
    vk_memory_properties(other.vk_memory_properties),
    vk_used_blocks(std::move(other.vk_used_blocks)),
    vk_free_blocks(std::move(other.vk_free_blocks))
{
    // Set the other's memory to nullptr to avoid deallocation
    other.vk_memory = nullptr;
    other.vk_used_blocks.clear();
}

/* Destructor for the MemoryPool class. */
MemoryPool::~MemoryPool() {
    DENTER("Compute::MemoryPool::~MemoryPool");
    DLOG(info, "Cleaning MemoryPool...");
    DINDENT;

    // Delete all buffers
    if (this->vk_used_blocks.size() > 0) {
        DLOG(info, "Deallocating buffers...");
        for (const std::pair<BufferHandle, UsedBlock>& p : this->vk_used_blocks) {
            vkDestroyBuffer(this->gpu, p.second.vk_buffer, nullptr);
        }
    }

    // Deallocate the allocated memory
    if (this->vk_memory != nullptr) {
        DLOG(info, "Deallocating device memory...");
        vkFreeMemory(this->gpu, this->vk_memory, nullptr);
    }

    DDEDENT;
    DLEAVE;
}



/* Returns a reference to the internal buffer with the given handle. Always performs out-of-bounds checking. */
Buffer MemoryPool::at(BufferHandle buffer) const {
    DENTER("Compute::MemoryPool::at");

    // Check if it exists
    if (this->vk_used_blocks.find(buffer) == this->vk_used_blocks.end()) {
        DLOG(fatal, "Buffer with handle '" + std::to_string(buffer) + "' does not exist.");
    }
    DRETURN init_buffer(this->vk_used_blocks.at(buffer), this->vk_memory, this->vk_memory_properties);
}



/* Tries to get a new buffer from the pool of the given size and with the given flags. Applies extra checks if NDEBUG is not defined. */
BufferHandle MemoryPool::allocate(VkDeviceSize n_bytes, VkBufferUsageFlags usage_flags, VkSharingMode sharing_mode, VkBufferCreateFlags create_flags) {
    DENTER("Compute::MemoryPool::allocate");

    // First, prepare the buffer info struct
    VkBufferCreateInfo buffer_info;
    populate_buffer_info(buffer_info, n_bytes, usage_flags, sharing_mode, create_flags);

    // Pick a suitable memory location for this buffer; either as a new buffer or a previously deallocated one
    BufferHandle result = 0;
    bool unique = false;
    while (!unique) {
        unique = true;
        for (const std::pair<BufferHandle, UsedBlock>& p : this->vk_used_blocks) {
            if (result == MemoryPool::NullHandle || result == p.first) {
                // If result is the maximum value, then throw an error
                if (result == std::numeric_limits<BufferHandle>::max()) {
                    DLOG(fatal, "All buffer handles have been used.");
                }

                // Otherwise, increment and re-try
                ++result;
                unique = false;
                break;
            }
        }
    }
    
    // Reserve space in our map
    this->vk_used_blocks.insert(std::make_pair(result, UsedBlock()));
    UsedBlock& block = this->vk_used_blocks.at(result);

    // Next, create the buffer struct
    VkResult vk_result;
    if ((vk_result = vkCreateBuffer(this->gpu, &buffer_info, nullptr, &block.vk_buffer)) != VK_SUCCESS) {
        DLOG(fatal, "Could not create buffer: " + vk_error_map[vk_result]);
    }

    // Get the memory requirements of the buffer
    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(this->gpu, block.vk_buffer, &mem_requirements);

    #ifndef NDEBUG
    // Make sure the memory requirements of the buffer and that of the allocated memory align
    if (!(mem_requirements.memoryTypeBits & (1 << this->vk_memory_type))) {
        DLOG(fatal, "Buffer is not compatible with this memory pool.");
    }
    #endif

    // Next, we find the first free block available of at least the required size
    VkDeviceSize offset;
    VkDeviceSize total_free = 0;
    bool found = false;
    for (size_t i = 0; i < this->vk_free_blocks.size(); i++) {
        total_free += this->vk_free_blocks[i].length;
        if (this->vk_free_blocks[i].length >= mem_requirements.size) {
            // This block has enough memory; mark its starting position as the used one
            offset = this->vk_free_blocks[i].start;

            // Shrink the block to mark this spot as used
            this->vk_free_blocks[i].start += mem_requirements.size;
            this->vk_free_blocks[i].length -= mem_requirements.size;

            // If the resulting space is no bytes, then remove the free block
            if (this->vk_free_blocks[i].length == 0) {
                this->vk_free_blocks.erase(i);
            }

            // Done
            found = true;
            break;
        }
    }
    if (!found) {
        // No memory was available; is this due to memory or to bad fragmentation?
        #ifdef NDEBUG
        DLOG(fatal, "Could not allocate new buffer");
        #else
        if (mem_requirements.size > total_free) {
            DLOG(fatal, "Could not allocate new buffer: not enough space left in pool (need " + std::to_string(mem_requirements.size) + " bytes, but " + std::to_string(total_free) + " bytes free)");
        } else {
            DLOG(fatal, "Could not allocate new buffer: no large enough block found, but we do have enough memory available; call defrag() first");
        }
        #endif
    }

    // With the offset, bind the memory to the new buffer
    if ((vk_result = vkBindBufferMemory(this->gpu, block.vk_buffer, this->vk_memory, offset)) != VK_SUCCESS) {
        DLOG(fatal, "Could not bind buffer memory: " + vk_error_map[vk_result]);
    }

    // Store the chosen parameters in the buffer for easy re-creation
    block.start = offset;
    block.length = n_bytes;
    block.req_length = mem_requirements.size;
    block.vk_usage_flags = usage_flags;
    block.vk_create_flags = create_flags;
    block.vk_sharing_mode = sharing_mode;

    // Done, return the handle
    DRETURN result;
}

/* Deallocates the buffer with the given handle. Does not throw an error if the handle doesn't exist, unless NDEBUG is not defined. */
void MemoryPool::deallocate(BufferHandle buffer) {
    DENTER("Compute::MemoryPool::deallocate");

    // First, try to fetch the given buffer
    std::unordered_map<BufferHandle, UsedBlock>::iterator iter = this->vk_used_blocks.find(buffer);
    if (iter == this->vk_used_blocks.end()) {
        DLOG(fatal, "Buffer with handle '" + std::to_string(buffer) + "' does not exist.");
    }

    // Copy the block...
    UsedBlock block = (*iter).second;
    // ...then safely delete it from the list
    vkDestroyBuffer(this->gpu, block.vk_buffer, nullptr);
    this->vk_used_blocks.erase(iter);

    // Get the start & offset of the buffer
    VkDeviceSize buffer_start = block.start;
    VkDeviceSize buffer_length = block.req_length;

    // Generate a new free block for the memory released by this buffer. We insert it sorted.
    bool inserted = false;
    for (size_t i = 0; i < this->vk_free_blocks.size(); i++) {
        // Get the start & offset for this free block
        VkDeviceSize free_start = this->vk_free_blocks[i].start;
        VkDeviceSize free_length = this->vk_free_blocks[i].length;

        #ifndef NDEBUG
        // Sanity check
        if (free_start == buffer_start) {
            DLOG(fatal, "Free block " + std::to_string(i) + " has same offset as previously allocated buffer");
        }
        #endif

        // Check if we should insert it here
        if (free_start > buffer_start) {
            // It should become the new i'th block

            #ifndef NDEBUG

            // Couple of more sanity checks
            if (i > 0) {
                // Sanity check the previous buffer as well
                VkDeviceSize free_m1_start = this->vk_free_blocks[i - 1].start;
                VkDeviceSize free_m1_length = this->vk_free_blocks[i - 1].length;

                if (i > 0 && free_m1_start + free_m1_length > buffer_start) {
                    DLOG(fatal, "Free block " + std::to_string(i - 1) + " overlaps with previously allocated buffer (previous neighbour)");
                } else if (i > 0 && free_m1_start + free_m1_length > free_start) {
                    DLOG(fatal, "Free blocks " + std::to_string(i - 1) + " and " + std::to_string(i) + " overlap");
                }
            }
            // Sanity check the current buffer
            if (buffer_start + buffer_length > free_start) {
                DLOG(fatal, "Free block " + std::to_string(i) + " overlaps with previously allocated buffer (next neighbour)");
            } 
            
            #endif

            // However, first check if it makes more sense to merge with its neighbours first
            if (i > 0) {
                VkDeviceSize free_m1_start = this->vk_free_blocks[i - 1].start;
                VkDeviceSize free_m1_length = this->vk_free_blocks[i - 1].length;

                if (free_m1_start + free_m1_length == buffer_start && buffer_start + buffer_length == free_start) {
                    // The buffer happens to precisely fill the gap between two blocks; merge all memory into one free block
                    this->vk_free_blocks[i - 1].length += free_length;
                    this->vk_free_blocks.erase(i);
                    inserted = true;
                } else if (free_m1_start + free_m1_length == buffer_start) {
                    // The buffer is only mergeable with the previous block
                    this->vk_free_blocks[i - 1].length += buffer_length;
                    inserted = true;
                }
            }

            // Examine the next block in its onesy 
            if (buffer_start + buffer_length == free_start) {
                // The buffer is only mergeable with the next block
                this->vk_free_blocks[i].start -= buffer_length;
                this->vk_free_blocks[i].length += buffer_length;
                inserted = true;
            }
            
            // If not yet inserted, then we want to add a new free block
            if (!inserted) {
                // Not mergeable; insert a new block by moving all blocks to the right
                this->vk_free_blocks.push_back(this->vk_free_blocks[this->vk_free_blocks.size() - 1]);
                for (size_t j = i; j < this->vk_free_blocks.size() - 1; j++) {
                    this->vk_free_blocks[j + 1] = this->vk_free_blocks[j];
                }
                this->vk_free_blocks[i] = MemoryPool::FreeBlock({ buffer_start, buffer_length });
                inserted = true;
            }

            // We always want to stop, though
            break;
        }
    }

    // If we did not insert, then append it as a free block at the end
    if (!inserted) {
        // If there is a previous block and its mergeable, do that
        if (this->vk_free_blocks.size() > 0) {
            VkDeviceSize free_start = this->vk_free_blocks[this->vk_free_blocks.size() - 1].start;
            VkDeviceSize free_length = this->vk_free_blocks[this->vk_free_blocks.size() - 1].length;

            if (free_start + free_length == buffer_start) {
                // It matches; merge the old block
                this->vk_free_blocks[this->vk_free_blocks.size() - 1].length += buffer_length;
                inserted = true;
            }
        }

        // If we didn't merge it, append it as a new block
        if (!inserted) {
            this->vk_free_blocks.push_back(MemoryPool::FreeBlock({ buffer_start, buffer_length }));
            inserted = true;
        }
    }

    // Done
    DRETURN;
}



/* Defragements the entire pool, aligning all buffers next to each other in memory to create a maximally sized free block. Note that existing handles will remain valid. */
void MemoryPool::defrag() {
    DENTER("Compute::MemoryPool::defrag");

    // We loop through all internal blocks
    VkResult vk_result;
    VkDeviceSize offset = 0;
    for (const std::pair<BufferHandle, UsedBlock>& p : this->vk_used_blocks) {
        // Get a reference to the block
        UsedBlock& block = this->vk_used_blocks.at(p.first);

        // Deallocate the current buffer
        vkDestroyBuffer(this->gpu, block.vk_buffer, nullptr);

        // Prepare a new struct for reallocation
        VkBufferCreateInfo buffer_info;
        populate_buffer_info(buffer_info, block.length, block.vk_usage_flags, block.vk_sharing_mode, block.vk_create_flags);

        // Create a new vk_buffer
        if ((vk_result = vkCreateBuffer(this->gpu, &buffer_info, nullptr, &block.vk_buffer)) != VK_SUCCESS) {
            DLOG(fatal, "Could not re-create VkBuffer object: " + vk_error_map[vk_result]);
        }

        // Get the memory requirements for this new buffer
        VkMemoryRequirements mem_requirements;
        vkGetBufferMemoryRequirements(this->gpu, block.vk_buffer, &mem_requirements);

        // Be sure that we still make it; due to aligning we might get a different size
        if (offset + mem_requirements.size > this->vk_memory_size) {
            DLOG(fatal, "Could not defrag buffer: memory requirements changed (need " + std::to_string(mem_requirements.size) + " bytes, but " + std::to_string(this->vk_memory_size) + " bytes free)");
        }

        // Bind new memory for, at the start of this index.
        if ((vk_result = vkBindBufferMemory(this->gpu, block.vk_buffer, this->vk_memory, offset)) != VK_SUCCESS) {
            DLOG(fatal, "Could not re-bind memory to buffer: " + vk_error_map[vk_result]);
        }

        // Update the offset & possibly size in the block
        block.start = offset;
        block.length = mem_requirements.size;

        // Increment the offset for the next buffer
        offset += mem_requirements.size;
    }

    // Once done, re-initialize the list of free blocks
    this->vk_free_blocks.clear();
    if (offset < this->vk_memory_size) {
        this->vk_free_blocks.push_back(MemoryPool::FreeBlock({ offset, this->vk_memory_size - offset }));
    }

    // Done
    DRETURN;
}



/* Swap operator for the MemoryPool class. */
void Compute::swap(MemoryPool& mp1, MemoryPool& mp2) {
    using std::swap;

    DENTER("Compute::swap(MemoryPool)");

    #ifndef NDEBUG
    // If the GPU is not the same, then initialize to all nullptrs and everything
    if (mp1.gpu != mp2.gpu) {
        DLOG(fatal, "Cannot swap memory pools with different GPUs");
    }
    #endif

    // Swap EVERYTHING but the GPU
    swap(mp1.vk_memory, mp2.vk_memory);
    swap(mp1.vk_memory_type, mp2.vk_memory_type);
    swap(mp1.vk_memory_size, mp2.vk_memory_size);
    swap(mp1.vk_memory_properties, mp2.vk_memory_properties),
    swap(mp1.vk_used_blocks, mp2.vk_used_blocks);
    swap(mp1.vk_free_blocks, mp2.vk_free_blocks);

    DRETURN;
}



/* Static function that helps users decide the best memory queue. */
uint32_t MemoryPool::select_memory_type(const GPU& gpu, VkBufferUsageFlags usage_flags, VkMemoryPropertyFlags memory_properties, VkSharingMode sharing_mode, VkBufferCreateFlags create_flags) {
    DENTER("Compute::MemoryPool::select_memory_type");

    // Get the available memory in the internal device
    VkPhysicalDeviceMemoryProperties gpu_properties;
    vkGetPhysicalDeviceMemoryProperties(gpu, &gpu_properties);

    // Next, temporarily create a buffer to discover its type
    VkBufferCreateInfo buffer_info;
    populate_buffer_info(buffer_info, 16, usage_flags, sharing_mode, create_flags);

    // Create the buffer
    VkResult vk_result;
    VkBuffer dummy;
    if ((vk_result = vkCreateBuffer(gpu, &buffer_info, nullptr, &dummy)) != VK_SUCCESS) {
        DLOG(fatal, "Could not allocate temporary dummy buffer: " + vk_error_map[vk_result]);
    }

    // Ge the memory requirements of the buffer
    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(gpu, dummy, &mem_requirements);

    // We can already destroy the buffer
    vkDestroyBuffer(gpu, dummy, nullptr);

    // Try to find suitable memory (i.e., check if the device has enough memory bits(?) and if the required properties match)
    for (uint32_t i = 0; i < gpu_properties.memoryTypeCount; i++) {
        if (mem_requirements.memoryTypeBits & (1 << i) && (gpu_properties.memoryTypes[i].propertyFlags & memory_properties) == memory_properties) {
            DRETURN i;
        }
    }

    // Didn't find any
    DLOG(fatal, "No suitable memory on device for given buffer configuration.");
    DRETURN 0;
}
