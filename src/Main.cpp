/* MAIN.cpp
 *   by Lut99
 *
 * Created:
 *   08/04/2021, 13:20:40
 * Last edited:
 *   03/05/2021, 13:37:12
 * Auto updated?
 *   Yes
 *
 * Description:
 *   The entry point to the RayTracer. Is mostly parsing arguments and
 *   initialising libraries.
**/

#include <CppDebugger.hpp>

#include "renderer/Renderer.hpp"

#include "entities/Triangle.hpp"

#include "camera/Camera.hpp"
#include "camera/Frame.hpp"

using namespace std;
using namespace RayTracer;
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

        // Initialize the renderer
        Renderer* renderer = initialize_renderer();

        // Initialize the camera object
        Camera cam;
        cam.update(width, height, 2.0, ((float) width / (float) height) * 2.0, 2.0);
        
        

        // Prerender the frame
        Tools::Array<ECS::RenderEntity*> entities({ ECS::create_triangle({-1.0, 0.0, 3.0}, {1.0, 0.0, 3.0}, {0.0, 1.0, 3.0}, {0.0, 0.0, -1.0}, {1.0, 1.0, 1.0}) });
        renderer->prerender(entities);
        renderer->render(cam);
        delete entities[0];

        // With the queue idle for sure, copy the result buffer back to the staging buffer
        DLOG(info, "Saving frame...");
        const Frame& result = cam.get_frame();
        result.to_png("result.png");
        result.to_ppm("result.ppm");



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
