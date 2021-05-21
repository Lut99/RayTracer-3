/* SUITE.hpp
 *   by Lut99
 *
 * Created:
 *   15/05/2021, 13:36:07
 * Last edited:
 *   21/05/2021, 14:28:47
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Contains a very simple struct that can be used to pass necessary data
 *   around for computing with Vulkan.
**/

#ifndef COMPUTE_SUITE_HPP
#define COMPUTE_SUITE_HPP

#include "GPU.hpp"
#include "MemoryPool.hpp"
#include "DescriptorPool.hpp"
#include "CommandPool.hpp"

namespace RayTracer::Compute {
    /* The Compute::Suite struct, which can be used to easily pass the structures around for computing on a GPU. */
    struct Suite {
        /* Constant reference to the GPU where the computation will be done on. All other fields are on this GPU too. */
        const GPU& gpu;

        /* Memory pool for device-local, faster memory. */        
        MemoryPool& device_memory_pool;
        /* Memory pool for host-visible but slower memory, used for staging. */
        MemoryPool& stage_memory_pool;
        
        /* Pool to allocate descriptorsets on. Is required to support the most demanding operation that it will be used for. */
        DescriptorPool& descriptor_pool;
        
        /* Command pool which can be used to allocate one-time-record commandbuffers for actually running the compute shader. */
        CommandPool& compute_command_pool;
        /* Command buffer which can be used to perform staging memory operations with. */
        CommandBuffer staging_cb;
    };
}

#endif
