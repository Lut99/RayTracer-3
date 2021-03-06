/* FRAME.cpp
 *   by Lut99
 *
 * Created:
 *   28/04/2021, 14:28:32
 * Last edited:
 *   25/05/2021, 18:14:13
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
using namespace CppDebugger::SeverityValues;


/***** FRAME CLASS *****/
/* Constructor for the Frame class, which takes the dimension of the frame. */
Frame::Frame(uint32_t width, uint32_t height) :
    data(new uint32_t[width * height]),
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
    this->data = new uint32_t[this->width * this->height];
    // Copy the entire memory in one go
    memcpy(this->data, other.data, this->width * this->height * sizeof(uint32_t));

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
        uint32_t pixel = this->data[i];

        // Unpack the individual pixel values
        raw_image[4 * i    ] = (pixel >> 24) & 0xFF; // r
        raw_image[4 * i + 1] = (pixel >> 16) & 0xFF; // g
        raw_image[4 * i + 2] = (pixel >>  8) & 0xFF; // b
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
        // Unpack the integer
        uint32_t pixel = this->data[i];
        unsigned char r = (pixel >> 24) & 0xFF;
        unsigned char g = (pixel >> 16) & 0xFF;
        unsigned char b = (pixel >>  8) & 0xFF;
        DLOG(info, "pixel: (" + std::to_string(r) + "," + std::to_string(g) + "," + std::to_string(b) + ")");

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
