/* VULKAN ONLINE RENDERER.cpp
 *   by Lut99
 *
 * Created:
 *   09/05/2021, 18:30:34
 * Last edited:
 *   25/05/2021, 18:14:13
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Derived class of the VulkanRenderer class, which renders to a
 *   swapchain in real-time instead of to images. Therefore, the call to
 *   this render is blocking, returning an empty frame once the user closes
 *   the window.
**/

#include <chrono>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <CppDebugger.hpp>

#include "compute/Pipeline.hpp"
#include "compute/Swapchain.hpp"
#include "compute/ErrorCodes.hpp"
#include "tools/Common.hpp"

#include "VulkanOnlineRenderer.hpp"

using namespace std;
using namespace RayTracer;
using namespace RayTracer::Compute;
using namespace CppDebugger::SeverityValues;


/***** POPULATE FUNCTIONS *****/
/* Populates a given VkBufferImageCopy struct. */
static void populate_buffer_image_copy(VkBufferImageCopy& buffer_image_copy, uint32_t width, uint32_t height) {
    DENTER("populate_buffer_image_copy");

    // Prepare the subresource layer struct
    VkImageSubresourceLayers image_subresource_layers{};
    image_subresource_layers.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_subresource_layers.baseArrayLayer = 0;
    image_subresource_layers.mipLevel = 0;
    image_subresource_layers.layerCount = 1;

    // Set the properties of the source buffer (we copy it entirely)
    buffer_image_copy.bufferOffset = 0;
    buffer_image_copy.bufferRowLength = 0;
    buffer_image_copy.bufferImageHeight = 0;

    // Set the properties of the target image (we write to it entirely)
    buffer_image_copy.imageOffset = VkOffset3D({ 0, 0, 0 });
    buffer_image_copy.imageExtent = VkExtent3D({ width, height, 1 });
    buffer_image_copy.imageSubresource = image_subresource_layers;

    // We're done here
    DRETURN;
}

// /* Populate a given VkImageViewCreateInfo struct. */
// static void populate_view_info(VkImageViewCreateInfo& view_info, const VkImage& vk_image, const VkFormat& vk_format) {
//     DENTER("populate_view_info");

//     // Prepare the struct that defines which color channels map to what
//     VkComponentMapping component_mapping{};
//     component_mapping.r = VK_COMPONENT_SWIZZLE_IDENTITY;
//     component_mapping.g = VK_COMPONENT_SWIZZLE_IDENTITY;
//     component_mapping.b = VK_COMPONENT_SWIZZLE_IDENTITY;
//     component_mapping.a = VK_COMPONENT_SWIZZLE_IDENTITY;

//     // Prepare the struct that defines how the subresource looks like
//     VkImageSubresourceRange subresource_range{};
//     subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//     subresource_range.baseArrayLayer = 0;
//     subresource_range.layerCount = 1;
//     subresource_range.baseMipLevel = 0;
//     subresource_range.levelCount = 1;

//     // Set the default stuff
//     view_info = {};
//     view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

//     // Set the image and associated parameters
//     view_info.image = vk_image;
//     view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
//     view_info.format = vk_format;
//     view_info.subresourceRange = subresource_range;
//     view_info.components = component_mapping;

//     // Done
//     DRETURN;
// }

/* Populates a given VkImageMemoryBarrier struct. */
static void populate_image_barrier(VkImageMemoryBarrier& image_barrier, VkImage vk_image, VkImageLayout old_layout, VkImageLayout new_layout, VkAccessFlags src_access, VkAccessFlags dst_access, uint32_t src_queue, uint32_t dst_queue = UINT32_MAX) {
    DENTER("populate_image_barrier");

    // If the destination queue is the default value, then set it to the source queue
    if (dst_queue == UINT32_MAX) {
        dst_queue = src_queue;
    }

    // Set to std
    image_barrier = {};
    image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

    // Set the image to wait for
    image_barrier.image = vk_image;

    // Set from which to which layout we move
    image_barrier.oldLayout = old_layout;
    image_barrier.newLayout = new_layout;

    // Set which access masks
    image_barrier.srcAccessMask = src_access;
    image_barrier.dstAccessMask = dst_access;

    // Define to which queue we transfer
    image_barrier.srcQueueFamilyIndex = src_queue;
    image_barrier.dstQueueFamilyIndex = dst_queue;

    // Set the subresource information for this image (std)
    image_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_barrier.subresourceRange.baseArrayLayer = 0;
    image_barrier.subresourceRange.baseMipLevel = 0;
    image_barrier.subresourceRange.levelCount = 1;
    image_barrier.subresourceRange.layerCount = 1;

    DRETURN;
}

/* Populates a given VkBufferMemoryBarrier struct. */
static void populate_buffer_barrier(VkBufferMemoryBarrier& buffer_barrier, const Buffer& buffer, VkAccessFlags src_access, VkAccessFlags dst_access, uint32_t src_queue, uint32_t dst_queue = UINT32_MAX) {
    DENTER("populate_buffer_barrier");

    // If the destination queue is the default value, then set it to the source queue
    if (dst_queue == UINT32_MAX) {
        dst_queue = src_queue;
    }

    // Set to std
    buffer_barrier = {};
    buffer_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;

    // Set the buffer to wait for
    buffer_barrier.buffer = buffer;
    buffer_barrier.offset = 0;
    buffer_barrier.size = VK_WHOLE_SIZE;

    // Set which access masks
    buffer_barrier.srcAccessMask = src_access;
    buffer_barrier.dstAccessMask = dst_access;

    // Define to which queue we transfer
    buffer_barrier.srcQueueFamilyIndex = src_queue;
    buffer_barrier.dstQueueFamilyIndex = dst_queue;

    DRETURN;
}

/* Populates a given VkPresentInfoKHR struct. */
static void populate_present_info(VkPresentInfoKHR& present_info, const Swapchain& swapchain, const uint32_t& image_index, const VkSemaphore& vk_semaphore) {
    DENTER("populate_present_info");

    // First, set the default stuff
    present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    // Set the semaphore to wait for before we can present
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &vk_semaphore;

    // Set the image to present
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &swapchain.swapchain();
    present_info.pImageIndices = &image_index;

    // Done
    DRETURN;
}

/* Populates a given VkSemaphoreCreateInfo struct. */
static void populate_semaphore_info(VkSemaphoreCreateInfo& semaphore_info) {
    DENTER("populate_semaphore_info");

    // First, set the default stuff
    semaphore_info = {};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    // And that's it already!
    DRETURN;
}

/* Populates a given VkFenceCreateInfo struct. */
static void populate_fence_info(VkFenceCreateInfo& fence_info) {
    DENTER("populate_fence_info");

    // First, set the default stuff
    fence_info = {};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    // Mark that we create fences as already signalled, to avoid the first wait to be endless
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    // And that's it already!
    DRETURN;
}





/***** RECORD FUNCTIONS *****/
/* Records the compute command buffer. */
void record_compute_cb(CommandBuffer& compute_cb, const GPU& gpu, const Pipeline& pipeline, const DescriptorSet& descriptor_set, const Buffer& frame, const VkExtent2D& swapchain_extent) {
    DENTER("record_compute_cb");

    // Before we begin, prepare a buffer barrier for acquiring the buffer and stuff
    VkBufferMemoryBarrier buffer_barrier;
    
    // With that set, begin recording the command buffer
    compute_cb.begin();

    // First, acquire the buffer on the compute queue so that we can run it on the presentation queue next
    populate_buffer_barrier(
        buffer_barrier,
        frame,
        VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_SHADER_WRITE_BIT,
        gpu.queue_info().presentation(), gpu.queue_info().compute()
    );
    vkCmdPipelineBarrier(compute_cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_DEPENDENCY_DEVICE_GROUP_BIT, 0, nullptr, 1, &buffer_barrier, 0, nullptr);

    // First, we dispatch the compute shader with its resources
    pipeline.bind(compute_cb);
    descriptor_set.bind(compute_cb, pipeline.layout());
    vkCmdDispatch(compute_cb, (swapchain_extent.width / 32) + 1, (swapchain_extent.height / 32) + 1, 1);

    // // Add a memory barrier before we schedule the copy to make sure that the shader is done rendering
    // VkMemoryBarrier memory_barrier{};
    // memory_barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    // memory_barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
    // memory_barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    // vkCmdPipelineBarrier(compute_cb, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_DEPENDENCY_DEVICE_GROUP_BIT, 1, &memory_barrier, 0, nullptr, 0, nullptr);

    // Release the buffer on the compute queue, so that we can run it on the presentation queue next
    populate_buffer_barrier(
        buffer_barrier,
        frame,
        VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT,
        gpu.queue_info().compute(), gpu.queue_info().presentation()
    );
    vkCmdPipelineBarrier(compute_cb, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_DEVICE_GROUP_BIT, 0, nullptr, 1, &buffer_barrier, 0, nullptr);

    // We're done with this commandbuffer
    compute_cb.end();

    DRETURN;
}

/* Records the copy-the-buffer command buffer. */
void record_copy_cb(CommandBuffer& copy_cb, const GPU& gpu, const Buffer& frame, VkImage vk_swapchain_image, const VkExtent2D& swapchain_extent) {
    DENTER("record_copy_cb");

    // Before we begin, prepare structs required throughout the recording
    VkBufferMemoryBarrier buffer_barrier;
    VkImageMemoryBarrier image_barrier;
    VkBufferImageCopy buffer_image_copy;

    // We start recording
    copy_cb.begin();

    // We first grab the buffer of the compute queue, ready to be transferred from
    populate_buffer_barrier(
        buffer_barrier,
        frame,
        VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_TRANSFER_READ_BIT,
        gpu.queue_info().compute(), gpu.queue_info().presentation()
    );
    vkCmdPipelineBarrier(copy_cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_DEVICE_GROUP_BIT, 0, nullptr, 1, &buffer_barrier, 0, nullptr);

    // With the buffer ready, we change the image to the required layout
    populate_image_barrier(
        image_barrier,
        vk_swapchain_image,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
        gpu.queue_info().presentation()
    );
    vkCmdPipelineBarrier(copy_cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_DEVICE_GROUP_BIT, 0, nullptr, 0, nullptr, 1, &image_barrier);

    // Then, we schedule a copy to the swapchain image
    populate_buffer_image_copy(buffer_image_copy, swapchain_extent.width, swapchain_extent.height);
    vkCmdCopyBufferToImage(copy_cb, frame, vk_swapchain_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &buffer_image_copy);

    // Once that is done, revert the image back to present layout
    populate_image_barrier(
        image_barrier,
        vk_swapchain_image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT,
        gpu.queue_info().compute(), gpu.queue_info().presentation()
    );
    vkCmdPipelineBarrier(copy_cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_DEVICE_GROUP_BIT, 0, nullptr, 0, nullptr, 1, &image_barrier);

    // Also release the buffer back to the compute queue
    populate_buffer_barrier(
        buffer_barrier,
        frame,
        VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_TRANSFER_READ_BIT,
        gpu.queue_info().presentation(), gpu.queue_info().compute()
    );
    vkCmdPipelineBarrier(copy_cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_DEVICE_GROUP_BIT, 0, nullptr, 1, &buffer_barrier, 0, nullptr);

    // Done recording, but leave actually submitting to the main render function
    copy_cb.end();

    DRETURN;
}





/***** VULKANONLINERENDERER CLASS *****/
/* Constructor for the VulkanOnlineRenderer class, which takes nothing to be compatible. Note that it does not rely on the parent constructor, since we want to start the vulkan instance & GPU differently. */
VulkanOnlineRenderer::VulkanOnlineRenderer() :
    VulkanRenderer(false),
    present_command_pool(nullptr)
{
    DENTER("VulkanOnlineRenderer::VulkanOnlineRenderer");
    DLOG(info, "Initializing Vulkan-based online renderer...");
    DINDENT;
    
    // First, prepare the GLFW library. Note that this is global, and is thus tricky to use in multiple ways I guess
    DLOG(info, "Initializing GLFW...");
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);



    // Next, setup the Vulkan stuff
    // We first collect a list of GLFW extensions
    uint32_t n_extensions = 0;
    const char** raw_extensions = glfwGetRequiredInstanceExtensions(&n_extensions);

    // Use them to populate the instance
    this->instance = new Instance(instance_extensions + Tools::Array<const char*>(raw_extensions, n_extensions));

    // Next, create the GPU in swapchain mode
    this->gpu = new GPU(*this->instance, device_extensions + Tools::Array<const char*>({ VK_KHR_SWAPCHAIN_EXTENSION_NAME }));

    // Before we continue, select suitable memory types for each pool.
    uint32_t device_memory_type = MemoryPool::select_memory_type(*this->gpu, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    uint32_t stage_memory_type = MemoryPool::select_memory_type(*this->gpu, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    // Next, initialize all  pools
    this->device_memory_pool = new MemoryPool(*this->gpu, device_memory_type, VulkanOnlineRenderer::device_memory_size, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    this->stage_memory_pool = new MemoryPool(*this->gpu, stage_memory_type, VulkanOnlineRenderer::stage_memory_size, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    this->descriptor_pool = new DescriptorPool(
        *this->gpu,
        Tools::Array<std::tuple<VkDescriptorType, uint32_t>>({
            std::make_tuple(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 * VulkanOnlineRenderer::max_frames_in_flight),
            std::make_tuple(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3 * VulkanOnlineRenderer::max_frames_in_flight)
            // std::make_tuple(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2),
            // std::make_tuple(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1)
        }),
        VulkanOnlineRenderer::max_descriptor_sets
    );

    this->compute_command_pool = new CommandPool(*this->gpu, this->gpu->queue_info().compute());
    this->memory_command_pool = new CommandPool(*this->gpu, this->gpu->queue_info().memory(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    // Initialize the descriptor set layout for the raytrace call
    this->raytrace_dsl = new DescriptorSetLayout(*this->gpu);
    // this->raytrace_dsl->add_binding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT);
    this->raytrace_dsl->add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT);
    this->raytrace_dsl->add_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT);
    this->raytrace_dsl->add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT);
    this->raytrace_dsl->add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT);
    this->raytrace_dsl->finalize();

    // Also allocate the command buffer(s)
    this->staging_cb_h = this->memory_command_pool->allocate_h();



    DDEDENT;
    DLEAVE;
}

/* Copy constructor for the VulkanOnlineRenderer class. */
VulkanOnlineRenderer::VulkanOnlineRenderer(const VulkanOnlineRenderer& other) :
    VulkanRenderer(other),
    present_command_pool(other.present_command_pool)
{
    DENTER("Compute::VulkanOnlineRenderer::VulkanOnlineRenderer(copy)");
    
    // Copy our special command pool too
    if (other.present_command_pool != nullptr) {
        this->present_command_pool = new CommandPool(*other.present_command_pool);
    }

    DLEAVE;
}

/* Move constructor for the VulkanOnlineRenderer class. */
VulkanOnlineRenderer::VulkanOnlineRenderer(VulkanOnlineRenderer&& other) :
    VulkanRenderer(std::move(other)),
    present_command_pool(other.present_command_pool)
{
    // Set the deallocatable stuff to nullptrs
    this->present_command_pool = nullptr;
}

/* Destructor for the VulkanOnlineRenderer class. */
VulkanOnlineRenderer::~VulkanOnlineRenderer() {
    DENTER("VulkanOnlineRenderer::~VulkanOnlineRenderer");
    DLOG(info, "Cleaning online renderer stuff...");
    DINDENT;
    
    if (this->present_command_pool != nullptr) {
        delete this->present_command_pool;
    }

    DLOG(info, "Terminating GLFW library...");
    glfwTerminate();

    DDEDENT;
    DLEAVE;
}



/* Renders the internal list of vertices to a window using the given camera position. Renders the entire simulation, including update steps, and returns a blackened frame since the output is unreachable and simultaneously not interesting. */
void VulkanOnlineRenderer::render(Camera& cam) const {
    DENTER("VulkanOnlineRenderer::render");
    uint32_t width = cam.w(), height = cam.h();

    /* Step 1: Initialize the window we'll be rendering to and get a surface. */
    DLOG(info, "Creating GLFW window...");
    GLFWwindow* glfw_window = glfwCreateWindow(width, height, "RayTracer-3", NULL, NULL);
    VkSurfaceKHR glfw_surface;
    VkResult vk_result;
    if ((vk_result = glfwCreateWindowSurface(*this->instance, glfw_window, nullptr, &glfw_surface)) != VK_SUCCESS) {
        DLOG(fatal, "Could not get GLFW window surface: " + vk_error_map[vk_result]);
    }

    // Also update the GPU info for its presentation queue support
    this->gpu->check_present(glfw_surface);

    // With this, allocate a compute queue for the newly found presentation queue
    CommandPool present_command_pool(*this->gpu, this->gpu->queue_info().presentation());



    /* Step 2: Prepare the initial swapchain. */
    Swapchain* swapchain = new Swapchain(*this->gpu, glfw_window, glfw_surface);



    /* Step 3: Allocate buffers. */
    DLOG(info, "Preparing camera buffers...");

    // First, allocate the camera buffer and a matching staging buffer
    size_t camera_size = sizeof(GCameraData);
    Buffer camera = this->device_memory_pool->allocate_buffer(camera_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    Buffer staging = this->stage_memory_pool->allocate_buffer(camera_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);



    /* Step 4: Prepare the frame output buffers. */
    DLOG(info, "Preparing output buffers...");

    // Allocate the images to output to
    size_t frame_size = swapchain->extent().width * swapchain->extent().height * sizeof(uint32_t);
    Tools::Array<Buffer> frames(VulkanOnlineRenderer::max_frames_in_flight);
    for (uint32_t i = 0; i < VulkanOnlineRenderer::max_frames_in_flight; i++) {
        // Allocate it
        frames.push_back(this->device_memory_pool->allocate_buffer(
            frame_size,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT
        ));
    }



    // /* Step 4: Prepare the image views for the frames. */
    // DLOG(info, "Preparing image views...");

    // // Next, create the image views themselves
    // Tools::Array<VkImageView> image_views;
    // image_views.resize(VulkanOnlineRenderer::max_frames_in_flight);
    // for (uint32_t i = 0; i < VulkanOnlineRenderer::max_frames_in_flight; i++) {
    //     // Prepare the create info
    //     VkImageViewCreateInfo view_info;
    //     populate_view_info(view_info, frames[i], frames[i].format());

    //     // Create the image view from this
    //     if ((vk_result = vkCreateImageView(*this->gpu, &view_info, nullptr, &image_views[i])) != VK_SUCCESS) {
    //         DLOG(fatal, "Could not create image view for output frame " + std::to_string(i) + ": " + vk_error_map[vk_result]);
    //     }
    // }

    // // Also define the subresource layer thingy bla bla
    // VkImageSubresourceLayers subresource_layers{};
    // subresource_layers.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    // subresource_layers.baseArrayLayer = 0;
    // subresource_layers.layerCount = 1;
    // subresource_layers.mipLevel = 1;



    /* Step 5: Prepare the pipeline. */
    // Create the pipeline
    VkExtent2D swapchain_extent = swapchain->extent();
    Pipeline pipeline(
        *this->gpu,
        Shader(*this->gpu, Tools::get_executable_path() + "/shaders/raytracer_v3.spv"),
        Tools::Array<DescriptorSetLayout>({ *this->raytrace_dsl }),
        std::unordered_map<uint32_t, std::tuple<uint32_t, void*>>({ { 0, std::make_tuple((uint32_t) sizeof(uint32_t), (void*) &swapchain_extent.width) }, { 1, std::make_tuple((uint32_t) sizeof(uint32_t), (void*) &swapchain_extent.height) } })
    );



    /* Step 6: Prepare the descriptor sets. */
    DLOG(info, "Preparing descriptor sets...");

    // Fetch the internal faces & vertex handles as buffers
    Buffer vk_entity_faces = this->device_memory_pool->deref_buffer(this->vk_entity_faces);
    Buffer vk_entity_vertices = this->device_memory_pool->deref_buffer(this->vk_entity_vertices);

    // Prepare the descriptor set, with all bindings
    Tools::Array<DescriptorSet> descriptor_sets(VulkanOnlineRenderer::max_frames_in_flight);
    for (size_t i = 0; i < VulkanOnlineRenderer::max_frames_in_flight; i++) {
        descriptor_sets.push_back(this->descriptor_pool->allocate(*this->raytrace_dsl));
        
        // We can also bind the data to the buffer already
        descriptor_sets[i].set(*this->gpu, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0, Tools::Array<Buffer>({ frames[i] }));
        descriptor_sets[i].set(*this->gpu, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, Tools::Array<Buffer>({ camera }));
        descriptor_sets[i].set(*this->gpu, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2, Tools::Array<Buffer>({ vk_entity_faces }));
        descriptor_sets[i].set(*this->gpu, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3, Tools::Array<Buffer>({ vk_entity_vertices }));
    }



    /* Step 6: Prepare the command buffers. */
    DLOG(info, "Preparing command buffers...");

    // Prepare the list of compute command buffers for each frame
    Tools::Array<CommandBuffer> compute_cbs(VulkanOnlineRenderer::max_frames_in_flight);
    Tools::Array<CommandBuffer> copy_cbs(VulkanOnlineRenderer::max_frames_in_flight);
    for (uint32_t i = 0; i < VulkanOnlineRenderer::max_frames_in_flight; i++) {
        compute_cbs.push_back(this->compute_command_pool->allocate());
        copy_cbs.push_back(present_command_pool.allocate());

        // Record the compute buffer already since that isn't swapchain dependent
        record_compute_cb(compute_cbs[i], *this->gpu, pipeline, descriptor_sets[i], frames[i], swapchain_extent);
    }

    

    /* Step 6: Synchronization structres. */
    DLOG(info, "Preparing synchronization structures...");
    // Keeps track of when an image has been acquired completely
    Tools::Array<VkSemaphore> image_ready_semaphores;
    // Keeps track of when a frame has been rendered
    Tools::Array<VkSemaphore> render_ready_semaphores;
    // Keeps track of when a frame is done being copied to the swapchain image
    Tools::Array<VkSemaphore> copy_ready_semaphores;
    // Keeps track of which frames we're using (NOT swapchain images)
    Tools::Array<VkFence> frame_in_flight_fences;
    // Keeps track of which swapchain images are used by a frame
    Tools::Array<VkFence> image_in_flight_fences;

    // Resize the arrays to the correct size
    image_ready_semaphores.resize(VulkanOnlineRenderer::max_frames_in_flight);
    render_ready_semaphores.resize(VulkanOnlineRenderer::max_frames_in_flight);
    copy_ready_semaphores.resize(VulkanOnlineRenderer::max_frames_in_flight);
    frame_in_flight_fences.resize(VulkanOnlineRenderer::max_frames_in_flight);
    image_in_flight_fences.resize(swapchain->size());

    // Prepare the only create infos we'll use for now
    VkSemaphoreCreateInfo semaphore_info;
    populate_semaphore_info(semaphore_info);
    VkFenceCreateInfo fence_info;
    populate_fence_info(fence_info);

    // Allocate two semaphores and a fence for each frame in flight
    for (uint32_t i = 0; i < VulkanOnlineRenderer::max_frames_in_flight; i++) {
        // First, allocate the image ready one
        if ((vk_result = vkCreateSemaphore(*this->gpu, &semaphore_info, nullptr, &image_ready_semaphores[i])) != VK_SUCCESS) {
            DLOG(fatal, "Could not create image ready semaphore " + std::to_string(i) + ": " + vk_error_map[vk_result]);
        }

        // Next, allocate the render ready one
        if ((vk_result = vkCreateSemaphore(*this->gpu, &semaphore_info, nullptr, &render_ready_semaphores[i])) != VK_SUCCESS) {
            DLOG(fatal, "Could not create render ready semaphore " + std::to_string(i) + ": " + vk_error_map[vk_result]);
        }

        // Allocate the copy ready one
        if ((vk_result = vkCreateSemaphore(*this->gpu, &semaphore_info, nullptr, &copy_ready_semaphores[i])) != VK_SUCCESS) {
            DLOG(fatal, "Could not create copy ready semaphore " + std::to_string(i) + ": " + vk_error_map[vk_result]);
        }

        // Finally, allocate a fence
        if ((vk_result = vkCreateFence(*this->gpu, &fence_info, nullptr, &frame_in_flight_fences[i])) != VK_SUCCESS) {
            DLOG(fatal, "Could not create fence " + std::to_string(i) + ": " + vk_error_map[vk_result]);
        }
    }

    // Set the images-in-flight fences to null handles
    for (uint32_t i = 0; i < swapchain->size(); i++) {
        image_in_flight_fences[i] = VK_NULL_HANDLE;
    }



    /* Step 6.5: Get some references. */
    DLOG(info, "Final preparations...");

    // Fetch the internal command buffer handle for staging
    CommandBuffer staging_cb = (*this->memory_command_pool)[this->staging_cb_h];
    


    /* Step 7: Game loop */
    DLOG(info, "Entering game loop...");
    DINDENT;
    uint32_t current_frame = 0;
    unsigned int fps_count = 0;
    std::chrono::time_point last_fps = std::chrono::system_clock::now();
    while (!glfwWindowShouldClose(glfw_window)) {
        // Let's handle the window events
        glfwPollEvents();



        // First, we wait until the current frame is available
        vkWaitForFences(*this->gpu, 1, &frame_in_flight_fences[current_frame], VK_TRUE, UINT64_MAX);

        

        // Try to get a new image to render to for this frame
        uint32_t swapchain_index;
        if ((vk_result = vkAcquireNextImageKHR(*this->gpu, *swapchain, UINT64_MAX, image_ready_semaphores[current_frame], VK_NULL_HANDLE, &swapchain_index)) != VK_SUCCESS) {
            DLOG(fatal, "Could not get next swapchain image: " + vk_error_map[vk_result]);
        }

        // We have the image, but if it's used by another frame already (i.e., if we have more frames in flight than swapchain images), we wait for the frame to be done
        if (image_in_flight_fences[swapchain_index] != VK_NULL_HANDLE) {
            vkWaitForFences(*this->gpu, 1, &image_in_flight_fences[swapchain_index], VK_TRUE, UINT64_MAX);
        }
        // Mark the image as being in use by this frame
        image_in_flight_fences[swapchain_index] = frame_in_flight_fences[current_frame];



        // Then, prepare a new frame by populatin the CPU camera buffer
        GCameraData camera_data({ cam.origin, cam.horizontal, cam.vertical, cam.lower_left_corner });
        camera.set(*this->gpu, staging, staging_cb, this->gpu->memory_queue(), (void*) &camera_data, (uint32_t) camera_size);

        

        // Record a new copy buffer for this frame
        present_command_pool.deallocate(copy_cbs[current_frame]);
        copy_cbs[current_frame] = present_command_pool.allocate();
        record_copy_cb(copy_cbs[current_frame], *this->gpu, frames[current_frame], (*swapchain)[swapchain_index], swapchain_extent);



        // Prepare submitting the compute command buffer, by adding relevant semaphore to its submit info
        VkSubmitInfo submit_info = compute_cbs[current_frame].get_submit_info();
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &image_ready_semaphores[current_frame];
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &render_ready_semaphores[current_frame];
        VkPipelineStageFlags semaphore_stage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        submit_info.pWaitDstStageMask = &semaphore_stage;

        // Submit it and don't wait
        if ((vk_result = vkQueueSubmit(gpu->compute_queue(), 1, &submit_info, VK_NULL_HANDLE)) != VK_SUCCESS) {
            DLOG(fatal, "Could not submit compute shader to queue: " + vk_error_map[vk_result]);
        }



        // Prepare submitting the copy command buffer, by adding relevant semaphore to its submit info
        submit_info = copy_cbs[current_frame].get_submit_info();
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &render_ready_semaphores[current_frame];
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &copy_ready_semaphores[current_frame];
        semaphore_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        submit_info.pWaitDstStageMask = &semaphore_stage;

        // Submit it and don't wait
        vkResetFences(*this->gpu, 1, &frame_in_flight_fences[current_frame]);
        if ((vk_result = vkQueueSubmit(gpu->present_queue(), 1, &submit_info, frame_in_flight_fences[current_frame])) != VK_SUCCESS) {
            DLOG(fatal, "Could not submit copy queue: " + vk_error_map[vk_result]);
        }



        // Now, present the resulting frame once it's done rendering
        VkPresentInfoKHR present_info;
        populate_present_info(present_info, *swapchain, swapchain_index, copy_ready_semaphores[current_frame]);
        
        // Hit it!
        if ((vk_result = vkQueuePresentKHR(gpu->present_queue(), &present_info)) != VK_SUCCESS) {
            DLOG(fatal, "Failed to present frame " + std::to_string(current_frame) + " to presentation queue: " + vk_error_map[vk_result]);
        }



        // Once done, advance to the next frame in flight
        current_frame = (current_frame + 1) % VulkanOnlineRenderer::max_frames_in_flight;



        // FPS counter
        ++fps_count;
        if (chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - last_fps).count() >= 1000) {
            // Reset the timer
            last_fps += chrono::milliseconds(1000);

            // Show the FPS
            glfwSetWindowTitle(glfw_window, ("RayTracer-3 (FPS: " + std::to_string(fps_count) + ")").c_str());
            fps_count = 0;
        }
    }
    DDEDENT;

    // Before we continue, wait until the device is idle tho
    vkDeviceWaitIdle(*this->gpu);



    /* Step 8: Cleanup. */
    DLOG(info, "Finalizing...");

    // Destroy the semaphores & fences
    for (uint32_t i = 0; i < VulkanOnlineRenderer::max_frames_in_flight; i++) {
        // Destroy the frame in flight fence
        vkDestroyFence(*this->gpu, frame_in_flight_fences[i], nullptr);

        // Destroy the copy ready semaphore
        vkDestroySemaphore(*this->gpu, copy_ready_semaphores[i], nullptr);

        // Destroy the render ready semaphore
        vkDestroySemaphore(*this->gpu, render_ready_semaphores[i], nullptr);

        // Destroy the image ready semaphore
        vkDestroySemaphore(*this->gpu, image_ready_semaphores[i], nullptr);
    }

    // // Destroy the image views
    // for (size_t i = 0; i < image_views.size(); i++) {
    //     vkDestroyImageView(*this->gpu, image_views[i], nullptr);
    // }

    // Destroy the buffers
    for (size_t i = 0; i < frames.size(); i++) {
        this->device_memory_pool->deallocate(frames[i]);
    }
    this->stage_memory_pool->deallocate(staging);
    this->device_memory_pool->deallocate(camera);

    // Destroy the swapchain
    delete swapchain;

    // Destroy the GLFW stuff
    vkDestroySurfaceKHR(*this->instance, glfw_surface, nullptr);
    glfwDestroyWindow(glfw_window);

    DRETURN;
}



/* Swap operator for the VulkanOnlineRenderer class. */
void RayTracer::swap(VulkanOnlineRenderer& r1, VulkanOnlineRenderer& r2) {
    using std::swap;

    // Swap as VulkanRenderer first
    swap((VulkanRenderer&) r1, (VulkanRenderer&) r2);

    // Swap our own fields
    swap(r1.present_command_pool, r2.present_command_pool);

    // Done
}





/***** FACTORY METHOD *****/
Renderer* RayTracer::initialize_renderer() {
    DENTER("initialize_renderer");

    // Initialize a new object
    VulkanOnlineRenderer* result = new VulkanOnlineRenderer();

    // Done, return
    DRETURN (Renderer*) result;
}
