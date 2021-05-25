/* MEMORY POOL.cpp
 *   by Lut99
 *
 * Created:
 *   25/04/2021, 11:36:42
 * Last edited:
 *   25/05/2021, 18:14:13
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
#include "tools/Common.hpp"

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

/* Populates a given VkImageCreateInfo struct. */
void populate_image_info(VkImageCreateInfo& image_info, const VkExtent3D& image_size, VkFormat image_format, VkImageLayout image_layout, VkBufferUsageFlags usage_flags, VkSharingMode sharing_mode, VkBufferCreateFlags create_flags) {
    DENTER("populate_image_info");

    // Only set to default
    image_info = {};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

    // Set the parameters passed
    image_info.extent = image_size;
    image_info.format = image_format;
    image_info.initialLayout = image_layout;
    image_info.usage = usage_flags;
    image_info.sharingMode = sharing_mode;
    image_info.flags = create_flags;
    
    // Set the image-specific parameters
    image_info.arrayLayers = 1;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.mipLevels = 1;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;

    // Since the sharing mode will be exlusive by default, let's for now hardcode this to not have a queue family
    image_info.queueFamilyIndexCount = 0;

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
Buffer::Buffer(BufferHandle handle, VkBuffer buffer, VkBufferUsageFlags vk_usage_flags, VkSharingMode vk_sharing_mode, VkBufferCreateFlags vk_create_flags, VkDeviceMemory vk_memory, VkDeviceSize memory_offset, VkDeviceSize memory_size, VkDeviceSize req_memory_size, VkMemoryPropertyFlags memory_properties) :
    vk_handle(handle),
    vk_buffer(buffer),
    vk_usage_flags(vk_usage_flags),
    vk_sharing_mode(vk_sharing_mode),
    vk_create_flags(vk_create_flags),
    vk_memory(vk_memory),
    vk_memory_offset(memory_offset),
    vk_memory_size(memory_size),
    vk_req_memory_size(req_memory_size),
    vk_memory_properties(memory_properties)
{}



/* Sets n_bytes data to this buffer using an intermediate staging buffer. The staging buffer is copied using the given command buffer on the given queue. */
void Buffer::set(const GPU& gpu, const Buffer& staging_buffer, const CommandBuffer& command_buffer, VkQueue vk_queue, void* data, uint32_t n_bytes) const {
    DENTER("Compute::Buffer::set");

    // First, map the staging buffer to an CPU-reachable area
    void* mapped_area;
    staging_buffer.map(gpu, &mapped_area);

    // Next, copy the data to the buffer
    memcpy(mapped_area, data, n_bytes);

    // Flush and then unmap the staging buffer
    staging_buffer.flush(gpu);
    staging_buffer.unmap(gpu);

    // Use the command buffer to copy the data around
    staging_buffer.copyto(command_buffer, vk_queue, *this, (VkDeviceSize) n_bytes);

    // Done
    DRETURN;
}

/* Gets n_bytes data from this buffer using an intermediate staging buffer. The buffers are copied over using the given command buffer on the given queue. The result is put in the given pointer. */
void Buffer::get(const GPU& gpu, const Buffer& staging_buffer, const CommandBuffer& command_buffer, VkQueue vk_queue, void* data, uint32_t n_bytes) const {
    DENTER("Compute::Buffer::set");

    // First, copy the data we have to the staging buffer
    this->copyto(command_buffer, vk_queue, staging_buffer, (VkDeviceSize) n_bytes);

    // Then, map the staging buffer to an CPU-reachable area
    void* mapped_area;
    staging_buffer.map(gpu, &mapped_area);

    // Next, copy the data to a user-defined location
    memcpy(data, mapped_area, n_bytes);

    // Unmap the staging buffer. No need to flush cuz we didn't change anything
    staging_buffer.unmap(gpu);

    // Done
    DRETURN;
}



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
void Buffer::copyto(const CommandBuffer& command_buffer, VkQueue vk_queue, const Buffer& destination, VkDeviceSize n_bytes, VkDeviceSize target_offset, bool wait_queue_idle) const {
    DENTER("Compute::Buffer::copyto");

    // If the number of bytes to transfer is the max, default to the buffer size
    if (n_bytes == numeric_limits<VkDeviceSize>::max()) {
        n_bytes = this->vk_memory_size;
    }

    // First, check if the destination buffer is large enough
    if (destination.vk_memory_size - target_offset < n_bytes) {
        DLOG(fatal, "Cannot copy " + Tools::bytes_to_string(n_bytes) + " to buffer of only " + Tools::bytes_to_string(destination.vk_memory_size) + " (with offset=" + std::to_string(target_offset) + ").");
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
    copy_region.dstOffset = target_offset;
    copy_region.size = n_bytes;
    vkCmdCopyBuffer(command_buffer, this->vk_buffer, destination.vk_buffer, 1, &copy_region);

    // Since that's all, submit the queue. Note that we only return once the copy is 
    command_buffer.end(vk_queue, wait_queue_idle);

    DRETURN;
}





/***** IMAGE CLASS *****/
/* Private constructor for the Buffer class, which takes the buffer, the buffer's size and the properties of the pool's memory. */
Image::Image(ImageHandle handle, VkImage image, VkExtent2D vk_extent, VkFormat vk_format, VkImageLayout vk_layout, VkImageUsageFlags vk_usage_flags, VkSharingMode vk_sharing_mode, VkImageCreateFlags vk_create_flags, VkDeviceMemory vk_memory, VkDeviceSize memory_offset, VkDeviceSize memory_size, VkDeviceSize req_memory_size, VkMemoryPropertyFlags memory_properties) :
    vk_handle(handle),
    vk_image(image),
    vk_extent(vk_extent),
    vk_format(vk_format),
    vk_layout(vk_layout),
    vk_usage_flags(vk_usage_flags),
    vk_sharing_mode(vk_sharing_mode),
    vk_create_flags(vk_create_flags),
    vk_memory(vk_memory),
    vk_memory_offset(memory_offset),
    vk_memory_size(memory_size),
    vk_req_memory_size(req_memory_size),
    vk_memory_properties(memory_properties)
{}





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

/* Copy constructor for the MemoryPool class. */
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
        for (const std::pair<MemoryHandle, UsedBlock*>& p : this->vk_used_blocks) {
            // Either destroy the buffer or the image
            if (p.second->type == MemoryBlockType::buffer) {
                vkDestroyBuffer(this->gpu, ((BufferBlock*) p.second)->vk_buffer, nullptr);
            } else if (p.second->type == MemoryBlockType::image) {
                vkDestroyImage(this->gpu, ((ImageBlock*) p.second)->vk_image, nullptr);
            }

            // Then destroy the block itself
            delete p.second;
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



/* Private helper function that actually performs memory allocation. Returns a reference to a UsedBlock that describes the block allocated. */
MemoryHandle MemoryPool::allocate_memory(MemoryBlockType type, VkDeviceSize n_bytes, const VkMemoryRequirements& mem_requirements) {
    DENTER("allocate_memory");

    // Pick a suitable memory location for this block in the array of used blocks; either as a new block or use the location of a previously allocated one
    MemoryHandle result = 0;
    bool unique = false;
    while (!unique) {
        unique = true;
        for (const std::pair<MemoryHandle, UsedBlock*>& p : this->vk_used_blocks) {
            if (result == MemoryPool::NullHandle || result == p.first) {
                // If result is the maximum value, then throw an error
                if (result == std::numeric_limits<MemoryHandle>::max()) {
                    DLOG(fatal, "Buffer handle overflow; cannot allocate more buffers.");
                }

                // Otherwise, increment and re-try
                ++result;
                unique = false;
                break;
            }
        }
    }
    // Reserve space in our map
    this->vk_used_blocks.insert(std::make_pair(result, type == MemoryBlockType::buffer ? (UsedBlock*) new BufferBlock() : (UsedBlock*) new ImageBlock()));
    UsedBlock* block = this->vk_used_blocks.at(result);

    #ifndef NDEBUG
    // Next, make sure the given memory requirements are aligning with our internal type
    if (!(mem_requirements.memoryTypeBits & (1 << this->vk_memory_type))) {
        DLOG(fatal, "Buffer is not compatible with this memory pool.");
    }
    #endif

    // Next, we find the first free block available of at least the required size
    VkDeviceSize offset;
    VkDeviceSize total_free = 0;
    bool found = false;
    for (size_t i = 0; i < this->vk_free_blocks.size(); i++) {
        // Compute how many bytes of alignment we need to take into account when looking at where this block starts
        VkDeviceSize align_bytes = mem_requirements.alignment - this->vk_free_blocks[i].start % mem_requirements.alignment;
        if (align_bytes == mem_requirements.alignment) { align_bytes = 0; }

        // Check if this block (aligned) is large enough
        if (this->vk_free_blocks[i].length >= align_bytes + mem_requirements.size) {
            // This block has enough memory; mark its starting position as the used one
            offset = align_bytes + this->vk_free_blocks[i].start;

            // Shrink the block to mark this spot as used
            this->vk_free_blocks[i].start += align_bytes + mem_requirements.size;
            this->vk_free_blocks[i].length -= align_bytes + mem_requirements.size;

            // If the resulting space is no bytes, then remove the free block
            if (this->vk_free_blocks[i].length == 0) {
                this->vk_free_blocks.erase(i);
            }

            // Done
            found = true;
            break;
        }

        // Keep track of how many free bytes there are in total
        total_free += this->vk_free_blocks[i].length;
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

    // Store the chosen parameters in the buffer for easy re-creation
    block->start = offset;
    block->length = n_bytes;
    block->req_length = mem_requirements.size;

    // We're done, so return the block for further initialization of the image or buffer
    DRETURN result;
}



/* Tries to get a new buffer from the pool of the given size and with the given flags, returning only its handle. Applies extra checks if NDEBUG is not defined. */
BufferHandle MemoryPool::allocate_buffer_h(VkDeviceSize n_bytes, VkBufferUsageFlags usage_flags, VkSharingMode sharing_mode, VkBufferCreateFlags create_flags)  {
    DENTER("Compute::MemoryPool::allocate_buffer_h");

    // First, prepare the buffer info struct
    VkBufferCreateInfo buffer_info;
    populate_buffer_info(buffer_info, n_bytes, usage_flags, sharing_mode, create_flags);

    // Next, create the buffer struct
    VkResult vk_result;
    VkBuffer buffer;
    if ((vk_result = vkCreateBuffer(this->gpu, &buffer_info, nullptr, &buffer)) != VK_SUCCESS) {
        DLOG(fatal, "Could not create buffer: " + vk_error_map[vk_result]);
    }

    // Get the memory requirements of the buffer
    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(this->gpu, buffer, &mem_requirements);

    // Use the helper function to do the allocation
    BufferHandle result = this->allocate_memory(MemoryBlockType::buffer, n_bytes, mem_requirements);
    BufferBlock* block = (BufferBlock*) this->vk_used_blocks.at(result);

    // With the block, bind the memory to the new buffer
    if ((vk_result = vkBindBufferMemory(this->gpu, buffer, this->vk_memory, block->start)) != VK_SUCCESS) {
        DLOG(fatal, "Could not bind buffer memory: " + vk_error_map[vk_result]);
    }

    // Next, populate the buffer parts of this block
    block->vk_buffer = buffer;
    block->vk_usage_flags = usage_flags;
    block->vk_create_flags = create_flags;
    block->vk_sharing_mode = sharing_mode;

    // Done, so return the handle
    DRETURN result;
}

/* Tries to get a new image from the pool of the given sizes and with the given flags, returning only its handle. Applies extra checks if NDEBUG is not defined. */
ImageHandle MemoryPool::allocate_image_h(uint32_t width, uint32_t height, VkFormat image_format, VkImageLayout image_layout, VkImageUsageFlags usage_flags, VkSharingMode sharing_mode, VkImageCreateFlags create_flags) {
    DENTER("Compute::MemoryPool::allocate_image_h");

    // First, define the width & height as a 3D extent
    VkExtent3D image_size = {};
    image_size.width = width;
    image_size.height = height;
    image_size.depth = 1;

    // Next, populate the create info for the image
    VkImageCreateInfo image_info;
    populate_image_info(image_info, image_size, image_format, image_layout, usage_flags, sharing_mode, create_flags);

    // Create the image
    VkResult vk_result;
    VkImage image;
    if ((vk_result = vkCreateImage(this->gpu, &image_info, nullptr, &image)) != VK_SUCCESS) {
        DLOG(fatal, "Could not create image: " + vk_error_map[vk_result]);
    }

    // Get the memory requirements of the image
    VkMemoryRequirements mem_requirements;
    vkGetImageMemoryRequirements(this->gpu, image, &mem_requirements);

    // Next, allocate memory for the image
    ImageHandle result = this->allocate_memory(MemoryBlockType::image, 3 * width * height * sizeof(uint8_t), mem_requirements);
    ImageBlock* block = (ImageBlock*) this->vk_used_blocks.at(result);

    // Bind that memory to the image
    if ((vk_result = vkBindImageMemory(this->gpu, image, this->vk_memory, block->start)) != VK_SUCCESS) {
        DLOG(fatal, "Could not bind image memory: " + vk_error_map[vk_result]);
    }

    // Populate the rest of the block before terminating
    block->vk_image = image;
    block->vk_extent = image_size;
    block->vk_format = image_format;
    block->vk_layout = image_layout;
    block->vk_usage_flags = usage_flags;
    block->vk_create_flags = create_flags;
    block->vk_sharing_mode = sharing_mode;

    // When done, return the handle
    DRETURN result;
}

/* Deallocates the buffer or image with the given handle. Does not throw an error if the handle doesn't exist, unless NDEBUG is not defined. */
void MemoryPool::deallocate(MemoryHandle handle) {
    DENTER("Compute::MemoryPool::deallocate");

    // First, try to fetch the given buffer
    std::unordered_map<MemoryHandle, UsedBlock*>::iterator iter = this->vk_used_blocks.find(handle);
    if (iter == this->vk_used_blocks.end()) {
        DLOG(fatal, "Object with handle '" + std::to_string(handle) + "' does not exist.");
    }

    // Copy the block...
    UsedBlock* block = (*iter).second;
    // ...then safely delete it from the list
    if (block->type == MemoryBlockType::buffer) {
        vkDestroyBuffer(this->gpu, ((BufferBlock*) block)->vk_buffer, nullptr);
    } else if (block->type == MemoryBlockType::image) {
        vkDestroyImage(this->gpu, ((ImageBlock*) block)->vk_image, nullptr);
    }
    this->vk_used_blocks.erase(iter);

    // Get the start & offset of the buffer
    VkDeviceSize buffer_start = block->start;
    VkDeviceSize buffer_length = block->req_length;

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
                    this->vk_free_blocks[i - 1].length += buffer_length + free_length;
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
                this->vk_free_blocks.resize(this->vk_free_blocks.size() + 1);
                for (size_t j = i; j < this->vk_free_blocks.size() - 1; j++) {
                    this->vk_free_blocks[j + 1] = this->vk_free_blocks[j];
                }

                // With space created, insert it
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

    // Deallocate the block itself
    delete block;

    // Done
    DRETURN;
}



/* Defragements the entire pool, aligning all buffers next to each other in memory to create a maximally sized free block. Note that existing handles will remain valid. */
void MemoryPool::defrag() {
    DENTER("Compute::MemoryPool::defrag");

    // We loop through all internal blocks
    VkResult vk_result;
    VkMemoryRequirements mem_requirements;
    VkDeviceSize offset = 0;
    for (const std::pair<MemoryHandle, UsedBlock*>& p : this->vk_used_blocks) {
        // Get a reference to the block
        UsedBlock* block = this->vk_used_blocks.at(p.first);

        // Switch based on the type of block what to do next
        if (block->type == MemoryBlockType::buffer) {
            // It's a buffer
            BufferBlock* bblock = (BufferBlock*) block;

            // Destroy the old one
            vkDestroyBuffer(this->gpu, bblock->vk_buffer, nullptr);

            // Prepare a new struct for reallocation
            VkBufferCreateInfo buffer_info;
            populate_buffer_info(buffer_info, bblock->length, bblock->vk_usage_flags, bblock->vk_sharing_mode, bblock->vk_create_flags);

            // Create a new vk_buffer
            if ((vk_result = vkCreateBuffer(this->gpu, &buffer_info, nullptr, &bblock->vk_buffer)) != VK_SUCCESS) {
                DLOG(fatal, "Could not re-create VkBuffer object: " + vk_error_map[vk_result]);
            }

            // Get the memory requirements for this new buffer
            vkGetBufferMemoryRequirements(this->gpu, bblock->vk_buffer, &mem_requirements);

            // Be sure that we still make it; due to aligning we might get a different size
            if (offset + mem_requirements.size > this->vk_memory_size) {
                DLOG(fatal, "Could not defrag buffer: memory requirements changed (need " + std::to_string(mem_requirements.size) + " bytes, but " + std::to_string(this->vk_memory_size) + " bytes free)");
            }

            // Bind new memory for, at the start of this index.
            if ((vk_result = vkBindBufferMemory(this->gpu, bblock->vk_buffer, this->vk_memory, offset)) != VK_SUCCESS) {
                DLOG(fatal, "Could not re-bind memory to buffer: " + vk_error_map[vk_result]);
            }
            
        } else if (block->type == MemoryBlockType::image) {
            // It's an image
            ImageBlock* iblock = (ImageBlock*) block;

            // Destroy the old one
            vkDestroyImage(this->gpu, iblock->vk_image, nullptr);

            // Prepare a new struct for reallocation
            VkImageCreateInfo image_info;
            populate_image_info(image_info, iblock->vk_extent, iblock->vk_format, iblock->vk_layout, iblock->vk_usage_flags, iblock->vk_sharing_mode, iblock->vk_create_flags);

            // Create a new VkImage
            if ((vk_result = vkCreateImage(this->gpu, &image_info, nullptr, &iblock->vk_image)) != VK_SUCCESS) {
                DLOG(fatal, "Could not re-create VkImage object: " + vk_error_map[vk_result]);
            }

            // Get the memory requirements for this new image
            vkGetImageMemoryRequirements(this->gpu, iblock->vk_image, &mem_requirements);

            // Be sure that we still make it; due to aligning we might get a different size
            if (offset + mem_requirements.size > this->vk_memory_size) {
                DLOG(fatal, "Could not defrag image: memory requirements changed (need " + std::to_string(mem_requirements.size) + " bytes, but " + std::to_string(this->vk_memory_size) + " bytes free)");
            }

            // Bind new memory for, at the start of this index.
            if ((vk_result = vkBindImageMemory(this->gpu, iblock->vk_image, this->vk_memory, offset)) != VK_SUCCESS) {
                DLOG(fatal, "Could not re-bind memory to image: " + vk_error_map[vk_result]);
            }

        }

        // Finally, update the offset & possible size in the block
        block->start = offset;
        block->req_length = mem_requirements.size;

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



/* Static function that helps users decide the best memory queue for buffers. */
uint32_t MemoryPool::select_memory_type(const GPU& gpu, VkBufferUsageFlags usage_flags, VkMemoryPropertyFlags memory_properties, VkSharingMode sharing_mode, VkBufferCreateFlags create_flags) {
    DENTER("Compute::MemoryPool::select_memory_type(buffer)");

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

/* Static function that helps users decide the best memory queue for images. */
uint32_t MemoryPool::select_memory_type(const GPU& gpu, VkFormat format, VkImageLayout layout, VkImageUsageFlags usage_flags, VkMemoryPropertyFlags memory_properties, VkSharingMode sharing_mode, VkImageCreateFlags create_flags) {
    DENTER("Compute::MemoryPool::select_memory_type(image)");

    // Get the available memory in the internal device
    VkPhysicalDeviceMemoryProperties gpu_properties;
    vkGetPhysicalDeviceMemoryProperties(gpu, &gpu_properties);

    // Next, temporarily create a buffer to discover its type
    VkImageCreateInfo image_info;
    populate_image_info(image_info, VkExtent3D({ 4, 4, 1 }), format, layout, usage_flags, sharing_mode, create_flags);

    // Create the buffer
    VkResult vk_result;
    VkImage dummy;
    if ((vk_result = vkCreateImage(gpu, &image_info, nullptr, &dummy)) != VK_SUCCESS) {
        DLOG(fatal, "Could not allocate temporary dummy image: " + vk_error_map[vk_result]);
    }

    // Ge the memory requirements of the buffer
    VkMemoryRequirements mem_requirements;
    vkGetImageMemoryRequirements(gpu, dummy, &mem_requirements);

    // We can already destroy the buffer
    vkDestroyImage(gpu, dummy, nullptr);

    // Try to find suitable memory (i.e., check if the device has enough memory bits(?) and if the required properties match)
    for (uint32_t i = 0; i < gpu_properties.memoryTypeCount; i++) {
        if (mem_requirements.memoryTypeBits & (1 << i) && (gpu_properties.memoryTypes[i].propertyFlags & memory_properties) == memory_properties) {
            DRETURN i;
        }
    }

    // Didn't find any
    DLOG(fatal, "No suitable memory on device for given image configuration.");
    DRETURN 0;
}
