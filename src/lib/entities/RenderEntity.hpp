/* RENDER ENTITY.hpp
 *   by Lut99
 *
 * Created:
 *   30/04/2021, 13:08:59
 * Last edited:
 *   19/05/2021, 16:46:14
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Contains the RenderEntity struct, used in our own entity component system
 *   impementation. It forms the basis of all objects that shall be rendered 
 *   by the RayTracer.
**/

#ifndef ENTITIES_RENDER_ENTITY_HPP
#define ENTITIES_RENDER_ENTITY_HPP

#include <string>

namespace RayTracer::ECS {
    /* List of all entities registered to the RayTracer. */
    enum EntityType {
        /* A type indicating that it's not set yet. */
        et_none = 0,
        /* Basic, 2D-triangle shape. */
        et_triangle = 1,
        /* Basic sphere shape. */
        et_sphere = 2,
        /* Pre-rendered object from a file. */
        et_object= 3
    };
    /* Maps an entity type to a string name. */
    static const std::string entity_type_names[] = {
        "none",
        "triangle",
        "sphere",
        "object"
    };



    /* The mode in which the entity needs to be pre-rendered. */
    enum EntityPreRenderModeFlags {
        /* No render mode defined. */
        eprmf_none = 0x0,
        /* Can be pre-rendered on the CPU. */
        eprmf_cpu = 0x1,
        /* Can be pre-rendered on the GPU. */  
        eprmf_gpu = 0x2
    };
    
    /* Describes the possible pre-rendering operations that the Renderer has to do for this entity. */
    enum EntityPreRenderOperation {
        /* No render operation defined. */
        epro_none = 0,
        /* Generate a 2D triangle. */
        epro_generate_triangle = 1,
        /* Generate a 3D sphere. */
        epro_generate_sphere = 2,
        /* Load an object file. */
        epro_load_object_file = 3
    };
    /* Maps an entity pre render operation to a string name. */
    static const std::string entity_pre_render_operation_names[] = {
        "none",
        "generate_triangle",
        "generate_sphere",
        "load_object_file"
    };



    /* The RenderEntity struct, which forms the basis for all RenderEntities. */
    struct RenderEntity {
        /* The type of the RenderEntity. */
        EntityType type;

        /* How the entity needs to be pre-rendered. */
        unsigned int pre_render_mode;
        /* The pre-rendering operation that needs to happen for this entity. */
        EntityPreRenderOperation pre_render_operation;
        /* The number of faces generated during pre-rendering for this entity. Note that we require this to be known _before_ pre-rendering starts. */
        uint32_t pre_render_faces;
        /* The number of vertices generated during pre-rendering for this entity. Note that we require this to be known _before_ pre-rendering starts. */
        uint32_t pre_render_vertices;

    };
}

#endif
