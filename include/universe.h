#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <array>
#include <chrono>
#include "molecule.h"
#include "types.h"
#include <utility>


namespace bunshi {

    class Universe;

    template<typename... T>
    class EntityIterator {
        
        public:
            inline EntityIterator begin() {

                EntityIterator it;
                it.molecule_index = 0;
                it.entity_index = 0;
                it.molecule_count = molecule_count;

                //copy molecule pointers
                for(size_t i = 0; i < molecule_count; i++) {
                    it.molecules[i] = molecules[i];
                }

                if(molecule_count > 0) {
                    it.current_molecule = molecules[0];
                    it.component_index = 0;
                    ((it.component_index++, it.storages[it.component_index-1] = &it.current_molecule->compound[Types::type_id<T>()]), ...);

                } else {
                    it.molecule_index = -1;
                    it.entity_index = -1;
                }

                return it;
            }

            inline EntityIterator end() {
                EntityIterator it;
                it.molecule_index = -1;
                it.entity_index = -1;
                return it;
            }

            inline bool operator!=(EntityIterator& rhs) {
                return molecule_index != rhs.molecule_index;
            }

            inline void operator++() {

                size_t max_entities = current_molecule->count();
                entity_index++;

                if(entity_index == max_entities) {
                    entity_index = 0;
                    molecule_index++;

                    if(molecule_index == molecule_count) {
                        molecule_index = -1;
                        entity_index = -1;
                    } else {
                        current_molecule++;// = molecules[molecule_index];
                        component_index = 0;
                        ((component_index++, storages[component_index-1] = &current_molecule->compound[Types::type_id<T>()]), ...);
                    }
                }
            }

            inline std::tuple<Entity, T&...> operator*() {
                component_index = 0;
                return {current_molecule->get_entity(entity_index), (component_index++, *(T*)storages[component_index-1]->get_component_pointer(entity_index))...};
            }

        private:
            friend Universe;

            static size_t counter;
            size_t molecule_index;
            size_t entity_index;
            size_t component_index = 0;
            size_t molecule_count = 0;
            Molecule* current_molecule = nullptr;
            ComponentStorage* storages[sizeof...(T)];
            Molecule* molecules[128];
    };

    //An ECS collection with all the entites stored inside compounds, that are stored in specific molecules(archetypes).
    class Universe {
        public:

            Universe();

            /*
                Returns true if the entity provided exists,
                otherwise return false
            */
            bool exist(Entity entity);

            /*
                Tries to remove an entity. If successful, the function
                returns true, else it returns false.
            */
            bool remove(Entity entity);


            /*
                Creates a single entity with no components attached.
                Returns the id handle to the entity
            */
            Entity create();

            /*
                Tries to inserts a component into an entity.
                Returns true on success, and false on failure.
                If the entity already has a certain component,
                this function will overwrite it, and then return
                true.
            */
            template<typename T>
            bool insert_component(Entity entity, T component) {

                //check if the entity exists
                auto molecule_index_it = entities.find(entity);
                if(molecule_index_it != entities.end()) {

                    //it exists
                    size_t molecule_index = molecule_index_it->second;
                    Molecule& current_molecule = molecules[molecule_index];

                    //check if the current molecule already has this component
                    if(current_molecule.has_component<T>()) {
                        
                        //then we replace it with the new one that was given
                        size_t offset = current_molecule.entity_to_offset[entity];
                        ComponentStorage& storage = current_molecule.compound[Types::type_id<T>()];
                        storage.set_templated_component<T>(offset, component);

                    } else {

                        //it doesn't exist, we need to check if there is a molecule that fits the spec of the new entity
                        MoleculeSignature signature = current_molecule.get_molecule_signature();
                        signature.add(Types::type_id<T>(), sizeof(T));
                        size_t molecule_try = find_molecule(signature);

                        //couldn't find a molecule, we need to create one and swap with the new one
                        if(molecule_try == -1) {

                            size_t new_molecule_index = molecules.size();
                            molecules.emplace_back(signature);

                            //swap
                            Molecule& new_molecule = molecules[new_molecule_index];
                            new_molecule.insert_entity(entity);

                            size_t old_offset = molecules[molecule_index].entity_to_offset[entity];
                            size_t new_offset = new_molecule.entity_to_offset[entity];

                            for(size_t id : molecules[molecule_index].compound_indices) {
                                ComponentStorage& storage = molecules[molecule_index].compound[id];
                                void* data = storage.get_component_pointer(old_offset);
                                new_molecule.compound[id].set_component(new_offset, data); 
                            }

                            //remove from old molecule
                            molecules[molecule_index].remove_entity(entity);

                            //insert the new component
                            size_t id = Types::type_id<T>();
                            new_molecule.compound[id].set_templated_component<T>(new_offset, component);
                            
                            //change molecule index since we moved it
                            molecule_index_it->second = new_molecule_index;

                        } else {

                            //found one!! swap with this!
                            Molecule& new_molecule = molecules[molecule_try];
                            new_molecule.insert_entity(entity);

                            size_t old_offset = molecules[molecule_index].entity_to_offset[entity];
                            size_t new_offset = new_molecule.entity_to_offset[entity];
                            for(size_t id : molecules[molecule_index].compound_indices) {
                                ComponentStorage& storage = molecules[molecule_index].compound[id];
                                void* data = storage.get_component_pointer(old_offset);
                                new_molecule.compound[id].set_component(new_offset, data);
                            }

                            //remove from old molecule
                            current_molecule.remove_entity(entity);

                            //insert the new component
                            size_t id = Types::type_id<T>();
                            new_molecule.compound[id].set_templated_component<T>(new_offset, component);
                            
                            //change molecule index since we moved it
                            molecule_index_it->second = molecule_try;
                        }
                    }
                    return true;

                } else {

                    //else the entity didn't exist. Return false.
                    return false;
                }
            }

            /*
                Tries to return a reference to a component
                from an entity.
            */
            template<typename T>
            T* get_component(Entity entity) {

                //check if it exists
                auto molecule_index_it = entities.find(entity);
                if(molecule_index_it != entities.end()) {

                    //if it does, return the component from the molecule
                    size_t molecule_index = molecule_index_it->second;
                    return molecules[molecule_index].get_component<T>(entity);
                } else {

                    //else return nullptr, since the entity doesn't exist
                    return nullptr;
                }
            }

            /*
                Returns an iterator based on the 
                type arguments given. This then iterators over
                every entity that has these components.
            */
            template<typename... T>
            EntityIterator<T...> iterator() {

                //get the type ids
                //MoleculeSignature signature;
                //((signature.add(Types::type_id<T>(), sizeof(T))), ...);

                FastSignature sign;
                ((sign.add(Types::type_id<T>())), ...);

                //construct iterator
                EntityIterator<T...> iter;
                iter.molecule_index = 0;
                iter.entity_index = 0;
                iter.molecule_count = 0;

                //find molecules
                for(size_t i = 0; i < molecules.size(); i++) {
                    if(molecules[i].count() > 0) {
                        if(molecules[i].get_fast_signature().contains(sign)) {
                            iter.molecules[iter.molecule_count] = &molecules[i];
                            iter.molecule_count++;
                        }
                    }
                }

                /*for(size_t i = 0; i < molecules.size(); i++) {
                    if(molecules[i].count() > 0) {
                        if(molecules[i].get_molecule_signature().contains(signature)) {
                            iter.molecules[iter.molecule_count] = &molecules[i];
                            iter.molecule_count++;
                        }
                    }
                }*/

                return iter;
            }

            /*
                Tries to find a molecule with a given signature.
                Returns the index if it exists, otherwise it returns
                -1. Since this is a size_t, it means that it will return
                the largest value. (infinity..?)
            */
            size_t find_molecule(MoleculeSignature& signature);
            
        private:

            //counter for all the entites that have been spawned
            //starts at 1, since 0 is the "null" entity.
            size_t entity_counter = 1;

            //all the molecules
            std::vector<Molecule> molecules;

            //all the entities that exist inside this universe instance.
            //the value is which molecule index it uses in the vector above
            std::unordered_map<Entity, size_t> entities;
    };
}