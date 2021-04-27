/* SHADER.hpp
 *   by Lut99
 *
 * Created:
 *   27/04/2021, 14:56:30
 * Last edited:
 *   27/04/2021, 15:27:43
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Contains the Shader class, which loads, compiles & manages .spv files
 *   for use in pipelines.
**/

#ifndef COMPUTE_SHADER_HPP
#define COMPUTE_SHADER_HPP

#include <vulkan/vulkan.h>

#include "GPU.hpp"

namespace RayTracer::Compute {
    /* The Shader class, which loads, compiles and manages .spv files. */
    class Shader {
    private:
        /* Immutable reference to the device where we will compile the shader for. */
        const GPU& gpu;

        /* The VkShaderModule object that we wrap and will compile. */
        VkShaderModule vk_shader_module;
        /* The path where the shader resides that we're managing. */
        std::string path;
        /* The entry function of the shader. */
        std::string entry;
    
    public:
        /* Constructor for the Shader class, which takes the device to compile for and the path of the shader. Optionally also takes the entry function to the shader. */
        Shader(const GPU& gpu, const std::string& path, const std::string& entry_function = "main");
        /* Copy constructor for the Shader class. */
        Shader(const Shader& other);
        /* Move constructor for the Shader class. */
        Shader(Shader&& other);
        /* Destructor for the Shader class. */
        ~Shader();

        /* Reloads the shader from disk, and recompiles it. */
        void reload();

        /* Returns the entry function of the shader. */
        inline const std::string& entry_function() const { return this->entry; }
        /* Explicitly returns the internal VkShaderModule object. */
        inline VkShaderModule shader_module() const { return this->vk_shader_module; }
        /* Implicitly returns the internal VkShaderModule object. */
        inline operator VkShaderModule() const { return this->vk_shader_module; }

        /* Copy assignment operator for the Shader class. */
        inline Shader& operator=(const Shader& other) { return *this = Shader(other); }
        /* Move assignment operator for the Shader class. */
        Shader& operator=(Shader&& other);
        /* Swap operator for the Shader class. */
        friend void swap(Shader& s1, Shader& s2);

    };

    /* Swap operator for the Shader class. */
    void swap(Shader& s1, Shader& s2);
}

#endif
