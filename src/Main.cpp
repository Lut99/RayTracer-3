/* MAIN.cpp
 *   by Lut99
 *
 * Created:
 *   08/04/2021, 13:20:40
 * Last edited:
 *   17/04/2021, 17:00:08
 * Auto updated?
 *   Yes
 *
 * Description:
 *   The entry point to the RayTracer. Is mostly parsing arguments and
 *   initialising libraries.
**/

#include <CppDebugger.hpp>

#include "compute/GPU.hpp"

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
        DLOG(auxillary, "");

        

    } catch (std::runtime_error& e) {
        // Simply quit
        DRETURN -1;
    }

    // We're done!
    DLOG(auxillary, "");
    DLOG(auxillary, "Done.");
    DLOG(auxillary, "");
    DRETURN 0;
}
