/* RENDER OBJECT.cpp
 *   by Lut99
 *
 * Created:
 *   29/04/2021, 15:51:30
 * Last edited:
 *   29/04/2021, 17:04:04
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Contains the RenderObject class, which is the baseclass for all
 *   objects rendered with the RayTracer.
**/

#include <cstring>
#include <cmath>
#include <CppDebugger.hpp>

#include "RenderObject.hpp"

using namespace std;
using namespace RayTracer;
using namespace CppDebugger::SeverityValues;


/***** RENDEROBJECT CLASS *****/
/* Protected constructor of the RenderObject class. */
RenderObject::RenderObject(uint32_t n_cached_vertices) :
    cvertices(nullptr),
    cnormals(nullptr),
    ccolors(nullptr),
    n_cached_vertices(n_cached_vertices),
    cache_valid(false)
{
    DENTER("RenderObject::RenderObject");

    // Do a check on the cache size
    if (this->n_cached_vertices > RayTracer::max_object_vertices) {
        DLOG(fatal, "Cannot store more vertices in caches than are maximally allowed per object (" + std::to_string(this->n_cached_vertices) + " > " + std::to_string(RayTracer::max_object_vertices) + ")");
    }

    DLEAVE;
}

/* Copy constructor for the RenderObject class. */
RenderObject::RenderObject(const RenderObject& other) :
    cvertices(other.cvertices),
    cnormals(other.cnormals),
    ccolors(other.ccolors),
    n_cached_vertices(other.n_cached_vertices),
    cache_valid(other.cache_valid)
{
    DENTER("RenderObject::RenderObject(copy)");
    
    // Allocate a new vertex cache buffer if it's not a nullptr
    if (this->cvertices != nullptr) {
        this->cvertices = new glm::vec4[3 * this->n_cached_vertices];
        memcpy(this->cvertices, other.cvertices, 3 * this->n_cached_vertices * sizeof(glm::vec4));
    }

    // Allocate a new normal cache buffer if it's not a nullptr
    if (this->cnormals != nullptr) {
        this->cnormals = new glm::vec4[this->n_cached_vertices];
        memcpy(this->cnormals, other.cnormals, this->n_cached_vertices * sizeof(glm::vec4));
    }

    // Allocate a new color cache buffer if it's not a nullptr
    if (this->ccolors != nullptr) {
        this->ccolors = new glm::vec4[this->n_cached_vertices];
        memcpy(this->ccolors, other.ccolors, this->n_cached_vertices * sizeof(glm::vec4));
    }

    // Done
    DLEAVE;
}

/* Move constructor for the RenderObject class. */
RenderObject::RenderObject(RenderObject&& other) :
    cvertices(other.cvertices),
    cnormals(other.cnormals),
    ccolors(other.ccolors),
    n_cached_vertices(other.n_cached_vertices),
    cache_valid(other.cache_valid)
{
    // Prevent deallocation by the moved object
    other.cvertices = nullptr;
    other.cnormals = nullptr;
    other.ccolors = nullptr;
}

/* (Virtual) destructor for the RenderObject class. */
RenderObject::~RenderObject() {
    DENTER("RenderObject::~RenderObject");

    // Deallocate the cached buffers if they exist
    if (this->cvertices != nullptr) {
        delete[] this->cvertices;
    }
    if (this->cnormals != nullptr) {
        delete[] this->cnormals;
    }
    if (this->ccolors != nullptr) {
        delete[] this->ccolors;
    }

    DLEAVE;
}



/* Updates the render object, i.e., lets the derived class update itself accordingly. */
void RenderObject::update() {
    DENTER("RenderObject::update");

    // Call the actual update function
    this->__update();

    DRETURN;
}

/* Draws the RenderObjectData, but stores it in internal caches instead of pushing it to the global array. */
void RenderObject::draw_cached() {
    DENTER("RenderObject::draw_cached");

    // If the caches are still valid, there's nothing to do
    if (this->cache_valid) {
        DRETURN;
    }

    // If the caches do not exist, allocate them
    if (this->cvertices != nullptr) {
        this->cvertices = new glm::vec4[3 * this->n_cached_vertices];
    }
    if (this->cnormals != nullptr) {
        this->cnormals = new glm::vec4[this->n_cached_vertices];
    }
    if (this->ccolors != nullptr) {
        this->ccolors = new glm::vec4[this->n_cached_vertices];
    }

    // Next, call the draw function to populate the caches
    this->__draw(this->cvertices, this->cnormals, this->ccolors);

    // Mark the caches as valid and return
    this->cache_valid = true;
    DRETURN;
}

/* Draws the RenderObjectData to the given vertex, normal and color buffers. Also computes the indices and stores them in the returned RenderObjectData. If cached is true, then the actual vertices are stored in internal buffers. */
void RenderObject::draw(RenderObjectData* object_data, uint32_t& n_vertices, glm::vec4* vertices, glm::vec4* normals, glm::vec4* colors, bool cached) const {
    DENTER("RenderObject::draw");

    // If the number of cached vertices is 0, then stop
    if (this->n_cached_vertices == 0) {
        DLOG(fatal, "Cannot draw object of 0 vertices.");
    }

    // Prepare local matrices of at least the correct size
    glm::vec4* lvertices;
    glm::vec4* lnormals;
    glm::vec4* lcolors;
    if (cached && this->cache_valid) {
        // If we are allowed to use caches and the caches are, in fact, valid, then use them
        lvertices = this->cvertices;
        lnormals = this->cnormals;
        lcolors = this->ccolors;
    } else {
        // Otherwise, allocate new spaces and generate them
        lvertices = new glm::vec4[3 * this->n_cached_vertices];
        lnormals = new glm::vec4[this->n_cached_vertices];
        lcolors = new glm::vec4[this->n_cached_vertices];
        this->__draw(lvertices, lnormals, lcolors);
    }

    // With the vertices etc generated, generate a list of indices and add all non-existing vertices etc we need to the lists
    object_data->aabb[0] = glm::vec4(INFINITY, INFINITY, INFINITY, 0.0);
    object_data->aabb[1] = glm::vec4(-INFINITY, -INFINITY, -INFINITY, 0.0);
    for (uint32_t i = 0; i < this->n_cached_vertices; i++) {
        // First, make sure the bounding box encapsulates this vertex entirely
        for (uint32_t vertex_i = 0; vertex_i < 3; vertex_i++) {
            for (uint32_t d = 0; d < 3; d++) {
                if (lvertices[3 * i + vertex_i][d] < object_data->aabb[0][d]) {
                    object_data->aabb[0][d] = lvertices[3 * i + vertex_i][d];
                }
                if (lvertices[3 * i + vertex_i][d] > object_data->aabb[1][d]) {
                    object_data->aabb[1][d] = lvertices[3 * i + vertex_i][d];
                }
            }
        }

        // Next, cross-reference this vertex with the list
        bool found = false;
        for (uint32_t j = 0; j < n_vertices; j++) {
            // Check if this vertex is precisely the same
            if (vertices[3 * j    ] == lvertices[3 * i    ] &&
                vertices[3 * j + 1] == lvertices[3 * i + 1] &&
                vertices[3 * j + 2] == lvertices[3 * i + 2] &&
                normals[j] == lnormals[i] &&
                colors[j] == lcolors[i]) 
            {
                // They are; store the index in the index buffer and continue to the next
                object_data->indices[object_data->n_vertices++] = j;
                found = true;
                break;
            }
        }

        // If no match is found...
        if (!found) {
            // ...check if we have space
            if (n_vertices >= RayTracer::max_world_vertices) {
                DLOG(fatal, "Exceeding maximum number of vertices in the world.");
            }

            // ...append it to the list
            memcpy(vertices + 3 * n_vertices, lvertices + 3 * i, 3 * sizeof(glm::vec4));
            normals[n_vertices] = lnormals[i];
            colors[n_vertices] = lcolors[i];

            // ...and store the new index
            object_data->indices[object_data->n_vertices++] = n_vertices;
            ++n_vertices;
        }
    }

    // If we didn't use the cached variables, then deallocate
    if (!cached || !this->cache_valid) {
        delete[] lvertices;
        delete[] lnormals;
        delete[] lcolors;
    }

    // Done
    DRETURN;
}



/* Swap operator for the RenderObject class (useful for abstracting it away). */
void RayTracer::swap(RenderObject& ro1, RenderObject& ro2) {
    DENTER("RayTracer::swap(RenderObject)");

    using std::swap;

    swap(ro1.cvertices, ro2.cvertices);
    swap(ro1.cnormals, ro2.cnormals);
    swap(ro1.ccolors, ro2.ccolors);
    swap(ro1.n_cached_vertices, ro2.n_cached_vertices);

    // Done
    DRETURN;
}
