/* SPHERE.cpp
 *   by Lut99
 *
 * Created:
 *   01/05/2021, 12:45:50
 * Last edited:
 *   25/05/2021, 17:23:26
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Contains code for pre-rendering a Sphere on the CPU (single threaded).
**/

// #define ENABLE_VULKAN

#define _USE_MATH_DEFINES
#include <cmath>
#include "debugger/CppDebugger.hpp"

#ifdef ENABLE_VULKAN
#include "renderer/Vertex.hpp"

#include "compute/ErrorCodes.hpp"
#include "compute/DescriptorSetLayout.hpp"
#include "compute/Shader.hpp"
#include "compute/Pipeline.hpp"

#include "tools/Common.hpp"
#endif

#include "Sphere.hpp"

using namespace std;
using namespace RayTracer;
#ifdef ENABLE_VULKAN
using namespace RayTracer::Compute;
#endif
using namespace RayTracer::ECS;
using namespace CppDebugger::SeverityValues;


/***** STRUCTS *****/
#ifdef ENABLE_VULKAN
/* Data of a Sphere to transfer to the GPU for rendering. */
struct SphereData {
    /* The center of the sphere. */
    alignas(16) glm::vec3 center;
    /* The radius of the sphere. */
    alignas(4)  float radius;
    /* The number of meridians in the sphere (i.e., vertical lines). */
    alignas(4)  uint32_t n_meridians;
    /* The number of parallels in the sphere (i.e., horizontal lines). */
    alignas(4)  uint32_t n_parallels;
    /* The color of the sphere. */
    alignas(16) glm::vec3 color;
};
#endif

#define GLM_STR(V) \
    ("{" + std::to_string((V).x) + "," + std::to_string((V).y) + "," + std::to_string((V).z) + "}")





/***** HELPER FUNCTIONS *****/
/* Computes the coordinates of a single point on the sphere. */
static glm::vec3 compute_point(float fx, float fy, Sphere* sphere) {
    DENTER("ECS::compute_point");

    glm::vec3 result = sphere->center + sphere->radius * glm::vec3(
        sin(M_PI * (fy / (float) (sphere->n_parallels - 1))) * cos(2 * M_PI * (fx / (float) sphere->n_meridians)),
        cos(M_PI * (fy / (float) (sphere->n_parallels - 1))),
        sin(M_PI * (fy / (float) (sphere->n_parallels - 1))) * sin(2 * M_PI * (fx / (float) sphere->n_meridians))
    );

    DRETURN result;
}





/***** SPHERE FUNCTIONS *****/
/* Creates a new Sphere struct based on the given properties. */
Sphere* ECS::create_sphere(const glm::vec3& center, float radius, uint32_t n_meridians, uint32_t n_parallels, const glm::vec3& color) {
    DENTER("ECS::create_sphere");

    // Allocate the struct
    Sphere* result = new Sphere;

    // Set the RenderEntity fields
    result->type = EntityType::et_sphere;
    result->pre_render_mode = EntityPreRenderModeFlags::eprmf_cpu;
    #ifdef ENABLE_VULKAN
    result->pre_render_mode = result->pre_render_mode | EntityPreRenderModeFlags::eprmf_gpu;
    #endif
    result->pre_render_operation = EntityPreRenderOperation::epro_generate_sphere;
    // Compute how many faces & vertices to generate
    result->pre_render_faces = n_meridians + 2 * ((n_parallels - 3) * n_meridians) + n_meridians;
    result->pre_render_vertices = 2 + (n_parallels - 2) * n_meridians;

    // Set the conceptual properties of the sphere
    result->center = center;
    result->radius = radius;

    // Set the rendering properties of the sphere
    result->n_meridians = n_meridians;
    result->n_parallels = n_parallels;
    result->color = color;

    // Done!
    DRETURN result;
}



/* Pre-renders the sphere on the CPU, single-threaded, and returns a list of CPU-side buffers with the GFaces and the vertices. Assumes the given buffers contains irrelevant data, and already have the correct size. */
void ECS::cpu_pre_render_sphere(Tools::Array<GFace>& faces_buffer, Tools::Array<glm::vec4>& vertex_buffer, Sphere* sphere) {
    DENTER("ECS::cpu_pre_render_sphere");
    DLOG(info, "Pre-rendering sphere with " + std::to_string(sphere->n_meridians) + " meridians and " + std::to_string(sphere->n_parallels) + " parallels...");
    DINDENT;

    // Make shortcuts for the maximum x & y
    uint32_t max_x = sphere->n_meridians;
    uint32_t max_y = sphere->n_parallels;

    // Loop through all coordinates we need to do
    for (uint32_t y = 1; y < max_y; y++) {
        for (uint32_t x = 0; x < max_x; x++) {
            // Compute x & y - 1
            uint32_t x_m1 = x > 0 ? x - 1 : max_x - 1;
            uint32_t y_m1 = y - 1;

            // Switch to the correct mode
            if (y == 1) {
                // North pole

                // Get the index of the polar point
                uint32_t p1 = 0;
                // Get the index of the previous point in this circle
                uint32_t p2 = 1 + x_m1;
                // Get the index of the current point in this circle
                uint32_t p3 = 1 + x;

                // Compute the matching points
                glm::vec3 v1 = compute_point(0.0, 0.0, sphere);
                glm::vec3 v2 = compute_point((float) x_m1, (float) y, sphere);
                glm::vec3 v3 = compute_point((float) x, (float) y, sphere);

                // Compute the normal for these fellas
                glm::vec3 n = glm::normalize(glm::cross(v3 - v1, v2 - v1));
                // And finally, compute the color
                glm::vec3 c = sphere->color * abs(glm::dot(n, glm::vec3(0.0, 0.0, -1.0)));

                // Store the face
                faces_buffer[x].v1 = p1;
                faces_buffer[x].v2 = p2;
                faces_buffer[x].v3 = p3;
                faces_buffer[x].normal = n;
                faces_buffer[x].color = c;

                // Store the points
                vertex_buffer[p1] = glm::vec4(v1, 0.0);
                vertex_buffer[p2] = glm::vec4(v2, 0.0);
                vertex_buffer[p3] = glm::vec4(v3, 0.0);
            } else if (y < max_y - 1) {
                // // In between two circles

                // First, pre-compute the base index of this layer's vertices in the resulting buffer
                uint32_t f_index = max_x + 2 * (y - 2) * max_x;

                // Get the index of the previous point in previous circle
                uint32_t p1 = 1 + (y_m1 - 1) * max_x + x_m1;
                // Get the index of the current point in previous circle
                uint32_t p2 = 1 + (y_m1 - 1) * max_x + x;
                // Get the index of the previous point in this circle
                uint32_t p3 = 1 + (y - 1) * max_x + x_m1;
                // Get the index of the current point in this circle
                uint32_t p4 = 1 + (y - 1) * max_x + x;
                
                // Compute the matching points
                glm::vec3 v1 = compute_point((float) x_m1, (float) y_m1, sphere);
                glm::vec3 v2 = compute_point((float) x, (float) y_m1, sphere);
                glm::vec3 v3 = compute_point((float) x_m1, (float) y, sphere);
                glm::vec3 v4 = compute_point((float) x, (float) y, sphere);

                // Compute the two normals for the two vertices
                glm::vec3 n1 = glm::normalize(glm::cross(v4 - v1, v3 - v1));
                glm::vec3 n2 = glm::normalize(glm::cross(v4 - v1, v2 - v1));
                // And finally, compute the two colors
                glm::vec3 c1 = sphere->color * abs(glm::dot(n1, glm::vec3(0.0, 0.0, -1.0)));
                glm::vec3 c2 = sphere->color * abs(glm::dot(n2, glm::vec3(0.0, 0.0, -1.0)));

                // Store both of the faces
                faces_buffer[f_index + 2 * x].v1 = p1;
                faces_buffer[f_index + 2 * x].v2 = p3;
                faces_buffer[f_index + 2 * x].v3 = p4;
                faces_buffer[f_index + 2 * x].normal = n1;
                faces_buffer[f_index + 2 * x].color = c1;

                faces_buffer[f_index + 2 * x + 1].v1 = p1;
                faces_buffer[f_index + 2 * x + 1].v2 = p2;
                faces_buffer[f_index + 2 * x + 1].v3 = p4;
                faces_buffer[f_index + 2 * x + 1].normal = n2;
                faces_buffer[f_index + 2 * x + 1].color = c2;

                // Store the points
                vertex_buffer[p1] = glm::vec4(v1, 0.0);
                vertex_buffer[p2] = glm::vec4(v2, 0.0);
                vertex_buffer[p3] = glm::vec4(v3, 0.0);
                vertex_buffer[p4] = glm::vec4(v4, 0.0);

                // // Store them both
                // faces_buffer[f_index + 2 * x].v1 = 0;
                // faces_buffer[f_index + 2 * x].v2 = 0;
                // faces_buffer[f_index + 2 * x].v3 = 0;
                // faces_buffer[f_index + 2 * x].normal = glm::vec3(0.0, 0.0, 0.0);
                // faces_buffer[f_index + 2 * x].color = glm::vec3(0.0, 0.0, 0.0);

                // faces_buffer[f_index + 2 * x + 1].v1 = 0;
                // faces_buffer[f_index + 2 * x + 1].v2 = 0;
                // faces_buffer[f_index + 2 * x + 1].v3 = 0;
                // faces_buffer[f_index + 2 * x + 1].normal = glm::vec3(0.0, 0.0, 0.0);
                // faces_buffer[f_index + 2 * x + 1].color = glm::vec3(0.0, 0.0, 0.0);
            } else {
                // South pole

                // First, pre-compute the base index of this layer's vertices in the resulting buffer
                uint32_t f_index = max_x + 2 * (y - 2) * max_x;

                // Get the index of the polar point
                uint32_t p1 = 1 + (y - 1) * max_x;
                // Get the index of the previous point in (previous) circle
                uint32_t p2 = 1 + (y_m1 - 1) * max_x + x_m1;
                // Get the index of the current point in (previous) circle
                uint32_t p3 = 1 + (y_m1 - 1) * max_x + x;
                
                // Compute the matching points
                glm::vec3 v1 = compute_point(0, (float) y, sphere);
                glm::vec3 v2 = compute_point((float) x_m1, (float) y_m1, sphere);
                glm::vec3 v3 = compute_point((float) x, (float) y_m1, sphere);

                // Compute the normal for these fellas
                glm::vec3 n = glm::normalize(glm::cross(v3 - v1, v2 - v1));
                // Finally, compute the color
                glm::vec3 c = sphere->color * abs(glm::dot(n, glm::vec3(0.0, 0.0, -1.0)));

                // Store as vertex
                faces_buffer[f_index + x].v1 = p1;
                faces_buffer[f_index + x].v2 = p2;
                faces_buffer[f_index + x].v3 = p3;
                faces_buffer[f_index + x].normal = n;
                faces_buffer[f_index + x].color = c;

                // Store the points
                vertex_buffer[p1] = glm::vec4(v1, 0.0);
                vertex_buffer[p2] = glm::vec4(v2, 0.0);
                vertex_buffer[p3] = glm::vec4(v3, 0.0);
            }
        }
    }


    // // We implement a standard / UV sphere for simplicity
    // size_t n_vertices = sphere->n_meridians + 2 * ((sphere->n_parallels - 2) * sphere->n_meridians) + sphere->n_meridians;
    // faces.reserve(n_vertices);

    // // First, generate all vertices around the north pole
    // uint32_t p = 1;
    // for (uint32_t m = 0; m < sphere->n_meridians; m++) {
    //     // Compute the polar point
    //     glm::vec3 p1 = sphere->center + glm::vec3(0.0, sphere->radius, 0.0);
    //     // Compute the previous point in this circle
    //     glm::vec3 p2 = compute_point(m > 0 ? m - 1 : sphere->n_meridians - 1, p, sphere);
    //     // Compute the current point in this circle
    //     glm::vec3 p3 = compute_point(m, p, sphere);

    //     // Compute the normal for these fellas
    //     glm::vec3 n = glm::normalize(glm::cross(p3 - p1, p2 - p1));
    //     // And finally, compute the color
    //     glm::vec3 c = sphere->color * glm::abs(glm::dot(n, glm::vec3(0.0, 0.0, -1.0)));

    //     // Store as vertex
    //     faces.push_back({
    //         p1, p2, p3,
    //         n,
    //         c
    //     });
    //     if (m == 8) break;
    // }
    
    // // Next, generate the vertices for the circles not around the poles
    // for (p = 2; p < sphere->n_parallels; p++) {
    //     for (uint32_t m = 0; m < sphere->n_meridians; m++) {
    //         // Compute the previous point of the previous layer
    //         glm::vec3 p1 = compute_point(m > 0 ? m - 1 : sphere->n_meridians - 1, p - 1, sphere);
    //         // Compute the current point of the previous layer
    //         glm::vec3 p2 = compute_point(m, p - 1, sphere);
    //         // Compute the previous point of the current layer
    //         glm::vec3 p3 = compute_point(m > 0 ? m - 1 : sphere->n_meridians - 1, p, sphere);
    //         // Compute the current point of the current layer
    //         glm::vec3 p4 = compute_point(m, p, sphere);

    //         // Compute the two normals for the two vertices
    //         glm::vec3 n1 = glm::normalize(glm::cross(p4 - p1, p3 - p1));
    //         glm::vec3 n2 = glm::normalize(glm::cross(p4 - p1, p2 - p1));
    //         // And finally, compute the two colors
    //         glm::vec3 c1 = sphere->color * glm::abs(glm::dot(n1, glm::vec3(0.0, 0.0, -1.0)));
    //         glm::vec3 c2 = sphere->color * glm::abs(glm::dot(n2, glm::vec3(0.0, 0.0, -1.0)));

    //         // Store as vertices
    //         faces.push_back({
    //             p1, p3, p4,
    //             n1,
    //             c1
    //         });
    //         faces.push_back({
    //             p1, p2, p4,
    //             n2,
    //             c2
    //         });
    //     }
    // }

    // // Finally, generate the vertices around the south pole
    // for (uint32_t m = 0; m < sphere->n_meridians; m++) {
    //     // Compute the polar point
    //     glm::vec3 p1 = sphere->center - glm::vec3(0.0, sphere->radius, 0.0);
    //     // Compute the previous point in the (previous) circle
    //     glm::vec3 p2 = compute_point(m > 0 ? m - 1 : sphere->n_meridians - 1, p, sphere);
    //     // Compute the current point in the (previous) circle
    //     glm::vec3 p3 = compute_point(m, p, sphere);

    //     // Compute the normal for these fellas
    //     glm::vec3 n = glm::normalize(glm::cross(p3 - p1, p2 - p1));
    //     // Finally, compute the color
    //     glm::vec3 c = sphere->color * glm::abs(glm::dot(n, glm::vec3(0.0, 0.0, -1.0)));

    //     // Store as vertex
    //     faces.push_back({
    //         p1, p2, p3,
    //         n,
    //         c
    //     });
    // }

    // Done!
    DRETURN;
}

#ifdef ENABLE_VULKAN
/* Pre-renders the sphere on the GPU using Vulkan compute shaders. Uses the given GPU-allocated buffers as target buffers, so we won't have to get the stuff back to the CPU. The given offsets specify from where the pre-rendering is safe to put its results, up until that offset plus the specified size in the Sphere. */
void ECS::gpu_pre_render_sphere(const Compute::Buffer& faces_buffer, uint32_t faces_offset, const Compute::Buffer& vertex_buffer, uint32_t vertex_offset, Compute::Suite& gpu, Sphere* sphere) {
    DENTER("ECS::gpu_pre_render_sphere");
    DLOG(info, "Pre-rendering sphere with " + std::to_string(sphere->n_meridians) + " meridians and " + std::to_string(sphere->n_parallels) + " parallels...");
    DINDENT;

    /* Step 1: Prepare the staging buffer. */
    DLOG(info, "Preparing staging buffer...");

    // Allocate it only for the SphereData
    size_t gsphere_size = sizeof(SphereData);
    Buffer staging = gpu.stage_memory_pool.allocate_buffer(gsphere_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);



    /* Step 2: Prepare the SphereData on the GPU. */
    DLOG(info, "Copying sphere data to GPU...");

    // Then, allocate a uniform buffer that shall contain the sphere data
    Buffer gsphere = gpu.device_memory_pool.allocate_buffer(gsphere_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    // Next, map the staging buffer and populate it with the correct data
    SphereData* mapped_data;
    staging.map(gpu.gpu, (void**) &mapped_data);
    mapped_data->center = sphere->center;
    mapped_data->radius = sphere->radius;
    mapped_data->n_meridians = sphere->n_meridians;
    mapped_data->n_parallels = sphere->n_parallels;
    mapped_data->color = sphere->color;
    staging.flush(gpu.gpu);
    staging.unmap(gpu.gpu);

    // Copy the data over using the given transfer command buffer
    staging.copyto(gpu.staging_cb, gpu.gpu.memory_queue(), gsphere, (VkDeviceSize) gsphere_size);

    // We don't need the staging buffer anymore as we won't retrieve anything, so deallocate it
    gpu.stage_memory_pool.deallocate(staging);



    /* Step 3: Set the DescriptorSet. */
    DLOG(info, "Preparing descriptor sets...");
    
    // First, define a layout
    DescriptorSetLayout layout(gpu.gpu);
    layout.add_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT);
    layout.add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT);
    layout.add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT);
    layout.finalize();

    // Next, bind a descriptor set
    DescriptorSet descriptor_set = gpu.descriptor_pool.allocate(layout);
    descriptor_set.set(gpu.gpu, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, Tools::Array<Buffer>({ gsphere }));
    descriptor_set.set(gpu.gpu, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, Tools::Array<Buffer>({ faces_buffer }));
    descriptor_set.set(gpu.gpu, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2, Tools::Array<Buffer>({ vertex_buffer }));



    /* Step 5: Run the first shader stage */
    DLOG(info, "Running shaders...");
    DINDENT;
    {
        // Prepare the pipelines for this stage
        Pipeline pipeline_vertices(
            gpu.gpu,
            Shader(gpu.gpu, Tools::get_executable_path() + "/shaders/pre_render_sphere_v2_vertices.spv"),
            Tools::Array<DescriptorSetLayout>({ layout }),
            std::unordered_map<uint32_t, std::tuple<uint32_t, void*>>({
                { 0, std::make_tuple((uint32_t) sizeof(uint32_t), (void*) &vertex_offset) }
            })
        );
        Pipeline pipeline_faces(
            gpu.gpu,
            Shader(gpu.gpu, Tools::get_executable_path() + "/shaders/pre_render_sphere_v2_faces.spv"),
            Tools::Array<DescriptorSetLayout>({ layout }),
            std::unordered_map<uint32_t, std::tuple<uint32_t, void*>>({
                { 0, std::make_tuple((uint32_t) sizeof(uint32_t), (void*) &faces_offset) },
                { 1, std::make_tuple((uint32_t) sizeof(uint32_t), (void*) &vertex_offset) }
            })
        );

        // Next, record the command buffer
        DLOG(info, "Recording command buffer...");
        CommandBuffer cb_compute = gpu.compute_command_pool.allocate();
        cb_compute.begin();

        // Bind the vertex shader
        pipeline_vertices.bind(cb_compute);
        descriptor_set.bind(cb_compute, pipeline_vertices.layout());
        vkCmdDispatch(cb_compute, (sphere->n_meridians / 32) + 1, (sphere->n_parallels / 32) + 1, 1);
        
        // Add a barrier to prevent race conditions
        VkMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        vkCmdPipelineBarrier(cb_compute, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_DEPENDENCY_DEVICE_GROUP_BIT, 1, &barrier, 0, nullptr, 0, nullptr);

        // Bind the faces shader
        pipeline_faces.bind(cb_compute);
        descriptor_set.bind(cb_compute, pipeline_faces.layout());
        vkCmdDispatch(cb_compute, (sphere->n_meridians / 32) + 1, ((sphere->n_parallels - 1) / 32) + 1, 1);

        cb_compute.end();

        // Launch it
        DLOG(info, "Submitting command buffer...");
        VkResult vk_result;
        VkSubmitInfo submit_info = cb_compute.get_submit_info();
        if ((vk_result = vkQueueSubmit(gpu.gpu.compute_queue(), 1, &submit_info, VK_NULL_HANDLE)) != VK_SUCCESS) {
            DLOG(fatal, "Could not submit command buffer: " + vk_error_map[vk_result]);
        }

        // Finally, wait until the rendering is done as a synchronization barrier so we may re-use the command buffer
        if ((vk_result = vkQueueWaitIdle(gpu.gpu.compute_queue())) != VK_SUCCESS) {
            DLOG(fatal, "Could not wait for queue to become idle:" + vk_error_map[vk_result]);
        }

        // Deallocate the command buffer neatly
        gpu.compute_command_pool.deallocate(cb_compute);
    }
    DDEDENT;



    /* Step 7: Cleanup then done. */
    DLOG(info, "Cleaning up...");

    // Destroy the descriptor set
    gpu.descriptor_pool.deallocate(descriptor_set);

    // Destroy the sphere data as we won't need that anymore
    gpu.device_memory_pool.deallocate(gsphere);

    // Done!
    DDEDENT;
    DRETURN;
}
#endif
