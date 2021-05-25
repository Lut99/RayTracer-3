/* FRAME.hpp
 *   by Lut99
 *
 * Created:
 *   28/04/2021, 14:28:35
 * Last edited:
 *   25/05/2021, 16:54:51
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Contains the Frame class, which is a wrapper around CPU & Vulkan
 *   buffers to provide a conceptually easy frame to fill with colour
 *   pixels on the GPU. Also contains code to easily write to PNG, using
 *   the LodePNG library.
**/

#ifndef CAMERA_FRAME_HPP
#define CAMERA_FRAME_HPP

#include "glm/glm.hpp"

namespace RayTracer {
    /* The Pixel struct, which directly represents a single pixel. Can also be indexed by index. */
    struct Pixel {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };

    /* The IPixel union, which overlays a pixel over a uint32_t to make indexing a whole lotta easier. */
    union IPixel {
        uint32_t raw;
        Pixel pixel;  
    };



    /* The Frame class, which represents a single image to be rendered by the RayTracer. */
    class Frame {
    private:
        /* The actual frame buffer, tightly packed as 32-bit unsigned integers that represent RGBA. Note that the actual order is secretly BGRA. */
        uint32_t* data;
        /* The width, in pixels, of the frame. */
        uint32_t width;
        /* The height, in pixels, of the frame. */
        uint32_t height;

    public:
        /* Constructor for the Frame class, which takes the dimension of the frame. */
        Frame(uint32_t width, uint32_t height);
        /* Copy constructor for the Frame class. */
        Frame(const Frame& other);
        /* Move constructor for the Frame class. */
        Frame(Frame&& other);
        /* Desctructor for the Frame class. */
        ~Frame();
        
        /* Writes the internal frame to disk as a PNG. Assumes that the CPU buffer is synchronized with the GPU one. */
        void to_png(const std::string& path) const;
        /* Writes the internal frame to disk as a PPM. Assumes that the CPU buffer is synchronized with the GPU one. */
        void to_ppm(const std::string& path) const;

        /* Returns the width of the frame. */
        inline uint32_t w() const { return this->width; }
        /* Returns the height of the frame. */
        inline uint32_t h() const { return this->height; }
        /* Allows access to the internal frame buffer. */
        inline uint32_t* d() const { return this->data; }

        /* Returns the pixel at the given index. */
        inline IPixel operator[](size_t index) { IPixel result; result.raw = this->data[index]; return result; }
        
        /* Copy assignment operator for the Frame class. */
        inline Frame& operator=(const Frame& other) { return *this = Frame(other); }
        /* Move assignment operator for the Frame class. */
        inline Frame& operator=(Frame&& other) { if (this != &other) { swap(*this, other); } return *this; }
        /* Swap operator for the Frame class. */
        friend void swap(Frame& f1, Frame& f2);

    };

    /* Swap operator for the Frame class. */
    void swap(Frame& f1, Frame& f2);
}

#endif
