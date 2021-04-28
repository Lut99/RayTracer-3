/* MAIN.cpp
 *   by Lut99
 *
 * Created:
 *   08/04/2021, 13:20:40
 * Last edited:
 *   28/04/2021, 21:41:38
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

#include "camera/Camera.hpp"
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

        // Declare the descriptor pools
        DescriptorPool dpool(gpu, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2, 2);

        // Initialize the camera class to the set width & height
        Camera camera(gpu, dev_mpool, mem_cpool);
        camera.update(width, height, 2.0, 2.0, ((float) width / (float) height) * 2.0);

        // Define the descriptor set layout
        DescriptorSetLayout layout(gpu);
        camera.set_layout(layout);
        layout.finalize();

        // Define descriptors
        DescriptorSet descriptors = dpool.allocate(layout);
        camera.set_bindings(descriptors);

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
        
        // Render single frame with the chosen camera
        camera.render(compute_cpool, compute_pipeline, gpu.compute_queue(), descriptors);
        const Frame& result = camera.get_frame(transfer_mpool);



        // With the queue idle for sure, copy the result buffer back to the staging buffer
        DLOG(info, "Saving frame...");
        result.to_png("result.png");



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
