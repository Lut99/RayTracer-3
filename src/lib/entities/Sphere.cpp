/* SPHERE.cpp
 *   by Lut99
 *
 * Created:
 *   01/05/2021, 12:45:50
 * Last edited:
 *   06/05/2021, 16:48:04
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Contains code for pre-rendering a Sphere on the CPU (single threaded).
**/

#define ENABLE_VULKAN

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
void ECS::cpu_pre_render_sphere(Tools::Array<Vertex>& vertices, Sphere* sphere) {
    DENTER("ECS::cpu_pre_render_sphere");
    DLOG(info, "Pre-rendering sphere with " + std::to_string(sphere->n_meridians) + " meridians and " + std::to_string(sphere->n_parallels) + " parallels...");

    // We implement a standard / UV sphere for simplicity
    size_t n_vertices = sphere->n_meridians + 2 * ((sphere->n_parallels - 2) * sphere->n_meridians) + sphere->n_meridians;
    vertices.reserve(n_vertices);

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
        vertices.push_back({
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
            vertices.push_back({
                p1, p3, p4,
                n1,
                c1
            });
            vertices.push_back({
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
        vertices.push_back({
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
void ECS::gpu_pre_render_sphere(
    Tools::Array<Vertex>& vertices,
    const Compute::GPU& gpu,
    Compute::MemoryPool& device_memory_pool,
    Compute::MemoryPool& stage_memory_pool,
    Compute::DescriptorPool& descriptor_pool,
    Compute::CommandPool& compute_command_pool,
    const Compute::CommandBuffer& staging_cb,
    Sphere* sphere
) {
    DENTER("ECS::gpu_pre_render_sphere");
    DLOG(info, "Pre-rendering sphere with " + std::to_string(sphere->n_meridians) + " meridians and " + std::to_string(sphere->n_parallels) + " parallels...");
    DINDENT;

    /* Step 1: Prepare the staging buffer. */
    DLOG(info, "Preparing staging buffer...");

    // First, compute the sizes for all buffers
    size_t gsphere_size = sizeof(SphereData);
    size_t gvertices_count = (sphere->n_meridians + 2 * ((sphere->n_parallels - 2) * sphere->n_meridians) + sphere->n_meridians);
    size_t gvertices_size = gvertices_count * sizeof(Vertex);

    // Allocate the buffer itself using the largest size. We don't use the point buffer as we don't touch that data from the GPU
    size_t staging_size = max({ gsphere_size, gvertices_size });
    BufferHandle staging_h = stage_memory_pool.allocate(staging_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    Buffer staging = stage_memory_pool[staging_h];

    DINDENT;
    DLOG(info, "Staging buffer is " + std::to_string(staging_size) + " bytes.");
    DDEDENT;



    /* Step 2: Prepare the SphereData on the GPU. */
    DLOG(info, "Copying sphere data to GPU...");

    // Then, allocate a uniform buffer that shall contain the sphere data
    BufferHandle gsphere_h = device_memory_pool.allocate(gsphere_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    Buffer gsphere = device_memory_pool[gsphere_h];

    // Next, map the staging buffer and populate it with the correct data
    SphereData* mapped_data;
    staging.map(gpu, (void**) &mapped_data);
    mapped_data->center = sphere->center;
    mapped_data->radius = sphere->radius;
    mapped_data->n_meridians = sphere->n_meridians;
    mapped_data->n_parallels = sphere->n_parallels;
    mapped_data->color = sphere->color;
    staging.flush(gpu);
    staging.unmap(gpu);

    // Copy the data over using the given transfer command buffer
    staging.copyto(staging_cb, gpu.memory_queue(), gsphere, (VkDeviceSize) gsphere_size);

    

    /* Step 3: Prepare the output buffers. */
    DLOG(info, "Preparing output buffer...");

    // Next is the vertex buffer
    BufferHandle gvertices_h = device_memory_pool.allocate(gvertices_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    Buffer gvertices = device_memory_pool[gvertices_h];
    DINDENT;
    DLOG(info, "Vertex buffer is " + std::to_string(gvertices_size / sizeof(Vertex)) + " elements (" + std::to_string(gvertices_size) + " bytes)");
    DDEDENT;



    /* Step 4: Set the DescriptorSet. */
    DLOG(info, "Preparing descriptor sets...");
    
    // First, define a layout
    DescriptorSetLayout layout(gpu);
    layout.add_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT);
    layout.add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT);
    layout.finalize();

    // Next, bind a descriptor set
    DescriptorSet descriptor_set = descriptor_pool.allocate(layout);
    descriptor_set.set(gpu, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, Tools::Array<Buffer>({ gsphere }));
    descriptor_set.set(gpu, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, Tools::Array<Buffer>({ gvertices }));



    /* Step 5: Run the first shader stage */
    DLOG(info, "Running shader...");
    DINDENT;
    {
        // Prepare the pipeline for this stage
        Pipeline pipeline(
            gpu,
            Shader(gpu, Tools::get_executable_path() + "/shaders/pre_render_sphere_v1.spv"),
            Tools::Array<DescriptorSetLayout>({ layout })
        );

        // Next, record the command buffer
        CommandBufferHandle cb_compute_h = compute_command_pool.allocate();
        CommandBuffer cb_compute = compute_command_pool[cb_compute_h];
        cb_compute.begin();
        pipeline.bind(cb_compute);
        descriptor_set.bind(cb_compute, pipeline.layout());
        vkCmdDispatch(cb_compute, (sphere->n_meridians / 32) + 1, ((sphere->n_parallels - 1) / 32) + 1, 1);
        cb_compute.end();

        // Launch it
        VkResult vk_result;
        VkSubmitInfo submit_info = cb_compute.get_submit_info();
        if ((vk_result = vkQueueSubmit(gpu.compute_queue(), 1, &submit_info, VK_NULL_HANDLE)) != VK_SUCCESS) {
            DLOG(fatal, "Could not submit command buffer for first shader stage: " + vk_error_map[vk_result]);
        }

        // Finally, wait until the rendering is done as a synchronization barrier so we may re-use the command buffer
        if ((vk_result = vkQueueWaitIdle(gpu.compute_queue())) != VK_SUCCESS) {
            DLOG(fatal, "Could not wait for queue to become idle after submitting first shader stage:" + vk_error_map[vk_result]);
        }

        // Deallocate the command buffer neatly
        compute_command_pool.deallocate(cb_compute_h);
    }
    DDEDENT;



    /* Step 7: Retrieving vertices. */
    DLOG(info, "Retrieving vertices...");

    // Use the get method together with the staging buffer and the resulting lists' wdata
    vertices.reserve(gvertices_count);
    gvertices.get(gpu, staging, staging_cb, gpu.memory_queue(), vertices.wdata(gvertices_count), gvertices_size);
    // for (size_t i = 0; i < sphere->n_meridians; i++) {
    //     // Compute the point for funz
    //     // Generate the north pole
    //     glm::vec3 north = sphere->center + glm::vec3(0.0, sphere->radius, 0.0);

    //     // Generate a single dot on the previous location
    //     glm::vec3 p1(
    //         sinf(M_PI * ((float) 1 / (float) sphere->n_parallels)) * cosf(2 * M_PI * ((float) (i > 0 ? i - 1 : sphere->n_meridians - 1) / (float) sphere->n_meridians)) * sphere->radius,
    //         cosf(M_PI * ((float) 1 / (float) sphere->n_parallels)) * sphere->radius,
    //         sinf(M_PI * ((float) 1 / (float) sphere->n_parallels)) * sinf(2 * M_PI * ((float) (i > 0 ? i - 1 : sphere->n_meridians - 1) / (float) sphere->n_meridians)) * sphere->radius
    //     );
    //     // Translate the dot to the correct origin point
    //     p1 += sphere->center;

    //     // Generate a single dot on this location
    //     glm::vec3 p2(
    //         sinf(M_PI * ((float) 1 / (float) sphere->n_parallels)) * cosf(2 * M_PI * ((float) i / (float) sphere->n_meridians)) * sphere->radius,
    //         cosf(M_PI * ((float) 1 / (float) sphere->n_parallels)) * sphere->radius,
    //         sinf(M_PI * ((float) 1 / (float) sphere->n_parallels)) * sinf(2 * M_PI * ((float) i / (float) sphere->n_meridians)) * sphere->radius
    //     );
    //     // Translate the dot to the correct origin point
    //     p2 += sphere->center;

    //     glm::vec3 distance1 = glm::abs(vertices[i].p1 - north);
    //     glm::vec3 distance2 = glm::abs(vertices[i].p2 - p1);
    //     glm::vec3 distance3 = glm::abs(vertices[i].p3 - p2);
    //     if (distance1.x > 1e-6 || distance1.y > 1e-6 || distance1.z > 1e-6 ||
    //         distance2.x > 1e-6 || distance2.y > 1e-6 || distance2.z > 1e-6 ||
    //         distance3.x > 1e-6 || distance3.y > 1e-6 || distance3.z > 1e-6)
    //     {
    //         printf("Vertex %lu (north) differs:\n   Based on: (%f,%f,%f), (%f,%f,%f) and (%f,%f,%f)\n   Points: (%f,%f,%f), (%f,%f,%f) and (%f,%f,%f)\n   Normal: (%f,%f,%f)\n   Color: (%f,%f,%f)\n\n", i,
    //             north.x, north.y, north.z,
    //             p1.x, p1.y, p1.z,
    //             p2.x, p2.y, p2.z,
    //             vertices[i].p1.x, vertices[i].p1.y, vertices[i].p1.z,
    //             vertices[i].p2.x, vertices[i].p2.y, vertices[i].p2.z,
    //             vertices[i].p3.x, vertices[i].p3.y, vertices[i].p3.z,
    //             vertices[i].normal.x, vertices[i].normal.y, vertices[i].normal.z,
    //             vertices[i].color.x, vertices[i].color.y, vertices[i].color.z);
    //     }
    // }
    // for (size_t p = 2; p < sphere->n_parallels - 1; p++) {
    //     for (size_t i = 0; i < sphere->n_meridians; i++) {
    //         // Compute the point for funz

    //         // Generate a single dot on the previous location on the previous circle
    //         glm::vec3 p1(
    //             sinf(M_PI * ((float) (p - 1) / (float) sphere->n_parallels)) * cosf(2 * M_PI * ((float) (i > 0 ? i - 1 : sphere->n_meridians - 1) / (float) sphere->n_meridians)) * sphere->radius,
    //             cosf(M_PI * ((float) (p - 1) / (float) sphere->n_parallels)) * sphere->radius,
    //             sinf(M_PI * ((float) (p - 1) / (float) sphere->n_parallels)) * sinf(2 * M_PI * ((float) (i > 0 ? i - 1 : sphere->n_meridians - 1) / (float) sphere->n_meridians)) * sphere->radius
    //         );
    //         // Translate the dot to the correct origin point
    //         p1 += sphere->center;

    //         // Generate a single dot on this location on the previous circle
    //         glm::vec3 p2(
    //             sinf(M_PI * ((float) (p - 1) / (float) sphere->n_parallels)) * cosf(2 * M_PI * ((float) i / (float) sphere->n_meridians)) * sphere->radius,
    //             cosf(M_PI * ((float) (p - 1) / (float) sphere->n_parallels)) * sphere->radius,
    //             sinf(M_PI * ((float) (p - 1) / (float) sphere->n_parallels)) * sinf(2 * M_PI * ((float) i / (float) sphere->n_meridians)) * sphere->radius
    //         );
    //         // Translate the dot to the correct origin point
    //         p2 += sphere->center;

    //         // Generate a single dot on the previous location
    //         glm::vec3 p3(
    //             sinf(M_PI * ((float) p / (float) sphere->n_parallels)) * cosf(2 * M_PI * ((float) (i > 0 ? i - 1 : sphere->n_meridians - 1) / (float) sphere->n_meridians)) * sphere->radius,
    //             cosf(M_PI * ((float) p / (float) sphere->n_parallels)) * sphere->radius,
    //             sinf(M_PI * ((float) p / (float) sphere->n_parallels)) * sinf(2 * M_PI * ((float) (i > 0 ? i - 1 : sphere->n_meridians - 1) / (float) sphere->n_meridians)) * sphere->radius
    //         );
    //         // Translate the dot to the correct origin point
    //         p3 += sphere->center;

    //         // Generate a single dot on this location
    //         glm::vec3 p4(
    //             sinf(M_PI * ((float) p / (float) sphere->n_parallels)) * cosf(2 * M_PI * ((float) i / (float) sphere->n_meridians)) * sphere->radius,
    //             cosf(M_PI * ((float) p / (float) sphere->n_parallels)) * sphere->radius,
    //             sinf(M_PI * ((float) p / (float) sphere->n_parallels)) * sinf(2 * M_PI * ((float) i / (float) sphere->n_meridians)) * sphere->radius
    //         );
    //         // Translate the dot to the correct origin point
    //         p4 += sphere->center;

    //         size_t index = sphere->n_meridians + 2 * (p - 2) * sphere->n_meridians + 2 * i;
    //         glm::vec3 distance1 = glm::abs(vertices[index].p1 - p1);
    //         glm::vec3 distance2 = glm::abs(vertices[index].p2 - p3);
    //         glm::vec3 distance3 = glm::abs(vertices[index].p3 - p4);
    //         if (
    //             (distance1.x > 1e-6 || distance1.y > 1e-6 || distance1.z > 1e-6 ||
    //              distance2.x > 1e-6 || distance2.y > 1e-6 || distance2.z > 1e-6 ||
    //              distance3.x > 1e-6 || distance3.y > 1e-6 || distance3.z > 1e-6))
    //         {
    //             printf("Vertex %lu,%lu (1) differs:\n   Based on: (%f,%f,%f), (%f,%f,%f) and (%f,%f,%f)\n   Points: (%f,%f,%f), (%f,%f,%f) and (%f,%f,%f)\n   Normal: (%f,%f,%f)\n   Color: (%f,%f,%f)\n\n", p, i,
    //                 p1.x, p1.y, p1.z,
    //                 p3.x, p3.y, p3.z,
    //                 p4.x, p4.y, p4.z,
    //                 vertices[index].p1.x, vertices[index].p1.y, vertices[index].p1.z,
    //                 vertices[index].p2.x, vertices[index].p2.y, vertices[index].p2.z,
    //                 vertices[index].p3.x, vertices[index].p3.y, vertices[index].p3.z,
    //                 vertices[index].normal.x, vertices[index].normal.y, vertices[index].normal.z,
    //                 vertices[index].color.x, vertices[index].color.y, vertices[index].color.z);
    //         }

    //         index = sphere->n_meridians + 2 * (p - 2) * sphere->n_meridians + 2 * i + 1;
    //         distance1 = glm::abs(vertices[index].p1 - p1);
    //         distance2 = glm::abs(vertices[index].p2 - p2);
    //         distance3 = glm::abs(vertices[index].p3 - p4);
    //         if (
    //             (distance1.x > 1e-6 || distance1.y > 1e-6 || distance1.z > 1e-6 ||
    //              distance2.x > 1e-6 || distance2.y > 1e-6 || distance2.z > 1e-6 ||
    //              distance3.x > 1e-6 || distance3.y > 1e-6 || distance3.z > 1e-6))
    //         {
    //             printf("Vertex %lu,%lu (2) differs:\n   Based on: (%f,%f,%f), (%f,%f,%f) and (%f,%f,%f)\n   Points: (%f,%f,%f), (%f,%f,%f) and (%f,%f,%f)\n   Normal: (%f,%f,%f)\n   Color: (%f,%f,%f)\n\n", p, i,
    //                 p1.x, p1.y, p1.z,
    //                 p2.x, p2.y, p2.z,
    //                 p4.x, p4.y, p4.z,
    //                 vertices[index].p1.x, vertices[index].p1.y, vertices[index].p1.z,
    //                 vertices[index].p2.x, vertices[index].p2.y, vertices[index].p2.z,
    //                 vertices[index].p3.x, vertices[index].p3.y, vertices[index].p3.z,
    //                 vertices[index].normal.x, vertices[index].normal.y, vertices[index].normal.z,
    //                 vertices[index].color.x, vertices[index].color.y, vertices[index].color.z);
    //         }
    //     }
    // }
    // for (size_t i = 0; i < sphere->n_meridians; i++) {
    //     // Compute the point for funz
    //     // Generate the north pole
    //     glm::vec3 south = sphere->center - glm::vec3(0.0, sphere->radius, 0.0);

    //     // Generate a single dot on the previous location
    //     glm::vec3 p1(
    //         sinf(M_PI * ((float) (sphere->n_parallels - 2) / (float) sphere->n_parallels)) * cosf(2 * M_PI * ((float) (i > 0 ? i - 1 : sphere->n_meridians - 1) / (float) sphere->n_meridians)) * sphere->radius,
    //         cosf(M_PI * ((float) (sphere->n_parallels - 2) / (float) sphere->n_parallels)) * sphere->radius,
    //         sinf(M_PI * ((float) (sphere->n_parallels - 2) / (float) sphere->n_parallels)) * sinf(2 * M_PI * ((float) (i > 0 ? i - 1 : sphere->n_meridians - 1) / (float) sphere->n_meridians)) * sphere->radius
    //     );
    //     // Translate the dot to the correct origin point
    //     p1 += sphere->center;

    //     // Generate a single dot on this location
    //     glm::vec3 p2(
    //         sinf(M_PI * ((float) (sphere->n_parallels - 2) / (float) sphere->n_parallels)) * cosf(2 * M_PI * ((float) i / (float) sphere->n_meridians)) * sphere->radius,
    //         cosf(M_PI * ((float) (sphere->n_parallels - 2) / (float) sphere->n_parallels)) * sphere->radius,
    //         sinf(M_PI * ((float) (sphere->n_parallels - 2) / (float) sphere->n_parallels)) * sinf(2 * M_PI * ((float) i / (float) sphere->n_meridians)) * sphere->radius
    //     );
    //     // Translate the dot to the correct origin point
    //     p2 += sphere->center;

    //     size_t index = sphere->n_meridians + 2 * (sphere->n_parallels - 3) * sphere->n_meridians + i;
    //     glm::vec3 distance1 = glm::abs(vertices[index].p1 - south);
    //     glm::vec3 distance2 = glm::abs(vertices[index].p2 - p1);
    //     glm::vec3 distance3 = glm::abs(vertices[index].p3 - p2);
    //     if (distance1.x > 1e-6 || distance1.y > 1e-6 || distance1.z > 1e-6 ||
    //         distance2.x > 1e-6 || distance2.y > 1e-6 || distance2.z > 1e-6 ||
    //         distance3.x > 1e-6 || distance3.y > 1e-6 || distance3.z > 1e-6)
    //     {
    //         printf("Vertex %lu (south) differs:\n   Based on: (%f,%f,%f), (%f,%f,%f) and (%f,%f,%f)\n   Points: (%f,%f,%f), (%f,%f,%f) and (%f,%f,%f)\n   Normal: (%f,%f,%f)\n   Color: (%f,%f,%f)\n\n", i,
    //             south.x, south.y, south.z,
    //             p1.x, p1.y, p1.z,
    //             p2.x, p2.y, p2.z,
    //             vertices[index].p1.x, vertices[index].p1.y, vertices[index].p1.z,
    //             vertices[index].p2.x, vertices[index].p2.y, vertices[index].p2.z,
    //             vertices[index].p3.x, vertices[index].p3.y, vertices[index].p3.z,
    //             vertices[index].normal.x, vertices[index].normal.y, vertices[index].normal.z,
    //             vertices[index].color.x, vertices[index].color.y, vertices[index].color.z);
    //     }
    // }



    /* Step 8: Cleanup then done. */
    DLOG(info, "Cleaning up...");

    // Destroy the descriptor set
    descriptor_pool.deallocate(descriptor_set);

    // Destroy the three main buffers
    device_memory_pool.deallocate(gvertices_h);
    device_memory_pool.deallocate(gsphere_h);

    // Finally, destroy the staging buffer
    stage_memory_pool.deallocate(staging_h);

    // Done!
    DDEDENT;
    DRETURN;
}
#endif
