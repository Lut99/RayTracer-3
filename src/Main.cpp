/* MAIN.cpp
 *   by Lut99
 *
 * Created:
 *   08/04/2021, 13:20:40
 * Last edited:
 *   27/04/2021, 16:50:49
 * Auto updated?
 *   Yes
 *
 * Description:
 *   The entry point to the RayTracer. Is mostly parsing arguments and
 *   initialising libraries.
**/

#include <CppDebugger.hpp>

#include "compute/GPU.hpp"
#include "compute/MemoryPool.hpp"
#include "compute/DescriptorSetLayout.hpp"
#include "compute/DescriptorPool.hpp"
#include "compute/CommandPool.hpp"

using namespace std;
using namespace RayTracer;
using namespace RayTracer::Compute;
using namespace CppDebugger::SeverityValues;


/***** CONSTANTS *****/
/* The Vulkan instance extensions we want to be enabled. */
static const Tools::Array<const char*> instance_extensions({
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME
});
/* The Vulkan device extensions we want to be enabled. */
static const Tools::Array<const char*> device_extensions({
    // Nothing lmao
});
/* The Vulkan validation layers we want to be enabled. */
static const Tools::Array<const char*> required_layers({
    "VK_LAYER_KHRONOS_validation" 
});



/***** ENTRY POINT *****/
int main() {
    DSTART("main"); DENTER("main");

    DLOG(auxillary, "");
    DLOG(auxillary, "<<< RAYTRACER v3 >>>");
    DLOG(auxillary, "");

    try {
        // Start by creating the GPU object
        GPU gpu(instance_extensions, device_extensions, required_layers);

        // Search for the required memory types
        uint32_t transfer_mem_type = MemoryPool::select_memory_type(gpu, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        uint32_t device_mem_type = MemoryPool::select_memory_type(gpu, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        // Declare the command pools
        CommandPool compute_cpool(gpu, gpu.queue_info().compute());
        CommandPool mem_cpool(gpu, gpu.queue_info().memory());

        // Declare the memory pools
        MemoryPool transfer_mpool(gpu, transfer_mem_type, 1024 * sizeof(uint16_t), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        MemoryPool dev_mpool(gpu, device_mem_type, 2 * 1024 * sizeof(uint16_t), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        // Declare the descriptor pool
        DescriptorPool dpool(gpu, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2, 2);



        // Initialize the memory command buffer
        CommandBufferHandle cb_copy_h = mem_cpool.allocate();
        CommandBuffer cb_copy = mem_cpool[cb_copy_h];

        // Initialize the buffers for this memory
        BufferHandle copyfrom_h = dev_mpool.allocate(1024 * sizeof(uint16_t), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        BufferHandle copyto_h = dev_mpool.allocate(1024 * sizeof(uint16_t), VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        Buffer copyfrom = dev_mpool[copyfrom_h], copyto = dev_mpool[copyto_h];

        // Initialize the staging buffer
        BufferHandle staging_h = transfer_mpool.allocate(1024 * sizeof(uint16_t), VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
        Buffer staging = transfer_mpool[staging_h];

        // Define the descriptor set layouts for our test copy shader
        DescriptorSetLayout src_layout(gpu, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT);
        DescriptorSetLayout dst_layout(gpu, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT);

        // Define descriptors for the uniform buffers used in the pipeline
        DescriptorSet src_descriptor = dpool.nallocate(1, src_layout)[0];
        DescriptorSet dst_descriptor = dpool.nallocate(1, dst_layout)[0];

        // Set the descriptors with the correct buffer values
        src_descriptor.set(gpu, 0, copyfrom);
        dst_descriptor.set(gpu, 1, copyto);



        // We're done initializing, so hit 'em with a newspace
        DLOG(auxillary, "");

        DLOG(info, "Mapping staging buffer...");
        uint16_t* data;
        staging.map(gpu, (void**) &data);

        DLOG(info, "Populating staging buffer...");
        for (uint16_t i = 0; i < 1024; i++) {
            data[i] = i;
        }

        DLOG(info, "Flushing staging buffer...");
        staging.flush(gpu);
        staging.unmap(gpu);

        DLOG(info, "Populating source buffer...");
        staging.copy(gpu, cb_copy, copyfrom);



        // With that prepared, it's time to call our compute pipeline



        // Dope, done
        DLOG(auxillary, "");
    } catch (CppDebugger::Fatal& e) {
        // Simply quit
        DRETURN -1;
    }

    // We're done!
    DLOG(auxillary, "");
    DLOG(auxillary, "Done.");
    DLOG(auxillary, "");
    DRETURN 0;
}
