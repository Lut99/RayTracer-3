/* SHADER.cpp
 *   by Lut99
 *
 * Created:
 *   27/04/2021, 14:55:16
 * Last edited:
 *   28/04/2021, 15:38:55
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Contains the Shader class, which loads, compiles & manages .spv files
 *   for use in pipelines.
**/

#include "debugger/CppDebugger.hpp"
#include <vector>
#include <fstream>
#include <cstring>
#include <cerrno>

#include "ErrorCodes.hpp"

#include "Shader.hpp"

using namespace std;
using namespace RayTracer;
using namespace RayTracer::Compute;
using namespace CppDebugger::SeverityValues;


/***** POPULATE FUNCTIONS *****/
/* Function that populates the given VkShaderModuleCreateInfo struct with the given values. */
static void populate_shader_module_info(VkShaderModuleCreateInfo& shader_module_info, const std::vector<char>& raw_shader) {
    DENTER("populate_shader_module_info");

    // Set to default
    shader_module_info = {};
    shader_module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    // Set the data of the code
    shader_module_info.codeSize = raw_shader.size();
    shader_module_info.pCode = reinterpret_cast<const uint32_t*>(raw_shader.data());

    // Done
    DRETURN;
}





/***** SHADER CLASS *****/
/* Constructor for the Shader class, which takes the device to compile for and the path of the shader. Optionally also takes the entry function to the shader. */
Shader::Shader(const GPU& gpu, const std::string& path, const std::string& entry_function) :
    gpu(gpu),
    path(path),
    entry(entry_function)
{
    DENTER("Compute::Shader::Shader");
    DLOG(info, "Initializing shader '" + path.substr(path.find_last_of("/") != string::npos ? path.find_last_of("/") + 1 : 0) + "'");
    DINDENT;

    // Simply call reload to do the work
    this->reload();

    DDEDENT;
    DLEAVE;
}

/* Copy constructor for the Shader class. */
Shader::Shader(const Shader& other) :
    gpu(other.gpu),
    path(other.path),
    entry(other.entry)
{
    DENTER("Compute::Shader::Shader(copy)");
    
    // Once again we rely on reload
    DMUTE("Compute::Shader::reload");
    this->reload();
    DUNMUTE("Compute::Shader::reload");

    DLEAVE;
}

/* Move constructor for the Shader class. */
Shader::Shader(Shader&& other) :
    gpu(other.gpu),
    vk_shader_module(other.vk_shader_module),
    path(other.path),
    entry(other.entry)
{
    // Set the shader module to nullptr to avoid deallocation
    other.vk_shader_module = nullptr;
}

/* Destructor for the Shader class. */
Shader::~Shader() {
    DENTER("Compute::Shader::~Shader");
    
    if (this->vk_shader_module != nullptr) {
        vkDestroyShaderModule(this->gpu, this->vk_shader_module, nullptr);
    }

    DLEAVE;
}



/* Reloads the shader from disk, and recompiles it. */
void Shader::reload() {
    DENTER("Compute::Shader::reload");

    // Start by loading the file
    DLOG(info, "Loading file '" + this->path + "'...");

    // Open a file handle, if at all possible
    std::ifstream h(this->path, std::ios::ate | std::ios::binary);
    if (!h.is_open()) {
        // Get the error as char array; this is mumumu SaFE oN wINdOwS
        #ifdef _WIN32
        char buffer[BUFSIZ];
        strerror_s(buffer, BUFSIZ, errno);
        #else
        char* buffer = strerror(errno);
        #endif
        DLOG(fatal, "Could not open shader file: " + std::string(buffer));
    }

    // Since we're at the end of the file, read the position to know the size of our file buffer
    size_t file_size = h.tellg();

    // Initialize the buffer. We're using the std::vector to get proper alignment
    std::vector<char> buffer;
    buffer.resize(file_size);

    // Next, we read all bytes in one go
    h.seekg(0);
    h.read(buffer.data(), file_size);
    h.close();

    

    // With the bytes read, create the shader module from it
    DLOG(info, "Compiling " + std::to_string(file_size) + " bytes of shader code...");

    // Populate the create info
    VkShaderModuleCreateInfo shader_module_info;
    populate_shader_module_info(shader_module_info, buffer);

    // With the struct populated, create the actual module
    VkResult vk_result;
    if ((vk_result = vkCreateShaderModule(this->gpu, &shader_module_info, nullptr, &this->vk_shader_module)) != VK_SUCCESS) {
        DLOG(fatal, "Could not compile shader: " + vk_error_map[vk_result]);
    }

    // Done!
    DRETURN;
}



/* Move assignment operator for the Shader class. */
Shader& Shader::operator=(Shader&& other) {
    if (this != &other) { swap(*this, other); }
    return *this;
}

/* Swap operator for the Shader class. */
void Compute::swap(Shader& s1, Shader& s2) {
    DENTER("Compute::swap(Shader)");

    using std::swap;

    #ifndef NDEBUG
    // If the GPU is not the same, then initialize to all nullptrs and everything
    if (s1.gpu != s2.gpu) {
        DLOG(fatal, "Cannot swap shaders with different GPUs");
    }
    #endif

    // Swap all fields
    swap(s1.vk_shader_module, s2.vk_shader_module);
    swap(s1.path, s2.path);
    swap(s1.entry, s2.entry);

    // Done
    DRETURN;
}
