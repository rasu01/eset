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
                Copies the last component to another offset.
                Very useful when removing an entity's components from one
                molecule(archetype).
            */
            virtual void copy_from_end(size_t destination_offset) = 0;

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
                Inserts default data bat the end of the storage.
                Basically emplaces the default constructor at the end.
            */
            virtual void insert_default_end() = 0;

            /*
                Copies data from data pointer to
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

        private:
    };

    template<typename ComponentType>
    class ComponentStorage : public BaseStorage {

        public:
            void* get_component_pointer(size_t& offset) {
                return &components[offset];
            }

            void copy_from_end(size_t destination_offset) {
                components[destination_offset] = components.back();
                //std::swap(components[destination_offset], components.back());
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

            void insert_default_end() {
                components.emplace_back();
            }

            void set_component(size_t offset, void* data_pointer) {
                components[offset] = *(ComponentType*)data_pointer;
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