#pragma once
#include "component_storage.h"
#include "types.h"
#include <unordered_map>
#include <vector>
#include <array>

namespace bunshi {

    class Molecule;
    class Universe;

    class MoleculeSignature {

        public:
            MoleculeSignature() = default;
            size_t count();
            void add(size_t id, size_t size);
            bool operator==(MoleculeSignature& rhs);
            bool contains(MoleculeSignature& other);

        private:
            friend Molecule;
            size_t m_count = 0;
            size_t m_ids[32];
            size_t m_sizes[32];
    };

    //A molecule is in ECS terms just an archetype. It's definitions is just the types of components used.
    //It contains an unordered map with all the component's data inside it.
    class Molecule {
        public:

            Molecule() = default;
            Molecule(MoleculeSignature& signature);

            /*
                Provides an entity index and a type argument
                to then return that component data
            */
            template<typename T>
            T* get_component(Entity entity) {

                //get the component index
                size_t component_index = typeid(T).hash_code();

                //check if the component exist here
                if(has_component<T>()) {

                    //the component did exist! Return it.
                    size_t offset = entity_to_offset[entity];
                    return (T*)compound[component_index].get_component_pointer(offset);
                } else {
                    
                    //the component doesn't exist, return nullptr.
                    return nullptr;
                }
            }

            /*
                Removes an entity from this molecule and it's corresponding components.
                Under the hood it will swap with the last entity that was added.
                This shouldn't really be used by the user, since removing a nonexistant 
                entity will give undefined behaviour.
            */
            void remove_entity(Entity entity);

            /*
                Initializes an entity inside the molecule.
                The data isn't set here, so it needs to be set exactly after this.
            */
            void insert_entity(Entity entity);

            /*
                Checks if this molecule has a certain component.
                Returns true if the component exists, false otherwise.
            */
            template<typename T>
            bool has_component() {
                
                for(auto& [id, storage] : compound) {
                    if(id == typeid(T).hash_code()) {
                        return true;
                    }
                }
                return false;
            }

            /*
                Returns an array containing all the
                different type's ids.
            */
            MoleculeSignature get_molecule_signature();

            inline size_t count() {
                return offset_to_entity.size();
            }

            inline Entity get_entity(size_t& offset) {
                return offset_to_entity[offset];
            }

            template<typename T>
            inline void* get_data(size_t& offset) {
                return compound[typeid(T).hash_code()].get_component_pointer(offset);
            }

        private:

            friend Universe;

            //id handling
            size_t local_type_id_counter = 0;

            //the entities' data
            std::unordered_map<size_t, ComponentStorage> compound;

            //these are all the entities that have this molecule type
            std::unordered_map<Entity, size_t> entity_to_offset;
            std::vector<Entity> offset_to_entity;
    };
}