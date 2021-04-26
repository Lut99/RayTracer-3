/* MEMORY POOL.cpp
 *   by Lut99
 *
 * Created:
 *   25/04/2021, 11:36:42
 * Last edited:
 *   26/04/2021, 17:09:50
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Contains the MemoryPool class, which is in charge of a single pool of
 *   GPU memory that it can hand out to individual buffers.
**/

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





/***** BUFFER CLASS *****/
/* Private, constructor for the Buffer class, which takes the buffer and size. */
Buffer::Buffer(VkBuffer buffer, VkDeviceSize buffer_size) :
    vk_buffer(buffer),
    vk_buffer_size(buffer_size)
{}





/***** MEMORYPOOL CLASS *****/
/* Constructor for the MemoryPool class, which takes a device to allocate on, the type of memory we will allocate on and the total size of the allocated block. */
MemoryPool::MemoryPool(const GPU& gpu, uint32_t memory_type, VkDeviceSize n_bytes) :
    gpu(gpu),
    vk_memory_type(memory_type),
    vk_memory_size(n_bytes),
    vk_free_blocks({ MemoryPool::FreeBlock({ 0, this->vk_memory_size }) })
{
    DENTER("Compute::MemoryPool::MemoryPool");
    DLOG(info, "Initializing MemoryPool...");
    DINDENT;
    


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

/* Move constructor for the MemoryPool class. */
MemoryPool::MemoryPool(MemoryPool&& other):
    gpu(other.gpu),
    vk_memory(other.vk_memory),
    vk_memory_type(other.vk_memory_type),
    vk_memory_size(other.vk_memory_size),
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
    DRETURN init_buffer(this->vk_used_blocks.at(buffer));
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
            if (result == p.first) {
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
    VkDeviceSize buffer_length = block.length;

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



/* Move constructor for the MemoryPool class. */
MemoryPool& MemoryPool::operator=(MemoryPool&& other) {
    if (this != &other) { swap(*this, other); }
    return *this;
}

/* Swap operator for the MemoryPool class. */
void Compute::swap(MemoryPool& mp1, MemoryPool& mp2) {
    using std::swap;

    DENTER("Compute::swap(MemoryPool)");

    #ifndef NDEBUG
    // If the GPU is not the same, then initialize to all nullptrs and everything
    if (mp1.gpu.name() != mp2.gpu.name()) {
        DLOG(fatal, "Cannot swap memory pools with different GPUs");
    }
    #endif

    // Swap EVERYTHING but the GPU
    swap(mp1.vk_memory, mp2.vk_memory);
    swap(mp1.vk_memory_type, mp2.vk_memory_type);
    swap(mp1.vk_memory_size, mp2.vk_memory_size);
    swap(mp1.vk_used_blocks, mp2.vk_used_blocks);
    swap(mp1.vk_free_blocks, mp2.vk_free_blocks);

    DRETURN;
}



/* Static function that helps users decide the best memory queue. */
uint32_t MemoryPool::select_memory_type(const GPU& gpu, VkMemoryPropertyFlags mem_properties, VkBufferUsageFlags usage_flags, VkSharingMode sharing_mode, VkBufferCreateFlags create_flags) {
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
        if (mem_requirements.memoryTypeBits & (1 << i) && (gpu_properties.memoryTypes[i].propertyFlags & mem_properties) == mem_properties) {
            DRETURN i;
        }
    }

    // Didn't find any
    DLOG(fatal, "No suitable memory on device for given buffer configuration.");
    DRETURN 0;
}
