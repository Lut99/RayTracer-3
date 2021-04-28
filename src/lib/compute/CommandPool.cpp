/* COMMAND POOL.cpp
 *   by Lut99
 *
 * Created:
 *   27/04/2021, 13:03:50
 * Last edited:
 *   28/04/2021, 15:09:53
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Contains the CommandPool class, which handles allocating and
 *   destroying command buffers for a single device queue.
**/

#include <CppDebugger.hpp>

#include "ErrorCodes.hpp"

#include "CommandPool.hpp"

using namespace std;
using namespace RayTracer;
using namespace RayTracer::Compute;
using namespace CppDebugger::SeverityValues;


/***** POPULATE FUNCTIONS *****/
/* Functions that populates a given VkCommandPoolCreateInfo struct with the given values. */
static void populate_command_pool_info(VkCommandPoolCreateInfo& command_pool_info, uint32_t queue_index, VkCommandPoolCreateFlags create_flags) {
    DENTER("populate_command_pool_info");

    // Set to default
    command_pool_info = {};
    command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

    // Set the queue we're the pool for
    command_pool_info.queueFamilyIndex = queue_index;

    // Set the create flags
    command_pool_info.flags = create_flags;

    // Done!
    DRETURN;
}

/* Function that populates a given VkCommandBufferAllocateInfo struct with the given values. */
static void populate_allocate_info(VkCommandBufferAllocateInfo& allocate_info, VkCommandPool vk_command_pool, uint32_t n_buffers, VkCommandBufferLevel buffer_level) {
    DENTER("populate_allocate_info");

    // Set to default
    allocate_info = {};
    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;

    // Set the buffer level for this buffer
    allocate_info.level = buffer_level;

    // Tell it which command pool to allocate with
    allocate_info.commandPool = vk_command_pool;

    // Tell it how many buffers to allocate in one go
    allocate_info.commandBufferCount = n_buffers;

    // Done!
    DRETURN;
}

/* Function that populates a given VkCommandBufferBeginInfo struct with the given values. */
static void populate_begin_info(VkCommandBufferBeginInfo& begin_info, VkCommandBufferUsageFlags usage_flags) {
    DENTER("populate_begin_info");

    // Set the deafult
    begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    // Set the flags
    begin_info.flags = usage_flags;

    // Done
    DRETURN;
}

/* Function that populates a given VkSubmitINfo struct with the given values. */
static void populate_submit_info(VkSubmitInfo& submit_info, const VkCommandBuffer& vk_command_buffer) {
    DENTER("populate_submit_info");

    // Set to default
    submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    // Set the command buffer to submit
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &vk_command_buffer;

    // Make sure that there are no semaphores used
    submit_info.signalSemaphoreCount = 0;
    submit_info.waitSemaphoreCount = 0;

    // Done
    DRETURN;
}





/***** COMMANDBUFFER CLASS *****/
/* Private constructor for the CommandBuffer, which only takes the VkCommandBuffer object to wrap. */
CommandBuffer::CommandBuffer(VkCommandBuffer vk_command_buffer) :
    vk_command_buffer(vk_command_buffer)
{}

/* Begins recording the command buffer. Overwrites whatever is already recorded here, for some reason. Takes optional usage flags for this recording. */
void CommandBuffer::begin(VkCommandBufferUsageFlags usage_flags) {
    DENTER("Compute::CommandBuffer::begin");

    // Populate the begin info struct
    VkCommandBufferBeginInfo begin_info;
    populate_begin_info(begin_info, usage_flags);

    // Initialize the recording
    VkResult vk_result;
    if ((vk_result = vkBeginCommandBuffer(this->vk_command_buffer, &begin_info)) != VK_SUCCESS) {
        DLOG(fatal, "Could not begin recording command buffer: " + vk_error_map[vk_result]);
    }

    // Done
    DRETURN;
}

/* Ends recording the command buffer, but does not yet submit to any queue unless one is given. If so, then you can optionally specify to wait or not to wait for the queue to become idle. */
void CommandBuffer::end(VkQueue vk_queue, bool wait_queue_idle) {
    DENTER("Compute::CommandBuffer::end");

    // Whatever the parameters, always call the stop recording
    VkResult vk_result;
    if ((vk_result = vkEndCommandBuffer(this->vk_command_buffer)) != VK_SUCCESS) {
        DLOG(fatal, "Could not finish recording command buffer: " + vk_error_map[vk_result]);
    }

    // If a queue is given, then submit the buffer to that queue
    if (vk_queue != nullptr) {
        // Populate the submit info struct
        VkSubmitInfo submit_info;
        populate_submit_info(submit_info, this->vk_command_buffer);

        // Submit it to the queue
        if ((vk_result = vkQueueSubmit(vk_queue, 1, &submit_info, VK_NULL_HANDLE)) != VK_SUCCESS) {
            DLOG(fatal, "Could not submit command buffer to the given queue: " + vk_error_map[vk_result]);
        }

        // If we're told to wait, then do so
        if (wait_queue_idle) {
            if ((vk_result = vkQueueWaitIdle(vk_queue)) != VK_SUCCESS) {
                DLOG(fatal, "Could not wait for queue to become idle: " + vk_error_map[vk_result]);
            }
        }
    }

    // Done
    DRETURN;
}





/***** COMMANDPOOL CLASS *****/
/* Constructor for the CommandPool class, which takes the GPU to allocate for, the queue index for which this pool allocates buffers and optionally create flags for the pool. */
CommandPool::CommandPool(const GPU& gpu, uint32_t queue_index, VkCommandPoolCreateFlags create_flags) :
    gpu(gpu),
    vk_queue_index(queue_index)
{
    DENTER("Compute::CommandPool::CommandPool");
    DLOG(info, "Initializing CommandPool for queue " + std::to_string(this->vk_queue_index) + "...");

    // Start by populating the create info
    VkCommandPoolCreateInfo command_pool_info;
    populate_command_pool_info(command_pool_info, this->vk_queue_index, create_flags);

    // Call the create
    VkResult vk_result;
    if ((vk_result = vkCreateCommandPool(this->gpu, &command_pool_info, nullptr, &this->vk_command_pool)) != VK_SUCCESS) {
        DLOG(fatal, "Could not create CommandPool: " + vk_error_map[vk_result]);
    }

    // Done
    DLEAVE;
}

/* Move constructor for the CommandPool class. */
CommandPool::CommandPool(CommandPool&& other) :
    gpu(other.gpu),
    vk_command_pool(other.vk_command_pool),
    vk_queue_index(other.vk_queue_index),
    vk_command_buffers(std::move(other.vk_command_buffers))
{
    other.vk_command_pool = nullptr;
    other.vk_command_buffers.clear();
}

/* Destructor for the CommandPool class. */
CommandPool::~CommandPool() {
    DENTER("Compute::CommandPool::~CommandPool");
    DLOG(info, "Cleaning CommandPool for queue " + std::to_string(this->vk_queue_index) + "...");
    DINDENT;

    if (this->vk_command_buffers.size() > 0) {
        DLOG(info, "Cleaning command buffers...");
        for (const std::pair<CommandBufferHandle, VkCommandBuffer>& p : this->vk_command_buffers) {
            vkFreeCommandBuffers(this->gpu, this->vk_command_pool, 1, &p.second);
        }
    }

    if (this->vk_command_pool != nullptr) {
        DLOG(info, "Deallocating the pool...");
        vkDestroyCommandPool(this->gpu, this->vk_command_pool, nullptr);
    }

    DDEDENT;
    DLEAVE;
}



/* Returns a CommandBuffer from the given handle, which can be used as a CommandBuffer. Does perform checks on the handle validity. */
CommandBuffer CommandPool::at(CommandBufferHandle buffer) const {
    DENTER("Compute::CommandPool::at");

    // Check if the handle exists
    std::unordered_map<CommandBufferHandle, VkCommandBuffer>::const_iterator iter = this->vk_command_buffers.find(buffer);
    if (iter == this->vk_command_buffers.end()) {
        DLOG(fatal, "Given handle does not exist.");
    }

    // If it does, return it
    return CommandBuffer((*iter).second);
}



/* Allocates a single, new command buffer of the given level. Returns by handle. */
CommandBufferHandle CommandPool::allocate(VkCommandBufferLevel buffer_level) {
    DENTER("Compute::CommandPool::allocate");

    // Pick a suitable memory location for this buffer; either as a new buffer or a previously deallocated one
    CommandBufferHandle result = 0;
    bool unique = false;
    while (!unique) {
        unique = true;
        for (const std::pair<CommandBufferHandle, VkCommandBuffer>& p : this->vk_command_buffers) {
            if (result == p.first) {
                ++result;
                unique = false;
                break;
            }
        }
    }

    // Then, prepare the create info
    VkCommandBufferAllocateInfo allocate_info;
    populate_allocate_info(allocate_info, this->vk_command_pool, 1, buffer_level);

    // Call the create
    VkCommandBuffer buffer;
    VkResult vk_result;
    if ((vk_result = vkAllocateCommandBuffers(this->gpu, &allocate_info, &buffer)) != VK_SUCCESS) {
        DLOG(fatal, "Could not allocate command buffer: " + vk_error_map[vk_result]);
    }

    // Inject the result in the map
    this->vk_command_buffers.insert(std::make_pair(result, buffer));

    // Done, return the handle
    DRETURN result;
}

/* Allocates N new command buffers of the given level. Returns by handles. */
Tools::Array<CommandBufferHandle> CommandPool::nallocate(uint32_t n_buffers, VkCommandBufferLevel buffer_level) {
    DENTER("Compute::CommandPool::nallocate");

    // Pick n_buffers suitable memory locations for this buffer; either as a new buffer or a previously deallocated one
    Tools::Array<CommandBufferHandle> result;
    for (size_t i = 0; i < result.size(); i++) {
        result[i] = 0;
        bool unique = false;
        while (!unique) {
            unique = true;
            for (const std::pair<CommandBufferHandle, VkCommandBuffer>& p : this->vk_command_buffers) {
                if (result[i] == p.first) {
                    ++result[i];
                    unique = false;
                    break;
                }
            }
        }
    }

    // Prepare some temporary local space for the buffers
    Tools::Array<VkCommandBuffer> buffers(n_buffers);

    // Then, prepare the create info
    VkCommandBufferAllocateInfo allocate_info;
    populate_allocate_info(allocate_info, this->vk_command_pool, n_buffers, buffer_level);

    // Call the create
    VkResult vk_result;
    if ((vk_result = vkAllocateCommandBuffers(this->gpu, &allocate_info, buffers.wdata(n_buffers))) != VK_SUCCESS) {
        DLOG(fatal, "Could not allocate command buffers: " + vk_error_map[vk_result]);
    }

    // Inject each of the buffers into the map
    for (size_t i = 0; i < buffers.size(); i++) {
        this->vk_command_buffers.insert(std::make_pair(result[i], buffers[i]));
    }

    // Done, return the list of handles
    DRETURN result;
}

/* Deallocates the CommandBuffer behind the given handle. Note that all buffers are deallocated automatically when the CommandPool is destructed, but this could save you memory. */
void CommandPool::deallocate(CommandBufferHandle buffer) {
    DENTER("Compute::CommandPool::deallocate");

    // Check if the handle exists
    std::unordered_map<CommandBufferHandle, VkCommandBuffer>::iterator iter = this->vk_command_buffers.find(buffer);
    if (iter == this->vk_command_buffers.end()) {
        DLOG(fatal, "Given handle does not exist.");
    }

    // If it does, then first free the buffer
    vkFreeCommandBuffers(this->gpu, this->vk_command_pool, 1, &((*iter).second));

    // Remove it from the list
    this->vk_command_buffers.erase(iter);

    // Done
    DRETURN;
}



/* Move assignment operator for the CommandPool class. */
CommandPool& CommandPool::operator=(CommandPool&& other) {
    if (this != &other) { swap(*this, other); }
    return *this;
}

/* Swap operator for the CommandPool class. */
void Compute::swap(CommandPool& cp1, CommandPool& cp2) {
    DENTER("Compute::swap(CommandPool)");

    using std::swap;

    #ifndef NDEBUG
    // If the GPU is not the same, then initialize to all nullptrs and everything
    if (cp1.gpu != cp2.gpu) {
        DLOG(fatal, "Cannot swap command pools with different GPUs");
    }
    #endif

    swap(cp1.vk_command_pool, cp2.vk_command_pool);
    swap(cp1.vk_queue_index, cp2.vk_queue_index);
    swap(cp1.vk_command_buffers, cp2.vk_command_buffers);

    DRETURN;
}