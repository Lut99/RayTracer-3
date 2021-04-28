/* MAIN.cpp
 *   by Lut99
 *
 * Created:
 *   08/04/2021, 13:20:40
 * Last edited:
 *   28/04/2021, 17:29:04
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
#include "compute/Pipeline.hpp"

#include "frame/Frame.hpp"

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
        // Define the frame size
        const constexpr uint32_t width = 800, height = 600;
        
        // Start by creating the GPU object
        GPU gpu(instance_extensions, device_extensions, required_layers);

        // Search for the required memory types
        uint32_t transfer_mem_type = MemoryPool::select_memory_type(gpu, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        uint32_t device_mem_type = MemoryPool::select_memory_type(gpu, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        // Declare the command pools
        CommandPool compute_cpool(gpu, gpu.queue_info().compute());
        CommandPool mem_cpool(gpu, gpu.queue_info().memory(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

        // Declare the memory pools
        MemoryPool transfer_mpool(gpu, transfer_mem_type, 1024 * 1024 * 1024, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        MemoryPool dev_mpool(gpu, device_mem_type, 1024 * 1024 * 1024, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        // Declare the descriptor pool
        DescriptorPool dpool(gpu, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2, 2);

        // Initialize the compute command buffer
        CommandBufferHandle cb_compute_h = compute_cpool.allocate();
        CommandBuffer cb_compute = compute_cpool[cb_compute_h];

        // Initialize the Frame we'll render to
        Frame frame(gpu, dev_mpool, mem_cpool, width, height);

        // Define the descriptor set layout
        DescriptorSetLayout layout(gpu);
        frame.add_binding(layout);
        layout.finalize();

        // Define descriptors for the uniform buffers used in the pipeline & bind them to the buffers
        DescriptorSet descriptors = dpool.allocate(layout);
        frame.set_descriptor(descriptors, 0);

        // Finally, load the shader & create the Pipeline
        Shader shader(gpu, "bin/shaders/simple_blue_background.spv");
        Pipeline compute_pipeline(
            gpu,
            shader,
            Tools::Array<DescriptorSetLayout>({ layout }),
            std::unordered_map<uint32_t, std::tuple<uint32_t, void*>>({ { 0, std::make_tuple(sizeof(uint32_t), (void*) &width) }, { 1, std::make_tuple(sizeof(uint32_t), (void*) &height) } })
        );



        // We're done initializing, so hit 'em with a newspace
        DLOG(auxillary, "");



        // With that prepared, it's time to record the command buffer
        DLOG(info, "Recording command buffer...");
        cb_compute.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        compute_pipeline.bind(cb_compute);
        descriptors.bind(cb_compute, compute_pipeline.layout());
        vkCmdDispatch(cb_compute, (width / 32) + 1, (height / 32) + 1, 1);
        DLOG(info, "Executing compute shader...");
        cb_compute.end(gpu.compute_queue());



        // With the queue idle for sure, copy the result buffer back to the staging buffer
        DLOG(info, "Retrieving frame...");
        frame.sync(transfer_mpool);
        frame.to_png("result.png");



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
