/* MAIN.cpp
 *   by Lut99
 *
 * Created:
 *   08/04/2021, 13:20:40
 * Last edited:
 *   02/05/2021, 17:36:03
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


/***** ENTRY POINT *****/
int main() {
    DSTART("main"); DENTER("main");

    DLOG(auxillary, "");
    DLOG(auxillary, "<<< RAYTRACER v3 >>>");
    DLOG(auxillary, "");

    try {
        // Define the frame size
        const constexpr uint32_t width = 800, height = 600;
        
        
        


        // With the queue idle for sure, copy the result buffer back to the staging buffer
        DLOG(info, "Saving frame...");
        // const Frame& result = camera.get_frame(transfer_mpool);
        // result.to_png("result.png");
        // result.to_ppm("result.ppm");



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
