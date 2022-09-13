#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <array>
#include <chrono>
#include <utility>
#include "archetype.h"
#include "types.h"
#include "signal.h"

namespace bunshi {

    class Universe;

    template<typename... T>
    class EntityIterator {
        
        public:
            inline EntityIterator begin() {

                EntityIterator it;
                it.archetype_index = 0;
                it.entity_index = 0;
                it.archetype_count = archetype_count;

                //copy archetype pointers
                for(size_t i = 0; i < archetype_count; i++) {
                    it.archetypes[i] = archetypes[i];
                }

                if(archetype_count > 0) {
                    it.current_archetype = archetypes[0];
                    it.set_storages(std::make_index_sequence<sizeof...(T)>{});
                } else {
                    it.archetype_index = -1;
                    it.entity_index = -1;
                }

                return it;
            }

            inline EntityIterator end() {
                EntityIterator it;
                it.archetype_index = -1;
                it.entity_index = -1;
                return it;
            }

            inline bool operator!=(EntityIterator& rhs) {
                return archetype_index != rhs.archetype_index;
            }

            inline void operator++() {

                size_t max_entities = current_archetype->count();
                entity_index++;

                if(entity_index == max_entities) {
                    entity_index = 0;
                    archetype_index++;

                    if(archetype_index == archetype_count) {
                        archetype_index = -1;
                        entity_index = -1;
                    } else {
                        current_archetype++;
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
                return {current_archetype->get_entity(entity_index), (*(T*)storages[index]->get_component_pointer(entity_index))...};
            }

            template<size_t... index>
            inline void set_storages(std::integer_sequence<size_t, index...>) {
                ((storages[index] = current_archetype->compound[Types::type_id<T>()]), ...);
            }

            static size_t counter;
            size_t archetype_index;
            size_t entity_index;
            size_t archetype_count = 0;
            Archetype* current_archetype = nullptr;
            BaseStorage* storages[sizeof...(T)];
            Archetype* archetypes[128];
    };


    //An ECS collection with all the entites stored inside compounds, that are stored in specific archetypes(archetypes).
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
                auto archetype_index_it = entities.find(entity);
                if(archetype_index_it != entities.end()) {

                    //it exists
                    size_t archetype_index = archetype_index_it->second;
                    Archetype& current_archetype = archetypes[archetype_index];

                    //check if the current archetype already has this component
                    if(current_archetype.has_component<T>()) {
                        
                        //then we replace it with the new one that was given
                        size_t offset = current_archetype.entity_to_offset[entity];
                        BaseStorage* storage = current_archetype.compound[Types::type_id<T>()];
                        storage->set_component(offset, &component);

                    } else {

                        //it doesn't exist, we need to check if there is a archetype that fits the spec of the new entity
                        ArchetypeSignature signature = current_archetype.get_archetype_signature();
                        signature.add(Types::type_id<T>(), sizeof(T));
                        size_t archetype_try = find_archetype(signature);

                        //couldn't find a archetype, we need to create one and swap with the new one
                        if(archetype_try == -1) {

                            size_t new_archetype_index = archetypes.size();

                            //get the signatures
                            std::vector<BaseStorage*> storage_pointers;
                            for(size_t i = 0; i < current_archetype.compound_indices.size(); i++) {
                                //std::cout << i << "\n";
                                BaseStorage* copy = current_archetype.compound[current_archetype.compound_indices[i]]->make_empty_copy();
                                storage_pointers.push_back(copy);
                            }
                            BaseStorage* new_storage = new ComponentStorage<T>();
                            storage_pointers.push_back(new_storage);
                            archetypes.emplace_back(storage_pointers);

                            //swap
                            Archetype& new_archetype = archetypes[new_archetype_index];
                            new_archetype.insert_entity(entity);

                            size_t old_offset = archetypes[archetype_index].entity_to_offset[entity];
                            size_t new_offset = new_archetype.entity_to_offset[entity];

                            for(size_t id : archetypes[archetype_index].compound_indices) {
                                BaseStorage* storage = archetypes[archetype_index].compound[id];
                                void* data = storage->get_component_pointer(old_offset);
                                new_archetype.compound[id]->push_back(data); 
                            }

                            //remove from old archetype
                            archetypes[archetype_index].remove_entity(entity, nullptr);

                            //insert the new component
                            size_t id = Types::type_id<T>();
                            new_archetype.compound[id]->push_back(&component);
                            
                            //change archetype index since we moved it
                            archetype_index_it->second = new_archetype_index;

                        } else {

                            //found one!! swap with this!
                            Archetype& new_archetype = archetypes[archetype_try];
                            new_archetype.insert_entity(entity);

                            size_t old_offset = archetypes[archetype_index].entity_to_offset[entity];
                            size_t new_offset = new_archetype.entity_to_offset[entity];
                            for(size_t id : archetypes[archetype_index].compound_indices) {
                                BaseStorage* storage = archetypes[archetype_index].compound[id];
                                void* data = storage->get_component_pointer(old_offset);
                                new_archetype.compound[id]->push_back(data);
                            }

                            //remove from old archetype
                            current_archetype.remove_entity(entity, nullptr);

                            //insert the new component
                            size_t id = Types::type_id<T>();
                            new_archetype.compound[id]->push_back(&component);
                            
                            //change archetype index since we moved it
                            archetype_index_it->second = archetype_try;
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
                auto archetype_index_it = entities.find(entity);
                if(archetype_index_it != entities.end()) {

                    //if it does, return the component from the archetype
                    size_t archetype_index = archetype_index_it->second;
                    return archetypes[archetype_index].get_component<T>(entity);
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
                auto archetype_index_it = entities.find(entity);
                if(archetype_index_it != entities.end()) {

                    //if it does, return the component from the archetype
                    size_t archetype_index = archetype_index_it->second;
                    Archetype* archetype = &archetypes[archetype_index];
                    return {(archetype->get_component<Ts>(entity))...};
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
                iter.archetype_index = 0;
                iter.entity_index = 0;
                iter.archetype_count = 0;

                //find archetypes
                for(size_t i = 0; i < archetypes.size(); i++) {
                    if(archetypes[i].count() > 0) {
                        if(archetypes[i].get_fast_signature().contains(sign)) {
                            iter.archetypes[iter.archetype_count] = &archetypes[i];
                            iter.archetype_count++;
                        }
                    }
                }

                return iter;
            }

            /*
                Tries to find a archetype with a given signature.
                Returns the index if it exists, otherwise it returns
                -1. Since this is a size_t, it means that it will return
                the largest value. (infinity..?)
            */
            size_t find_archetype(ArchetypeSignature& signature);

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

            //all the archetypes
            std::vector<Archetype> archetypes;

            //signals
            Signal<Entity> on_remove_signals[MAX_COMPONENTS];

            //all the entities that exist inside this universe instance.
            //the value is which archetype index it uses in the vector above
            std::unordered_map<Entity, size_t> entities;
    };
}