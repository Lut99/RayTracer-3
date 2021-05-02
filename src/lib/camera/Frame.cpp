/* FRAME.cpp
 *   by Lut99
 *
 * Created:
 *   28/04/2021, 14:28:32
 * Last edited:
 *   02/05/2021, 17:53:04
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Contains the Frame class, which is a wrapper around CPU & Vulkan
 *   buffers to provide a conceptually easy frame to fill with colour
 *   pixels on the GPU. Also contains code to easily write to PNG, using
 *   the LodePNG library.
**/

#include <fstream>
#include <cstring>
#include <cerrno>
#include <limits>
#include <CppDebugger.hpp>

#include "LodePNG.hpp"

#include "Frame.hpp"

using namespace std;
using namespace RayTracer;
using namespace RayTracer::Compute;
using namespace CppDebugger::SeverityValues;


/***** FRAME CLASS *****/
/* Constructor for the Frame class, which takes the dimension of the frame. */
Frame::Frame(uint32_t width, uint32_t height) :
    data(new glm::vec3[this->width * this->height]),
    width(width),
    height(height)
{}

/* Copy constructor for the Frame class. */
Frame::Frame(const Frame& other) :
    width(other.width),
    height(other.height)
{
    DENTER("Frame::Frame(copy)");

    // Allocate a new CPU-side buffer
    this->data = new glm::vec3[this->width * this->height];
    // Copy the entire memory in one go
    memcpy(this->data, other.data, this->width * this->height * sizeof(glm::vec3));

    // Done
    DLEAVE;
}

/* Move constructor for the Frame class. */
Frame::Frame(Frame&& other) :
    data(other.data),
    width(other.width),
    height(other.height)
{
    // Set fields to nullptrs etc to avoid deallocation
    other.data = nullptr;
}

/* Desctructor for the Frame class. */
Frame::~Frame() {
    DENTER("Frame::~Frame");

    // Possibly deallocate the cpu_buffer, if it isn't stolen
    if (this->data != nullptr) {
        delete[] this->data;
    }

    DLEAVE;
}



/* Writes the internal frame to disk as a PNG. Assumes that the CPU buffer is synchronized with the GPU one. */
void Frame::to_png(const std::string& path) const {
    DENTER("Frame::to_png");

    // Use the internal CPU buffer to construct a 0-255, four channel vector
    vector<unsigned char> raw_image;
    raw_image.resize(4 * this->width * this->height);
    for (uint32_t i = 0; i < this->width * this->height; i++) {
        glm::vec3& pixel = this->data[i];

        // Convert to 255
        raw_image[4 * i    ] = (unsigned char) (255.0 * pixel.r);
        raw_image[4 * i + 1] = (unsigned char) (255.0 * pixel.g);
        raw_image[4 * i + 2] = (unsigned char) (255.0 * pixel.b);
        raw_image[4 * i + 3] = 255;
    }

    // With the raw image created, we can send it to the file using lodepng
    unsigned result = lodepng::encode(path.c_str(), raw_image, this->width, this->height);
    if (result != 0) {
        DLOG(fatal, "Could not write to PNG: " + std::string(lodepng_error_text(result)));
    }
    
    // Done
    DRETURN;
}

/* Writes the internal frame to disk as a PPM. Assumes that the CPU buffer is synchronized with the GPU one. */
void Frame::to_ppm(const std::string& path) const {
    DENTER("Frame::to_ppm");

    // Open a file handle
    std::ofstream h(path, ios::binary);
    if (!h.is_open()) {
        #ifdef _WIN32
        char buffer[BUFSIZ];
        strerror_s(buffer, BUFSIZ, errno);
        #else
        char* buffer = strerror(errno);
        #endif
        DLOG(fatal, "Could not open '" + path + "': " + buffer);
    }

    // Otherwise, write the PPM header
    h << "P6" << endl;
    h << "# Image rendered by the RayTracer-3" << endl;
    h << this->width << " " << this->height << endl;
    h << "255" << endl;

    // Now we write the data
    for (size_t i = 0; i < this->width * this->height; i++) {
        // Convert the floats to bytes
        glm::vec3& pixel = this->data[i];
        unsigned char r = (unsigned char) (255.0 / pixel.r);
        unsigned char g = (unsigned char) (255.0 / pixel.g);
        unsigned char b = (unsigned char) (255.0 / pixel.b);

        // Write to the file
        h.write((const char*) &r, sizeof(unsigned char));
        h.write((const char*) &g, sizeof(unsigned char));
        h.write((const char*) &b, sizeof(unsigned char));
    }

    // Alrighty! Looks good!
    h.close();
    DRETURN;
}



/* Swap operator for the Frame class. */
void RayTracer::swap(Frame& f1, Frame& f2) {
    using std::swap;

    // Simply swap all fields
    swap(f1.data, f2.data);
    swap(f1.width, f2.width);
    swap(f1.height, f2.height);
}
