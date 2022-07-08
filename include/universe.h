#pragma once
#include <vector>
#include <unordered_map>
#include "universe.h"
#include "molecule.h"
#include "types.h"

namespace bunshi {

    template<typename... T>
    class EntityIterator {

        public:
            EntityIterator begin() {
                EntityIterator it;
                it.molecule_index = 0;
                it.entity_index = 0;
                it.finished = false;
                return it;
            }

            EntityIterator end() {
                EntityIterator it;
                it.molecule_index = 0;
                it.entity_index = 0;
                it.finished = true;
                return it;
            }   

            bool operator!=(EntityIterator& rhs) {
                return molecule_index != rhs.molecule_index && entity_index != rhs.entity_index && finished != rhs.finished;
            }

            void operator++() {
                //finished = true;
            }

            std::tuple<Entity, T*...> operator*() {
                
            }

        private:
            friend Universe;
            Universe* universe;
            size_t molecule_index;
            size_t entity_index;
            bool finished;
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
                std::unordered_map<Entity, size_t>::iterator molecule_index_it = entities.find(entity);
                if(molecule_index_it != entities.end()) {

                    //it exists
                    size_t molecule_index = molecule_index_it->second;
                    Molecule& current_molecule = molecules[molecule_index];

                    //check if the current molecule already has this component
                    if(current_molecule.has_component<T>()) {
                        
                        //then we replace it with the new one that was given
                        size_t offset = current_molecule.entity_to_offset[entity];
                        ComponentStorage& storage = current_molecule.compound[typeid(T).hash_code()];
                        storage.set_component(offset, &component);

                    } else {

                        //it doesn't exist, we need to check if there is a molecule that fits the spec of the new entity
                        MoleculeSignature signature = current_molecule.get_molecule_signature();
                        signature.add(typeid(T).hash_code(), sizeof(T));
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

                            for(auto& [id, storage] : molecules[molecule_index].compound) {
                                void* data = storage.get_component_pointer(old_offset);
                                new_molecule.compound[id].set_component(new_offset, data);
                            }

                            //remove from old molecule
                            molecules[molecule_index].remove_entity(entity);

                            //change molecule index since we moved it
                            entities[entity] = new_molecule_index;

                            //insert the new component
                            new_molecule.compound[typeid(T).hash_code()].set_component(new_offset, &component);

                        } else {

                            //found one!! swap with this!
                            Molecule& new_molecule = molecules[molecule_try];
                            new_molecule.insert_entity(entity);

                            size_t old_offset = molecules[molecule_index].entity_to_offset[entity];
                            size_t new_offset = new_molecule.entity_to_offset[entity];
                            for(auto& [id, storage] :  molecules[molecule_index].compound) {
                                void* data = storage.get_component_pointer(old_offset);
                                new_molecule.compound[id].set_component(new_offset, data);
                            }

                            //remove from old molecule
                            current_molecule.remove_entity(entity);

                            //change molecule index since we moved it
                            entities[entity] = molecule_try;

                            //insert the new component
                            new_molecule.compound[typeid(T).hash_code()].set_component(new_offset, &component);
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
                std::unordered_map<Entity, size_t>::iterator molecule_index_it = entities.find(entity);
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

            */
            template<typename... T>
            EntityIterator<T...> iterator() {

                //get the type ids
                MoleculeSignature signature;
                ((signature.add(typeid(T).hash_code(), sizeof(T))), ...);

                size_t found_molecules[1024];
                size_t molecule_amount = 0;
                for(size_t i = 0; i < molecules.size(); i++) {
                    if(molecules[i].get_molecule_signature().contains(signature)) {
                        found_molecules[molecule_amount] = i;
                        molecule_amount++;
                    }
                }

                //construct iterator
                EntityIterator<T...> iter;
                iter.finished = false;
                iter.molecule_index = 0;
                iter.entity_index = 0;
                iter.universe = this;
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
            size_t entity_counter = 0;

            //all the molecules
            std::vector<Molecule> molecules;

            //all the entities that exist inside this universe instance.
            //the value is which molecule index it uses in the vector above
            std::unordered_map<Entity, size_t> entities;
    };
}