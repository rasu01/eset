#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <array>
#include <chrono>
#include <utility>
#include "molecule.h"
#include "types.h"
#include "signal.h"

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
                    it.set_storages(std::make_index_sequence<sizeof...(T)>{});
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
                        current_molecule++;
                        set_storages(std::make_index_sequence<sizeof...(T)>{});
                    }
                }
            }

            inline std::tuple<Entity, T&...> operator*() {
                return get_tuple(std::make_index_sequence<sizeof...(T)>{});
            }

        private:
            friend Universe;

            template<size_t... index>
            inline std::tuple<Entity, T&...> get_tuple(std::integer_sequence<size_t, index...>) {
                return {current_molecule->get_entity(entity_index), (*(T*)storages[index]->get_component_pointer(entity_index))...};
            }

            template<size_t... index>
            inline void set_storages(std::integer_sequence<size_t, index...>) {
                ((storages[index] = current_molecule->compound[Types::type_id<T>()]), ...);
            }

            static size_t counter;
            size_t molecule_index;
            size_t entity_index;
            size_t molecule_count = 0;
            Molecule* current_molecule = nullptr;
            BaseStorage* storages[sizeof...(T)];
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
                        BaseStorage* storage = current_molecule.compound[Types::type_id<T>()];
                        storage->set_component(offset, &component);

                    } else {

                        //it doesn't exist, we need to check if there is a molecule that fits the spec of the new entity
                        MoleculeSignature signature = current_molecule.get_molecule_signature();
                        signature.add(Types::type_id<T>(), sizeof(T));
                        size_t molecule_try = find_molecule(signature);

                        //couldn't find a molecule, we need to create one and swap with the new one
                        if(molecule_try == -1) {

                            size_t new_molecule_index = molecules.size();

                            //get the signatures
                            std::vector<BaseStorage*> storage_pointers;
                            for(size_t i = 0; i < current_molecule.compound_indices.size(); i++) {
                                //std::cout << i << "\n";
                                BaseStorage* copy = current_molecule.compound[current_molecule.compound_indices[i]]->make_empty_copy();
                                storage_pointers.push_back(copy);
                            }
                            BaseStorage* new_storage = new ComponentStorage<T>();
                            storage_pointers.push_back(new_storage);
                            molecules.emplace_back(storage_pointers);

                            //swap
                            Molecule& new_molecule = molecules[new_molecule_index];
                            new_molecule.insert_entity(entity);

                            size_t old_offset = molecules[molecule_index].entity_to_offset[entity];
                            size_t new_offset = new_molecule.entity_to_offset[entity];

                            for(size_t id : molecules[molecule_index].compound_indices) {
                                BaseStorage* storage = molecules[molecule_index].compound[id];
                                void* data = storage->get_component_pointer(old_offset);
                                new_molecule.compound[id]->push_back(data); 
                            }

                            //remove from old molecule
                            molecules[molecule_index].remove_entity(entity, nullptr);

                            //insert the new component
                            size_t id = Types::type_id<T>();
                            new_molecule.compound[id]->push_back(&component);
                            
                            //change molecule index since we moved it
                            molecule_index_it->second = new_molecule_index;

                        } else {

                            //found one!! swap with this!
                            Molecule& new_molecule = molecules[molecule_try];
                            new_molecule.insert_entity(entity);

                            size_t old_offset = molecules[molecule_index].entity_to_offset[entity];
                            size_t new_offset = new_molecule.entity_to_offset[entity];
                            for(size_t id : molecules[molecule_index].compound_indices) {
                                BaseStorage* storage = molecules[molecule_index].compound[id];
                                void* data = storage->get_component_pointer(old_offset);
                                new_molecule.compound[id]->push_back(data);
                            }

                            //remove from old molecule
                            current_molecule.remove_entity(entity, nullptr);

                            //insert the new component
                            size_t id = Types::type_id<T>();
                            new_molecule.compound[id]->push_back(&component);
                            
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

            template<typename T, typename... Args>
            void emplace_component(Args&&... args) {

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
                Tries to return multiple pointers
                to multiple components. This is 
                more efficient than using get_component
                multiple times in a row to get different
                components from a single entity. 
            */
            template<typename... Ts>
            std::tuple<Ts*...> get_components(Entity entity) {
                //check if it exists
                auto molecule_index_it = entities.find(entity);
                if(molecule_index_it != entities.end()) {

                    //if it does, return the component from the molecule
                    size_t molecule_index = molecule_index_it->second;
                    Molecule* molecule = &molecules[molecule_index];
                    return {(molecule->get_component<Ts>(entity))...};
                } else {

                    //else return nullptr, since the entity doesn't exist
                    return {((Ts*)nullptr)...};
                }
            }

            /*
                Returns an iterator based on the 
                type arguments given. This then iterators over
                every entity that has these components.
            */
            template<typename... T>
            EntityIterator<T...> iterator() {

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

                return iter;
            }

            /*
                Tries to find a molecule with a given signature.
                Returns the index if it exists, otherwise it returns
                -1. Since this is a size_t, it means that it will return
                the largest value. (infinity..?)
            */
            size_t find_molecule(MoleculeSignature& signature);

            /*
                Connects a function to a signal
                that fires when a specific 
                components gets destroyed! 
            */
            template<typename T>
            void connect_on_remove(std::function<void(Entity)> function) {
                on_remove_signals[Types::type_id<T>()].connect(function);
            }

            /*
                Disconnects a function to 
                a signal that fires when 
                a specific function gets
                removed.
            */
            template<typename T>
            void disconnect_on_remove(std::function<void(Entity)> function) {
                on_remove_signals[Types::type_id<T>()].disconnect(function);
            }

            /*
                Connects a function that
                requires an instance to a signal
                that fires when a specific 
                components gets destroyed!
            */
            template<typename T, typename Function, typename InstanceType>
            void connect_on_remove(Function function, InstanceType instance) {
                on_remove_signals[Types::type_id<T>()].connect(function, instance);
            }
            
            /*
                Disconnects a function that
                requires an instance to a signal
                that fires when a specific 
                components gets destroyed!
                This should be done at most
                when the lifetime of the instance
                runs out. Otherwise the signal might
                throw a segmentation fault when emiting.
            */
            template<typename T, typename Function, typename InstanceType>
            void disconnect_on_remove(Function function, InstanceType instance) {
                on_remove_signals[Types::type_id<T>()].disconnect(function, instance);
            }
            
        private:

            //counter for all the entites that have been spawned
            //starts at 1, since 0 is the "null" entity.
            size_t entity_counter = 1;

            //all the molecules
            std::vector<Molecule> molecules;

            //signals
            Signal<Entity> on_remove_signals[MAX_COMPONENTS];

            //all the entities that exist inside this universe instance.
            //the value is which molecule index it uses in the vector above
            std::unordered_map<Entity, size_t> entities;
    };
}