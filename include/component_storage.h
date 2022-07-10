#pragma once
#include "types.h"
#include <iostream>

namespace bunshi {

    //a stack based strorage container used for a single component inside a molecule
    class ComponentStorage {
        public:

            ComponentStorage() = default;
            ComponentStorage(size_t component_size);
            ~ComponentStorage();

            /*
                Get the pointer to a component from an Entity's offset.
            */
            inline void* get_component_pointer(size_t& offset) {
                return data + offset*component_size;
            }

            template<typename T>
            inline T* get_templated_pointer(size_t& offset) {
                return ((T*)data) + offset;
            }

            /*
                Copies the last component to another offset.
                Very useful when removing an entity's components from one
                molecule(archetype).
            */
            void copy_end(size_t desination_offset);

            /*
                Inserts data from a component into the storage at the end.
            */
            void insert_end(void* pointer);

            /*
                Inserts default data(zeros) at the end of the storage
            */
            void insert_default_end();

            /*
                Removes the last component.
                In reality, it just decreases the component_count by one.
            */
            void remove_end();

            /*
                Returns the size in bytes of this
                component storage's component!
            */
            size_t get_component_size();

            /*
                Copies data from data pointer to
                the given offset. This keeps the
                size of the storage.
            */
            void set_component(size_t offset, void* pointer);

            /*
                Uses the copy constructor instead
            */
            template<typename T>
            void set_templated_component(size_t offset, T& component) {
                *((T*)data + offset) = component;
            }

        private:
            uint8_t* data = nullptr;
            size_t size = 0;
            size_t component_count;
            size_t component_size;

            /*
                //Resizes the data storage if more space is needed.
                It will double the space used.
            */
            void resize();
    };
}