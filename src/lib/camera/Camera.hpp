/* CAMERA.hpp
 *   by Lut99
 *
 * Created:
 *   28/04/2021, 20:08:29
 * Last edited:
 *   02/05/2021, 18:09:11
 * Auto updated?
 *   Yes
 *
 * Description:
 *   The camera class, which computes the required camera matrices
 *   per-frame and can optionally move the camera in between frames.
**/

#ifndef CAMERA_CAMERA_HPP
#define CAMERA_CAMERA_HPP

#include "glm/glm.hpp"

#include "Frame.hpp"

namespace RayTracer {
    /* The Camera class, which computes the required camera matrices for each frame. */
    class Camera {
    public:
        /* The origin point of the camera, conceptually. */
        glm::vec3 origin;
        /* The vector pointing the window to the right. */
        glm::vec3 horizontal;
        /* The vector pointing the window up. */
        glm::vec3 vertical;
        /* The bottom left corner of the window we render through. */
        glm::vec3 lower_left_corner;

    private:
        /* The internal Frame that the result is rendered to. */
        Frame* frame;

    public:
        /* Constructor for the Camera class, */
        Camera();
        /* Copy constructor for the Camera class. */
        Camera(const Camera& other);
        /* Move constructor for the Camera class. */
        Camera(Camera&& other);
        /* Destructor for the Camera class. */
        ~Camera();

        /* Computes new camera matrices for the given position and orientation. */
        void update(uint32_t width, uint32_t height, float focal_length, float viewport_width, float viewport_height);

        /* Returns the width (in pixels) of the current camera frame. Will probably segfault if not set. */
        inline uint32_t w() const { return this->frame->w(); }
        /* Returns the height (in pixels) of the current camera frame. Will probably segfault if not set. */
        inline uint32_t h() const { return this->frame->h(); }
        /* Returns the result of a render as a constant reference to the internal frame. Should of course only be used once the rendering is done. */
        inline const Frame& get_frame() const { return *this->frame; }

        /* Copy assignment operator for the Camera class. */
        inline Camera& operator=(const Camera& other) { return *this = Camera(other); }
        /* Move assignment operator for the Camera class. */
        inline Camera& operator=(Camera&& other) { if (this != &other) { swap(*this, other); } return *this; }
        /* Swap operator for the Camera class. */
        friend void swap(Camera& c1, Camera& c2);

    };

    /* Swap operator for the Camera class. */
    void swap(Camera& c1, Camera& c2);
}

#endif
