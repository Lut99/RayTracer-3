/* CAMERA.cpp
 *   by Lut99
 *
 * Created:
 *   28/04/2021, 20:08:23
 * Last edited:
 *   25/05/2021, 18:14:13
 * Auto updated?
 *   Yes
 *
 * Description:
 *   The camera class, which computes the required camera matrices
 *   per-frame and can optionally move the camera in between frames.
**/

#include <limits>
#include <CppDebugger.hpp>

#include "Camera.hpp"

using namespace std;
using namespace RayTracer;
using namespace CppDebugger::SeverityValues;


/***** CAMERA CLASS *****/
/* Constructor for the Camera class, */
Camera::Camera() :
    frame(nullptr)
{}

/* Copy constructor for the Camera class. */
Camera::Camera(const Camera& other) :
    origin(other.origin),
    horizontal(other.horizontal),
    vertical(other.vertical),
    lower_left_corner(other.lower_left_corner),
    frame(other.frame)
{
    DENTER("Camera::Camera(copy)");

    // Copy the frame (if any)
    if (this->frame != nullptr) {
        this->frame = new Frame(*other.frame);
    }

    // Done
    DLEAVE;
}

/* Move constructor for the Camera class. */
Camera::Camera(Camera&& other)  :
    origin(other.origin),
    horizontal(other.horizontal),
    vertical(other.vertical),
    lower_left_corner(other.lower_left_corner),
    frame(other.frame)
{
    // Set the other pointers w/e to null to avoid deallocation
    other.frame = nullptr;
}

/* Destructor for the Camera class. */
Camera::~Camera() {
    DENTER("Camera::~Camera");

    if (this->frame != nullptr) {
        delete this->frame;
    }

    DLEAVE;
}



/* Computes new camera matrices for the given position and orientation. */
void Camera::update(uint32_t width, uint32_t height, float focal_length, float viewport_width, float viewport_height /* TBD */) {
    DENTER("Camera::update");

    // If there already exists a frame, destroy it
    if (this->frame != nullptr) {
        delete this->frame;
    }

    // First, initialize the frame struct
    this->frame = new Frame(width, height);
    
    // Next, compute all new vectors
    this->origin = glm::vec3(0.0, 0.0, 0.0);
    this->horizontal = glm::vec3(viewport_width, 0.0, 0.0);
    this->vertical = glm::vec3(0.0, viewport_height, 0.0);
    this->lower_left_corner = this->origin - this->horizontal / glm::vec3(2.0) - this->vertical / glm::vec3(2.0) - glm::vec3(0.0, 0.0, focal_length);

    // Done
    DRETURN;
}



/* Swap operator for the Camera class. */
void RayTracer::swap(Camera& c1, Camera& c2) {
    using std::swap;

    // Swap all fields
    swap(c1.origin, c2.origin);
    swap(c1.horizontal, c2.horizontal);
    swap(c1.vertical, c2.vertical);
    swap(c1.lower_left_corner, c2.lower_left_corner);
    swap(c1.frame, c2.frame);
}
