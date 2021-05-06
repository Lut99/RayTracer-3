/* MAIN.cpp
 *   by Lut99
 *
 * Created:
 *   08/04/2021, 13:20:40
 * Last edited:
 *   06/05/2021, 16:49:51
 * Auto updated?
 *   Yes
 *
 * Description:
 *   The entry point to the RayTracer. Is mostly parsing arguments and
 *   initialising libraries.
**/

#include <iostream>
#include <cstring>
#include <limits>
#include <CppDebugger.hpp>

#include "renderer/Renderer.hpp"

#include "entities/Triangle.hpp"
#include "entities/Sphere.hpp"

#include "camera/Camera.hpp"
#include "camera/Frame.hpp"

using namespace std;
using namespace RayTracer;
using namespace CppDebugger::SeverityValues;


/***** MACROS *****/
/* Shortcut for checking a short and long label as c string. */
#define CHECK_CLI(ARG, SHORTLABEL, LONGLABEL) \
    (strcmp((ARG), (SHORTLABEL)) == 0 || strcmp((ARG), (LONGLABEL)) == 0)





/***** ENUMS *****/
enum class OutputType {
    png,
    ppm
};

// Maps the values of OutputType to a string
unordered_map<OutputType, std::string> output_type_names = {
    { OutputType::png, "png" },
    { OutputType::ppm, "ppm" }
};





/***** STRUCTS *****/
/* Stores the option as given by the command line arguments. */
struct CLIOptions {
    /* Path to the image that contains the output frame. */
    std::string output_path;
    /* Output type of the image. */
    OutputType output_type;

    /* Width of the resulting frame. */
    uint32_t width;
    /* Height of the resulting frame. */
    uint32_t height;


    /* Default constructor for the CLIOptions class, which sets the values to default. */
    CLIOptions() :
        output_path(""),
        output_type(OutputType::png),
        width(800),
        height(600)
    {}
};





/***** HELPER FUNCTIONS *****/
/* Parses command line arguments. */
int parse_cli(CLIOptions& options, int argc, const char** argv) {
    DENTER("parse_cli");

    // Make sure that there's a program name
    if (argc < 1) {
        DLOG(fatal, "No argument 0 given.");
    }

    // Go through all the given arguments
    size_t positional_index = 0;
    bool accept_options = true;
    for (int i = 1; i < argc; i++) {
        if (accept_options && argv[i][0] == '-') {
            // First, make sure that we split any value off
            std::string key = argv[i];
            std::string value = "";
            if (key.length() > 1 && key[1] != '-') {
                value = key.substr(2);
                key = key.substr(0, 2);
            } else if (key.find_first_of('=') != string::npos) {
                value = key.substr(key.find_first_of('=') + 1);
                key = key.substr(0, key.find_first_of('=') - 1);
            }

            // Check the option
            if (key == "-h" || key == "--help") {
                // It's the help function
                cout << "Usage: " << argv[0] << " [<options>] <output_path>" << endl << endl;
                
                cout << "Options:" << endl;
                cout << "\t-f,--format\tThe format of the resulting frame. Supported formats are: 'png' and 'ppm' (default: png)." << endl;
                cout << "\t-W,--width\tThe width of the resulting image, in pixels (default: 800)." << endl;
                cout << "\t-H,--height\tThe height of th resulting image, in pixels (default: 600)." << endl;

                cout << endl << "\t-h,--help\tShows this help menu, then exits." << endl << endl;

                DRETURN 0;

            } else if (key == "-f" || key == "--format") {
                // Make sure a value is given
                if (value.empty()) {
                    // Be sure there are enough values
                    if (i == argc - 1 || (accept_options && argv[i + 1][0] == '-')) {
                        cerr << key << " has no value." << endl;
                        DRETURN -1;
                    }

                    // Pop the value as format's value
                    value = argv[++i];
                }

                // Pop the value as format's value
                if (value == "png") {
                    options.output_type = OutputType::png;
                } else if (value == "ppm") {
                    options.output_type = OutputType::ppm;
                } else {
                    cerr << "Unknown output format '" << value << "'" << endl;
                    DRETURN -1;
                }
            
            } else if (key == "-W" || key == "--width") {
                // Make sure a value is given
                if (value.empty()) {
                    // Be sure there are enough values
                    if (i == argc - 1 || (accept_options && argv[i + 1][0] == '-')) {
                        cerr << key << " has no value." << endl;
                        DRETURN -1;
                    }

                    // Pop the value as width's value
                    value = argv[++i];
                }

                // Parse as unsigned integer
                try {
                    unsigned long ivalue = stoul(value);
                    if (ivalue > numeric_limits<uint32_t>::max()) {
                        cerr << "Width too large '" + value + "'";
                        DRETURN -1;
                    }
                    options.width = (uint32_t) ivalue;
                } catch (std::invalid_argument& e) {
                    cerr << "Invalid width '" + value + "'";
                    DRETURN -1;
                } catch (std::out_of_range& e) {
                    cerr << "Width too large '" + value + "'";
                    DRETURN -1;
                }

            } else if (key == "-H" || key == "--height") {
                // Make sure a value is given
                if (value.empty()) {
                    // Be sure there are enough values
                    if (i == argc - 1 || (accept_options && argv[i + 1][0] == '-')) {
                        cerr << key << " has no value." << endl;
                        DRETURN -1;
                    }

                    // Pop the value as width's value
                    value = argv[++i];
                }

                // Parse as unsigned integer
                try {
                    unsigned long ivalue = stoul(value);
                    if (ivalue > numeric_limits<uint32_t>::max()) {
                        cerr << "Height too large '" + value + "'";
                        DRETURN -1;
                    }
                    options.height = (uint32_t) ivalue;
                } catch (std::invalid_argument& e) {
                    cerr << "Invalid height '" + value + "'";
                    DRETURN -1;
                } catch (std::out_of_range& e) {
                    cerr << "Height too large '" + value + "'";
                    DRETURN -1;
                }

            } else {
                // Show that this isn't a valid option
                cerr << "Unknown option '" << argv[i] << "'" << endl << endl;

                cerr << "Run '" << argv[0] << " -h' to see a list of valid options." << endl << endl;

                DRETURN -1;

            }

        } else {
            // Treat it as a normal value
            if (positional_index == 0) {
                // Parse it as the output path
                options.output_path = argv[i];
                ++positional_index;

            } else {
                // Ignore
            }
        }
    }

    // If the value of the required arguments are still empty, throw errors
    if (options.output_path.empty()) {
        cerr << "No output path given." << endl;
        DRETURN -1;
    }
 
    // Done
    DRETURN 1;
}





/***** ENTRY POINT *****/
int main(int argc, const char** argv) {
    DSTART("main"); DENTER("main");

    // Parse the arguments
    CLIOptions options;
    int result = parse_cli(options, argc, argv);
    if (result <= 0) {
        return result;
    }

    DLOG(auxillary, "");
    DLOG(auxillary, "<<< RAYTRACER v3 >>>");
    DLOG(auxillary, "");
    DLOG(auxillary, "Options:");
    DLOG(auxillary, " - Output file  : '" + options.output_path + "'");
    DLOG(auxillary, " - Output type  : " + output_type_names[options.output_type]);
    DLOG(auxillary, " - Frame width  : " + std::to_string(options.width));
    DLOG(auxillary, " - Frame height : " + std::to_string(options.height));
    DLOG(auxillary, "");

    try {
        // Initialize the renderer
        Renderer* renderer = initialize_renderer();

        // Initialize the camera object
        Camera cam;
        cam.update(options.width, options.height, 2.0, ((float) options.width / (float) options.height) * 2.0, 2.0);
        
        

        // Prerender the frame
        Tools::Array<ECS::RenderEntity*> entities({ ECS::create_sphere({0.0, 0.0, -3.0}, 1.0, 256, 256, {1.0, 0.0, 0.0}) });
        renderer->prerender(entities); 
        renderer->render(cam);
        for (size_t i = 0; i < entities.size(); i++) {
            delete entities[i];
        }

        // With the queue idle for sure, copy the result buffer back to the staging buffer
        DLOG(info, "Saving frame...");
        const Frame& result = cam.get_frame();
        // Choose which type to store
        if (options.output_type == OutputType::png) {
            result.to_png(options.output_path);
        } else if (options.output_type == OutputType::ppm) {
            result.to_ppm(options.output_path);
        }



        // Dope, done
        DLOG(auxillary, "");
        delete renderer;
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
