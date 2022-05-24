#pragma once
#include "component_storage.h"
#include "types.h"
#include <unordered_map>
#include <vector>

namespace bunshi {

    //A molecule is in ECS terms just an archetype. It's definitions is just the types of components used.
    //It contains an unordered map with all the component's data inside it.
    class Molecule {
        public:

            /*
                Returns a type id local to the instance of a molecule.
            */
            template<typename T>
            size_t get_local_type_id() {
                static size_t type_id = local_type_id_counter++;
                return type_id;
            }

            /*
                Provides an entity index and a type argument
                to then return that component data
            */
            template<typename T>
            T* get_component(Entity entity) {
                size_t component_index = get_local_type_id<T>();
                size_t offset = entity_to_offset[entity];
                return (T*)compound[component_index].get_component_pointer(offset);    
            }
        private:

            //id handling
            size_t local_type_id_counter = 0;

            //the entities' data
            std::vector<ComponentStorage> compound;

            //these are all the entities that have this molecule type
            std::unordered_map<Entity, size_t> entity_to_offset;
            std::vector<Entity> offset_to_entity;
    };
}