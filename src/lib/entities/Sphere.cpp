/* SPHERE.cpp
 *   by Lut99
 *
 * Created:
 *   01/05/2021, 12:45:50
 * Last edited:
 *   05/05/2021, 18:43:17
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
    vertices.reserve(sphere->n_parallels * sphere->n_meridians);

    // First, define two buffers used to generate the vertices
    Tools::Array<glm::vec3> prev_circle(sphere->n_meridians);
    Tools::Array<glm::vec3> circle(sphere->n_meridians);

    // For all circles that are not poles, let's generate them
    for (uint32_t p = 1; p < sphere->n_parallels - 1; p++) {
        for (uint32_t m = 0; m < sphere->n_meridians; m++) {
            // Generate a single dot on this location
            glm::vec3 dot(
                sinf(M_PI * ((float) p / (float) sphere->n_parallels)) * cosf(2 * M_PI * ((float) m / (float) sphere->n_meridians)) * sphere->radius,
                cosf(M_PI * ((float) p / (float) sphere->n_parallels)) * sphere->radius,
                sinf(M_PI * ((float) p / (float) sphere->n_parallels)) * sinf(2 * M_PI * ((float) m / (float) sphere->n_meridians)) * sphere->radius
            );
            // Translate the dot to the correct origin point
            dot += sphere->center;

            // Add it to the current circle
            circle.push_back(dot);
        }

        // With the circle complete, generate a list of triangles as seen from the previous circle
        if (p == 1) {
            // It's the toplevel circle of vertices. Just create triangle with the top point being the top of the circle.
            for (size_t i = 0; i < sphere->n_meridians; i++) {
                // Compute the points of the vertex
                glm::vec3 p1 = sphere->center + glm::vec3({ 0.0, sphere->radius, 0.0 });
                glm::vec3 p2 = i > 0 ? circle[i - 1] : circle[sphere->n_meridians - 1];
                glm::vec3 p3 = circle[i];
                glm::vec3 normal = glm::normalize(glm::cross(p3 - p1, p2 - p1));
                glm::vec3 color = sphere->color * fabs(glm::dot(normal, glm::vec3(0.0, 0.0, -1.0)));

                // Append it to the list
                vertices.push_back({
                    p1, p2, p3,
                    normal,
                    color, 
                });
            }
        } else {
            // It's a non-toplevel circle of vertices. That means that for every pair of two points, we generate two to fill in the square
            for (size_t i = 0; i < sphere->n_meridians; i++) {
                // Compute the points of the first vertex
                glm::vec3 p1 = i > 0 ? prev_circle[i - 1] : prev_circle[sphere->n_meridians - 1];
                glm::vec3 p2 = i > 0 ? circle[i - 1] : circle[sphere->n_meridians - 1];
                glm::vec3 p3 = circle[i];
                glm::vec3 normal = glm::normalize(glm::cross(p3 - p1, p2 - p1));
                glm::vec3 color = sphere->color * fabs(glm::dot(normal, glm::vec3(0.0, 0.0, -1.0)));

                // Append it to the list
                vertices.push_back({
                    p1, p2, p3,
                    glm::normalize(glm::cross(p3 - p1, p2 - p1)),
                    color, 
                });

                // Compute the points of the second vertex
                p2 = prev_circle[i];
                normal = glm::normalize(glm::cross(p3 - p1, p2 - p1));
                color = sphere->color * fabs(glm::dot(normal, glm::vec3(0.0, 0.0, -1.0)));

                // Append it as well
                vertices.push_back({
                    p1, p2, p3,
                    glm::normalize(glm::cross(p3 - p1, p2 - p1)),
                    color, 
                });
            }
        }

        // Shuffle them around
        prev_circle = circle;
        circle.clear();
        circle.reserve(sphere->n_meridians);
    }

    // Once done, generate vertices for the soutch polar cap
    for (size_t i = 0; i < sphere->n_meridians; i++) {
        // Compute the points of the vertex
        glm::vec3 p1 = sphere->center - glm::vec3({ 0.0, sphere->radius, 0.0 });
        glm::vec3 p2 = i > 0 ? prev_circle[i - 1] : prev_circle[sphere->n_meridians - 1];
        glm::vec3 p3 = prev_circle[i];
        glm::vec3 normal = glm::normalize(glm::cross(p3 - p1, p2 - p1));
        glm::vec3 color = sphere->color * fabs(glm::dot(normal, glm::vec3(0.0, 0.0, -1.0)));

        // Append it to the list
        vertices.push_back({
            p1, p2, p3,
            normal,
            color, 
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
    size_t gpoints_size = 2 + (sphere->n_parallels - 2) * sphere->n_meridians * sizeof(glm::vec4);
    size_t gvertices_size = sphere->n_meridians + 2 * ((sphere->n_parallels - 2) * sphere->n_meridians) + sphere->n_meridians * sizeof(Vertex);

    // Allocate the buffer itself using the largest size. We don't use the point buffer as we don't touch that data from the GPU
    size_t staging_size = max({ gsphere_size, gpoints_size, gvertices_size });
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
    DLOG(info, "Preparing output buffers...");
    
    // Allocate the intermediate point buffer first
    BufferHandle gpoints_h = device_memory_pool.allocate(gpoints_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    Buffer gpoints = device_memory_pool[gpoints_h];

    // Next is the vertex buffer
    BufferHandle gvertices_h = device_memory_pool.allocate(gvertices_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    Buffer gvertices = device_memory_pool[gvertices_h];



    /* Step 4: Set the DescriptorSet. */
    DLOG(info, "Preparing descriptor sets...");
    
    // First, define a layout
    DescriptorSetLayout layout(gpu);
    layout.add_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT);
    layout.add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT);
    layout.add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT);
    layout.finalize();

    // Next, bind a descriptor set
    DescriptorSet descriptor_set = descriptor_pool.allocate(layout);
    descriptor_set.set(gpu, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, Tools::Array<Buffer>({ gsphere }));
    descriptor_set.set(gpu, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, Tools::Array<Buffer>({ gpoints }));
    descriptor_set.set(gpu, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2, Tools::Array<Buffer>({ gvertices }));



    /* Step 5: Run the first shader stage */
    DLOG(info, "Running first shader stage (generating points)...");
    DINDENT;
    Shader shader(gpu, Tools::get_executable_path() + "/shaders/pre_render_sphere_v1.spv");
    {
        // Prepare the pipeline for this stage
        uint32_t stage = 0;
        Pipeline pipeline(
            gpu,
            shader,
            Tools::Array<DescriptorSetLayout>({ layout }),
            std::unordered_map<uint32_t, std::tuple<uint32_t, void*>>({
                { 0, std::make_tuple(sizeof(uint32_t), (void*) &stage) }
            })
        );

        // Next, record the command buffer
        CommandBufferHandle cb_compute_h = compute_command_pool.allocate();
        CommandBuffer cb_compute = compute_command_pool[cb_compute_h];
        cb_compute.begin();
        pipeline.bind(cb_compute);
        descriptor_set.bind(cb_compute, pipeline.layout());
        vkCmdDispatch(cb_compute, (sphere->n_meridians / 32) + 1, (sphere->n_parallels / 32) + 1, 1);
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



    /* DEBUG step: download the points and compare them to those generate by the CPU-counterpart. */
    DLOG(warning, "Running debug step: point validity");
    Tools::Array<glm::vec4> gpu_points(2 + (sphere->n_parallels - 2) * sphere->n_meridians);
    gpoints.get(gpu, staging, staging_cb, gpu.memory_queue(), gpu_points.wdata(2 + (sphere->n_parallels - 2) * sphere->n_meridians), gpoints_size);
    Tools::Array<glm::vec4> cpu_points(2 + (sphere->n_parallels - 2) * sphere->n_meridians);
    // For all circles that are not poles, let's generate them
    cpu_points.push_back(glm::vec4(sphere->center + glm::vec3(0.0, sphere->radius, 0.0), 0.0));
    for (uint32_t p = 1; p < sphere->n_parallels - 1; p++) {
        for (uint32_t m = 0; m < sphere->n_meridians; m++) {
            // Generate a single dot on this location
            glm::vec3 dot(
                sinf(M_PI * ((float) p / (float) sphere->n_parallels)) * cosf(2 * M_PI * ((float) m / (float) sphere->n_meridians)) * sphere->radius,
                cosf(M_PI * ((float) p / (float) sphere->n_parallels)) * sphere->radius,
                sinf(M_PI * ((float) p / (float) sphere->n_parallels)) * sinf(2 * M_PI * ((float) m / (float) sphere->n_meridians)) * sphere->radius
            );
            // Translate the dot to the correct origin point
            dot += sphere->center;

            // Add it to the current circle
            cpu_points.push_back(glm::vec4(dot, p));
        }
    }
    cpu_points.push_back(glm::vec4(sphere->center - glm::vec3(0.0, sphere->radius, 0.0), sphere->n_parallels - 1));
    if (cpu_points.size() != gpu_points.size()) {
        printf("ERROR: CPU & GPU lists do not have the same size: CPU = %lu, GPU = %lu\n", cpu_points.size(), gpu_points.size());
    }
    float min_d = 1e-6;
    for (uint32_t i = 0; i < min({ cpu_points.size(), gpu_points.size() }); i++) {
        glm::vec4 distance = glm::abs(cpu_points[i] - gpu_points[i]);
        if (cpu_points[i].w < 2 || gpu_points[i].w < 2) {//distance.x > min_d || distance.y > min_d || distance.z > min_d || distance.w > min_d) {
            printf("ERROR: CPU & GPU lists differ @ %u: CPU = (%f,%f,%f,%f), GPU = (%f,%f,%f,%f) [y = %f]\n",
                i,
                cpu_points[i].x, cpu_points[i].y, cpu_points[i].z, cpu_points[i].w,
                gpu_points[i].x, gpu_points[i].y, gpu_points[i].z, gpu_points[i].w,
                gpu_points[i].w);
        }
    }
    


    /* Step 6: Run the second shader stage */
    DLOG(info, "Running second shader stage (generating vertices)...");
    DINDENT;
    {
        // Prepare the pipeline for this stage
        uint32_t stage = 1;
        Pipeline pipeline(
            gpu,
            shader,
            Tools::Array<DescriptorSetLayout>({ layout }),
            std::unordered_map<uint32_t, std::tuple<uint32_t, void*>>({
                { 0, std::make_tuple(sizeof(uint32_t), (void*) &stage) }
            })
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
            DLOG(fatal, "Could not submit command buffer for second shader stage: " + vk_error_map[vk_result]);
        }

        // Finally, wait until the rendering is done as a synchronization barrier so we may re-use the command buffer
        if ((vk_result = vkQueueWaitIdle(gpu.compute_queue())) != VK_SUCCESS) {
            DLOG(fatal, "Could not wait for queue to become idle after submitting second shader stage: " + vk_error_map[vk_result]);
        }

        // Deallocate the command buffer neatly
        compute_command_pool.deallocate(cb_compute_h);
    }
    DDEDENT;



    /* Step 7: Retrieving vertices. */
    DLOG(info, "Retrieving vertices...");

    // Use the get method together with the staging buffer and the resulting lists' wdata
    size_t n_vertices = sphere->n_meridians + 2 * ((sphere->n_parallels - 2) * sphere->n_meridians) + sphere->n_meridians;
    vertices.reserve(n_vertices);
    gvertices.get(gpu, staging, staging_cb, gpu.memory_queue(), vertices.wdata(n_vertices), gvertices_size);
    printf("Color of first vertex: %f, %f, %f\n", vertices[0].color.r, vertices[0].color.g, vertices[0].color.b);



    /* Step 8: Cleanup then done. */
    DLOG(info, "Cleaning up...");

    // Destroy the descriptor set
    descriptor_pool.deallocate(descriptor_set);

    // Destroy the three main buffers
    device_memory_pool.deallocate(gvertices_h);
    device_memory_pool.deallocate(gpoints_h);
    device_memory_pool.deallocate(gsphere_h);

    // Finally, destroy the staging buffer
    stage_memory_pool.deallocate(staging_h);

    // Done!
    DDEDENT;
    DRETURN;
}
#endif
