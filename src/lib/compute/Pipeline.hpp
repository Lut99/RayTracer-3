/* PIPELINE.hpp
 *   by Lut99
 *
 * Created:
 *   27/04/2021, 14:44:28
 * Last edited:
 *   27/04/2021, 15:31:07
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Contains the Pipeline class, which defines the compute pipeline, and
 *   thus how the shaders are organised and how they get their data.
**/

#ifndef COMPUTE_PIPELINE_HPP
#define COMPUTE_PIPELINE_HPP

#include <vulkan/vulkan.h>

#include "GPU.hpp"
#include "DescriptorSetLayout.hpp"
#include "Shader.hpp"

namespace RayTracer::Compute {
    /* The Pipeline class, which is the pipeline that actually contains our compute shaders. */
    class Pipeline {
    private:
        /* Immutable reference to the GPU object where the pipeline lives. */
        const GPU& gpu;

        /* The actual VkPipeline object this class wraps. */
        VkPipeline vk_compute_pipeline;

    private:
        /* Constructor for the Pipeline class, which takes the GPU where we create it for, the shader to load and the set of descriptor set layouts which describe all buffers used in the pipeline.  */
        Pipeline(const GPU& gpu, const Shader& shader, const Tools::Array<DescriptorSetLayout>& descriptor_set_layouts);
        /* Cope constructor for the Pipeline class, which is deleted. */
        Pipeline(const Pipeline& other) = delete;
        /* Move constructor for the Pipeline class. */
        Pipeline(Pipeline&& other);
        /* Destructor for the Pipeline class. */
        ~Pipeline();

        /* Explicitly returns the inner VkPipeline object. */
        inline VkPipeline compute_pipeline() const { return this->vk_compute_pipeline; }
        /* Implicitly returns the inner VkPipeline object. */
        inline operator VkPipeline() const { return this->vk_compute_pipeline; }

        /* Copy assignment operator for the Pipeline class, which is deleted. */
        Pipeline& operator=(const Pipeline& other) = delete;
        /* Move assignment operator for the Pipeline class. */
        Pipeline& operator=(Pipeline&& other);
        /* Swap operator for the Pipeline class. */
        friend void swap(Pipeline& p1, Pipeline& p2);

    };

    /* Swap operator for the Pipeline class. */
    void swap(Pipeline& p1, Pipeline& p2);

}

#endif
