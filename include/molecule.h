#pragma once
#include <unordered_map>
#include <vector>
#include <array>
#include <cstring>
#include "component_storage.h"
#include "types.h"
#include "signal.h"

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

    class FastSignature {

        public:
            FastSignature() {
                memset(m_ids, 0, MAX_COMPONENTS/8);
            };
            
            size_t count();

            inline void clear() {
                memset(m_ids, 0, MAX_COMPONENTS/8);
            }
            
            inline void add(size_t id) {
                size_t segment = id / 8;
                size_t rest = id - (segment*8);
                m_ids[segment] = m_ids[segment] | (1 << rest);
                m_count++;
            }

            inline bool contains(FastSignature& other) {
                if(m_count < other.m_count) {
                    return false;
                }

                for(size_t i = 0; i < MAX_COMPONENTS / 8; i++) {
                    if((m_ids[i] & other.m_ids[i]) != other.m_ids[i]) {
                        return false;
                    }
                }
                return true;
            }

            inline bool contains(size_t id) {
                size_t segment = id / 8;
                size_t rest = id - (segment*8);
                return ((m_ids[segment] | (1 << rest)) == m_ids[segment]);
            }

        private:
            size_t m_count = 0;
            uint8_t m_ids[MAX_COMPONENTS / 8];

    };

    //A molecule is in ECS terms just an archetype. It's definitions is just the types of components used.
    //It contains an unordered map with all the component's data inside it.
    class Molecule {
        public:

            Molecule() = default;
            Molecule(std::vector<BaseStorage*>& storage_pointers);
            ~Molecule();

            /*
                Move constructor. Needed since 
                emplace_back might reallocate if the
                capacity is too low. And we don't want
                to delete a molecule twice(because of copying) with the same
                component storages.. we want to copy it.
            */
            Molecule(Molecule&& other);

            /*
                Provides an entity index and a type argument
                to then return that component data
            */
            template<typename T>
            inline T* get_component(Entity entity) {

                //get the component index
                size_t component_index = Types::type_id<T>(); 

                //check if the component exist here
                if(has_component<T>()) {

                    //the component did exist! Return it.
                    size_t offset = entity_to_offset[entity];
                    return (T*)(compound[component_index]->get_component_pointer(offset));
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
            void remove_entity(Entity entity, Signal<Entity>* on_destroy_signals);

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
            inline bool has_component() {
                if(fast_signature.contains(Types::type_id<T>())) {
                    return compound[Types::type_id<T>()]->get_component_size() != 0;
                } else {
                    return false;
                }
            }

            /*
                Returns an array containing all the
                different type's ids.
            */
            MoleculeSignature get_molecule_signature();

            FastSignature get_fast_signature();

            inline size_t count() {
                return offset_to_entity.size();
            }

            inline Entity get_entity(size_t& offset) {
                return offset_to_entity[offset];
            }

            template<typename T>
            inline void* get_data(size_t& offset) {
                return compound[Types::type_id<T>()]->get_component_pointer(offset);
            }

        private:

            friend Universe;
            template<typename...> friend class EntityIterator;

            //these are all the entities that have this molecule type
            std::unordered_map<Entity, size_t> entity_to_offset;
            std::vector<Entity> offset_to_entity;

            //the entities' data
            std::vector<size_t> compound_indices;
            BaseStorage* compound[MAX_COMPONENTS];
            FastSignature fast_signature;
    };
}