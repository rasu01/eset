#pragma once
#include "types.h"
#include <iostream>
#include <vector>

namespace bunshi {

    class BaseStorage {
        public:
        
            /*
                Get the pointer to a component from an Entity's offset.
            */
            virtual void* get_component_pointer(size_t& offset) = 0;

            /*
                Moves the last component to another offset.
                Very useful when removing an entity's components from one
                archetype.
            */
            virtual void move_from_end(size_t destination_offset) = 0;

            /*
                Returns the size in bytes of this
                component storage's component!
            */
            virtual size_t get_component_size() = 0;

            /*
                Returns the type id of this component
                storage's component type.
            */
            virtual size_t get_component_type_id() = 0;

            /*
                Returns the amount of components inside this
                storage.
            */
            virtual size_t get_component_count() = 0;

            /*
                Inserts data at the end of the storage.
            */
            virtual void push_back(void* pointer) = 0;

            /*
                Moves data from data pointer to
                the given offset. This keeps the
                size of the storage.
            */
            virtual void set_component(size_t offset, void* data_pointer) = 0;

            /*
                Removes the last component.
                In reality, it just decreases the component_count by one.
            */
            virtual void remove_end() = 0;

            /*
                Makes a copy of a component storage.
                The copy is completely empty.
            */
            virtual BaseStorage* make_empty_copy() = 0;

            /*
                Default destructor. Marked virtual 
                since we want the C++ compiler to choose
                the derived destructor. Not this base class'
                destructor.
            */
            virtual ~BaseStorage() = default;

        private:
    };

    template<typename ComponentType>
    class ComponentStorage : public BaseStorage {

        public:
            void* get_component_pointer(size_t& offset) {
                return &components[offset];
            }

            void move_from_end(size_t destination_offset) {
                components[destination_offset] = std::move(components.back());
            }

            size_t get_component_size() {
                return sizeof(ComponentType);
            }

            size_t get_component_count() {
                return components.size();
            }

            size_t get_component_type_id() {
                return Types::type_id<ComponentType>();
            }
            
            void push_back(void* pointer) {
                components.push_back(std::move(*(ComponentType*)pointer));
            }

            void set_component(size_t offset, void* data_pointer) {
                components[offset] = std::move(*(ComponentType*)data_pointer);
            }

            void remove_end() {
                components.pop_back();
            }

            BaseStorage* make_empty_copy() {
                return new ComponentStorage<ComponentType>();
            }

        private:
            std::vector<ComponentType> components;
    };
}