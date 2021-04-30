/* RENDER ENTITY.hpp
 *   by Lut99
 *
 * Created:
 *   30/04/2021, 13:08:59
 * Last edited:
 *   30/04/2021, 15:21:28
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

namespace RayTracer::ECS {
    /* List of all entities registered to the RayTracer. */
    enum class EntityType {
        /* Basic sphere shape. */
        sphere,

        /* Finally, a type indicated that it's not set yet. */
        none
    };

    /* The mode in which the entity needs to be pre-rendered. */
    enum class EntityPreRenderModeFlags {
        /* No render mode defined. */
        none = 0x0,
        /* Can be pre-rendered on the CPU. */
        cpu = 0x1,
        /* Can be pre-rendered on the GPU. */  
        gpu = 0x2
    };
    /* Allows the flags to be OR'ed. */
    inline EntityPreRenderModeFlags operator|(const EntityPreRenderModeFlags& e1, const EntityPreRenderModeFlags& e2) { return (EntityPreRenderModeFlags) ((uint32_t) e1 | (uint32_t) e2); }
    /* Allows the flags to be AND'ed. */
    inline EntityPreRenderModeFlags operator&(const EntityPreRenderModeFlags& e1, const EntityPreRenderModeFlags& e2) { return (EntityPreRenderModeFlags) ((uint32_t) e1 & (uint32_t) e2); }



    /* The RenderEntity struct, which forms the basis for all RenderEntities. */
    struct RenderEntity {
        /* The type of the RenderEntity. */
        EntityType type;

        /* How the entity needs to be pre-rendered. */
        EntityPreRenderModeFlags render_mode;
    };
}

#endif
