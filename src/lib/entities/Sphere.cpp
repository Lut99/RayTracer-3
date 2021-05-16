/* SPHERE.cpp
 *   by Lut99
 *
 * Created:
 *   01/05/2021, 12:45:50
 * Last edited:
 *   16/05/2021, 12:46:34
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Contains code for pre-rendering a Sphere on the CPU (single threaded).
**/

// #define ENABLE_VULKAN

#include <cmath>
#include <CppDebugger.hpp>

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
    alignas(4)  uint n_meridians;
    /* The number of parallels in the sphere (i.e., horizontal lines). */
    alignas(4)  uint n_parallels;
    /* The color of the sphere. */
    alignas(16) glm::vec3 color;
};
#endif





/***** HELPER FUNCTIONS *****/
/* Computes the coordinates of a single point on the sphere. */
static glm::vec3 compute_point(float fx, float fy, Sphere* sphere) {
    DENTER("ECS::compute_point");

    glm::vec3 result = sphere->center + sphere->radius * glm::vec3(
        sin(M_PI * (fy / (float) sphere->n_parallels)) * cos(2 * M_PI * (fx / (float) sphere->n_meridians)),
        cos(M_PI * (fy / (float) sphere->n_parallels)),
        sin(M_PI * (fy / (float) sphere->n_parallels)) * sin(2 * M_PI * (fx / (float) sphere->n_meridians))
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



/* Pre-renders the sphere on the CPU, single-threaded. */
void ECS::cpu_pre_render_sphere(Tools::Array<Face>& faces, Sphere* sphere) {
    DENTER("ECS::cpu_pre_render_sphere");
    DLOG(info, "Pre-rendering sphere with " + std::to_string(sphere->n_meridians) + " meridians and " + std::to_string(sphere->n_parallels) + " parallels...");

    // We implement a standard / UV sphere for simplicity
    size_t n_vertices = sphere->n_meridians + 2 * ((sphere->n_parallels - 2) * sphere->n_meridians) + sphere->n_meridians;
    faces.reserve(n_vertices);

    // First, generate all vertices around the north pole
    uint32_t p = 1;
    for (uint32_t m = 0; m < sphere->n_meridians; m++) {
        // Compute the polar point
        glm::vec3 p1 = sphere->center + glm::vec3(0.0, sphere->radius, 0.0);
        // Compute the previous point in this circle
        glm::vec3 p2 = compute_point(m > 0 ? m - 1 : sphere->n_meridians - 1, p, sphere);
        // Compute the current point in this circle
        glm::vec3 p3 = compute_point(m, p, sphere);

        // Compute the normal for these fellas
        glm::vec3 n = glm::normalize(glm::cross(p3 - p1, p2 - p1));
        // And finally, compute the color
        glm::vec3 c = sphere->color * glm::abs(glm::dot(n, glm::vec3(0.0, 0.0, -1.0)));

        // Store as vertex
        faces.push_back({
            p1, p2, p3,
            n,
            c
        });
        if (m == 8) break;
    }
    
    // Next, generate the vertices for the circles not around the poles
    for (p = 2; p < sphere->n_parallels; p++) {
        for (uint32_t m = 0; m < sphere->n_meridians; m++) {
            // Compute the previous point of the previous layer
            glm::vec3 p1 = compute_point(m > 0 ? m - 1 : sphere->n_meridians - 1, p - 1, sphere);
            // Compute the current point of the previous layer
            glm::vec3 p2 = compute_point(m, p - 1, sphere);
            // Compute the previous point of the current layer
            glm::vec3 p3 = compute_point(m > 0 ? m - 1 : sphere->n_meridians - 1, p, sphere);
            // Compute the current point of the current layer
            glm::vec3 p4 = compute_point(m, p, sphere);

            // Compute the two normals for the two vertices
            glm::vec3 n1 = glm::normalize(glm::cross(p4 - p1, p3 - p1));
            glm::vec3 n2 = glm::normalize(glm::cross(p4 - p1, p2 - p1));
            // And finally, compute the two colors
            glm::vec3 c1 = sphere->color * glm::abs(glm::dot(n1, glm::vec3(0.0, 0.0, -1.0)));
            glm::vec3 c2 = sphere->color * glm::abs(glm::dot(n2, glm::vec3(0.0, 0.0, -1.0)));

            // Store as vertices
            faces.push_back({
                p1, p3, p4,
                n1,
                c1
            });
            faces.push_back({
                p1, p2, p4,
                n2,
                c2
            });
        }
    }

    // Finally, generate the vertices around the south pole
    for (uint32_t m = 0; m < sphere->n_meridians; m++) {
        // Compute the polar point
        glm::vec3 p1 = sphere->center - glm::vec3(0.0, sphere->radius, 0.0);
        // Compute the previous point in the (previous) circle
        glm::vec3 p2 = compute_point(m > 0 ? m - 1 : sphere->n_meridians - 1, p, sphere);
        // Compute the current point in the (previous) circle
        glm::vec3 p3 = compute_point(m, p, sphere);

        // Compute the normal for these fellas
        glm::vec3 n = glm::normalize(glm::cross(p3 - p1, p2 - p1));
        // Finally, compute the color
        glm::vec3 c = sphere->color * glm::abs(glm::dot(n, glm::vec3(0.0, 0.0, -1.0)));

        // Store as vertex
        faces.push_back({
            p1, p2, p3,
            n,
            c
        });
    }

    // Done!
    DRETURN;
}

#ifdef ENABLE_VULKAN
/* Pre-renders the sphere on the GPU using Vulkan compute shaders. */
void ECS::gpu_pre_render_sphere(Compute::BufferHandle& faces_buffer, Compute::BufferHandle& vertex_buffer, Compute::Suite& gpu, Sphere* sphere) {
    DENTER("ECS::gpu_pre_render_sphere");
    DLOG(info, "Pre-rendering sphere with " + std::to_string(sphere->n_meridians) + " meridians and " + std::to_string(sphere->n_parallels) + " parallels...");
    DINDENT;

    /* Step 1: Prepare the staging buffer. */
    DLOG(info, "Preparing staging buffer...");

    // Allocate it only for the SphereData
    size_t gsphere_size = sizeof(SphereData);
    Buffer staging = gpu.stage_memory_pool.allocate_buffer(gsphere_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);



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

    

    /* Step 3: Prepare the output buffers. */
    DLOG(info, "Preparing output buffers...");
    DINDENT;

    // Next, we allocate the faces & vertex buffers
    size_t gfaces_count = sphere->n_meridians + 2 * ((sphere->n_parallels - 2) * sphere->n_meridians) + sphere->n_meridians;
    size_t gfaces_size = gfaces_count * sizeof(GFace);
    Buffer gfaces = gpu.device_memory_pool.allocate_buffer(gfaces_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    faces_buffer = gfaces.handle();
    DLOG(info, "Face buffer is " + std::to_string(gfaces_size / sizeof(GFace)) + " elements (" + std::to_string(gfaces_size) + " bytes)");

    size_t gvertices_count = 2 + (sphere->n_parallels - 2) * sphere->n_meridians;
    size_t gvertices_size = gvertices_count * sizeof(glm::vec4);
    Buffer gvertices = gpu.device_memory_pool.allocate_buffer(gvertices_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    vertex_buffer = gvertices.handle();
    DLOG(info, "Vertex buffer is " + std::to_string(gvertices_size / sizeof(glm::vec4)) + "elements (" + std::to_string(gvertices_size) + " bytes)");
    
    DDEDENT;



    /* Step 4: Set the DescriptorSet. */
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
    descriptor_set.set(gpu.gpu, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, Tools::Array<Buffer>({ gfaces }));
    descriptor_set.set(gpu.gpu, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2, Tools::Array<Buffer>({ gvertices }));



    /* Step 5: Run the first shader stage */
    DLOG(info, "Running shaders...");
    DINDENT;
    {
        // Prepare the pipelines for this stage
        Pipeline pipeline_vertices(
            gpu.gpu,
            Shader(gpu.gpu, Tools::get_executable_path() + "/shaders/pre_render_sphere_v2_vertices.spv"),
            Tools::Array<DescriptorSetLayout>({ layout })
        );
        Pipeline pipeline_faces(
            gpu.gpu,
            Shader(gpu.gpu, Tools::get_executable_path() + "/shaders/pre_render_sphere_v2_faces.spv"),
            Tools::Array<DescriptorSetLayout>({ layout })
        );

        // Next, record the command buffer
        CommandBuffer cb_compute = gpu.compute_command_pool.allocate();
        cb_compute.begin();

        // Bind the vertex shader
        pipeline_vertices.bind(cb_compute);
        descriptor_set.bind(cb_compute, pipeline_vertices.layout());
        vkCmdDispatch(cb_compute, (sphere->n_meridians / 32) + 1, (sphere->n_parallels / 32) + 1, 1);

        // Bind the faces shader
        pipeline_faces.bind(cb_compute);
        descriptor_set.bind(cb_compute, pipeline_faces.layout());
        vkCmdDispatch(cb_compute, (sphere->n_meridians / 32) + 1, ((sphere->n_parallels - 1) / 32) + 1, 1);

        cb_compute.end();

        // Launch it
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



/* Returns the number of faces & vertices for this sphere, appended to the given integers. */
void ECS::get_size_sphere(uint32_t& n_faces, uint32_t& n_vertices, Sphere* sphere) {
    DENTER("ECS::get_size_sphere");

    n_faces += sphere->n_meridians + 2 * ((sphere->n_parallels - 2) * sphere->n_meridians) + sphere->n_meridians;
    n_vertices += 2 + (sphere->n_parallels - 2) * sphere->n_meridians;

    DRETURN;
}
