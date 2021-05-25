/* OBJECT.cpp
 *   by Lut99
 *
 * Created:
 *   06/05/2021, 16:51:56
 * Last edited:
 *   25/05/2021, 18:14:13
 * Auto updated?
 *   Yes
 *
 * Description:
 *   File that contains an entity that loads object files. Does not yet
 *   load textures, just the normals and the geometries.
**/

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cerrno>
#include <CppDebugger.hpp>

#include "Object.hpp"

using namespace std;
using namespace RayTracer;
using namespace RayTracer::ECS;
using namespace CppDebugger::SeverityValues;


/***** MACROS *****/
/* Shortcut for getting and checking a character in the tokenizer. */
#define GET_CHAR(C, H) \
    int ic = fgetc((H)); \
    if (ic == EOF && feof((H))) { \
        return Terminal::eof; \
    } else if (ic == EOF && ferror((H))) { \
        cerr << "ERROR: Could not read next character from input file: " << strerror(errno) << endl; \
        return Terminal::unknown; \
    } \
    ++col; \
    (C) = (char) ic;

/* Checks if given character is a whitespace (space, tab, carriage return or newline). */
#define IS_WHITESPACE(C) \
    ((C) == ' ' || (C) == '\t' || (C) == '\r' || (C) == '\n')





/***** OBJECT FUNCTIONS *****/
/* Creates a new Object struct based on the given properties. Note that the actual loading of the object is done during pre-rendering. */
Object* ECS::create_object(const std::string& file_path, const glm::vec3& center, float scale, const glm::vec3& color) {
    DENTER("ECS::create_object");

    // Allocate the struct
    Object* result = new Object;

    // Set the RenderEntity fields
    result->type = EntityType::et_object;
    result->pre_render_mode = EntityPreRenderModeFlags::eprmf_cpu;
    result->pre_render_operation = EntityPreRenderOperation::epro_load_object_file;
    // Set the number of faces & vertices to 0 as we will read them later
    result->pre_render_faces = 0;
    result->pre_render_vertices = 0;

    // Set the object file's path
    result->file_path = file_path;

    // Set the conceptual properties of the object
    result->center = center;
    result->scale = scale;

    // Set the rendering properties of the sphere
    result->color = color;

    // Next, open the file to read how many faces & vertices we need
    
    // Try to open a file handle
    std::ifstream h(file_path);
    if (!h.is_open()) {
        #ifdef _WIN32
        char buffer[BUFSIZ];
        strerror_s(buffer, BUFSIZ, errno);
        #else
        char* buffer = strerror(errno);
        #endif
        DLOG(fatal, "Could not open file: " + std::string(buffer));
    }

    // Start looping through it
    size_t line_i = 1;
    std::string line;
    while(std::getline(h, line)) {
        // First, try to parse the whole line as a triplet of floats with a character before it
        std::stringstream sstr(line);
        char type;
        float v1, v2, v3;
        if (!(sstr >> type >> v1 >> v2 >> v3)) {
            DLOG(fatal, "Encountered unreadable line on line " + std::to_string(line_i));
        }
        (void) v1;
        (void) v2;
        (void) v3;

        // Examine the correct mode
        if (type == 'f') {
            // It's a face
            ++result->pre_render_faces;
        } else if (type == 'v') {
            // It's a vertex
            ++result->pre_render_vertices;
        }
        // We ignore the warning since we'll pre-render it later anyway

        // Increment the line to progress
        line_i++;
    }

    // When done, close the file
    h.close();

    // Done!
    DRETURN result;
}



/* Pre-renders the sphere on the CPU, single-threaded. Basically just loads the file given on creation. Since object files are usually indexed, so are we. */
void ECS::cpu_pre_render_object(Tools::Array<GFace>& faces_buffer, Tools::Array<glm::vec4>& vertex_buffer, Object* obj) {
    DENTER("ECS::cpu_pre_render_object");
    DLOG(info, "Pre-rendering Object by loading file '" + obj->file_path + "'...");

    // Try to open a file handle
    std::ifstream h(obj->file_path);
    if (!h.is_open()) {
        #ifdef _WIN32
        char buffer[BUFSIZ];
        strerror_s(buffer, BUFSIZ, errno);
        #else
        char* buffer = strerror(errno);
        #endif
        DLOG(fatal, "Could not open file: " + std::string(buffer));
    }

    // Start looping through it
    size_t line_i = 1;
    size_t vertex_i = 0;
    size_t faces_i = 0;
    std::string line;
    while(std::getline(h, line)) {
        // First, try to parse the whole line as a triplet of floats with a character before it
        std::stringstream sstr(line);
        char type;
        float v1, v2, v3;
        if (!(sstr >> type >> v1 >> v2 >> v3)) {
            DLOG(fatal, "Encountered unreadable line on line " + std::to_string(line_i));
        }

        // Examine the correct mode
        if (type == 'v') {
            // Store as new vertex
            vertex_buffer[vertex_i++] = glm::vec4(obj->center + obj->scale * glm::vec3(v1, v2, v3), 0.0);
        } else if (type == 'f') {
            // Store as new faces of indices
            faces_buffer[faces_i++] = {
                (uint32_t) v1, (uint32_t) v2, (uint32_t) v3,
                glm::vec3(0.0, 0.0, 0.0),
                obj->color
            };
        } else {
            DLOG(warning, (std::string("Encountered line with unknown type '") += type) + "'");
        }

        // Increment the line to progress
        line_i++;
    }

    // Find the index offset in case the indices aren't zero-indexed
    uint32_t index_offset = numeric_limits<uint32_t>::max();
    for (size_t i = 0; i < faces_buffer.size(); i++) {
        if (faces_buffer[i].v1 < index_offset) { index_offset = faces_buffer[i].v1; }
        if (faces_buffer[i].v2 < index_offset) { index_offset = faces_buffer[i].v2; }
        if (faces_buffer[i].v3 < index_offset) { index_offset = faces_buffer[i].v3; }
    }

    // Then, scale the offsets and compute the normals / color
    for (size_t i = 0; i < faces_buffer.size(); i++) {
        faces_buffer[i].v1 -= index_offset;
        faces_buffer[i].v2 -= index_offset;
        faces_buffer[i].v3 -= index_offset;
        faces_buffer[i].normal = glm::normalize(glm::cross(glm::vec3(vertex_buffer[faces_buffer[i].v3]) - glm::vec3(vertex_buffer[faces_buffer[i].v1]), glm::vec3(vertex_buffer[faces_buffer[i].v2]) - glm::vec3(vertex_buffer[faces_buffer[i].v1])));
        faces_buffer[i].color *= glm::abs(glm::dot(faces_buffer[i].normal, glm::vec3(0.0, 0.0, -1.0)));
    }

    // We're done
    DRETURN;
}
