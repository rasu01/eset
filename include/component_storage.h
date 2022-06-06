#pragma once
#include "types.h"

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
            void* get_component_pointer(size_t offset);

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

        private:
            uint8_t* data;
            size_t size;
            size_t component_count;
            size_t component_size;

            /*
                //Resizes the data storage if more space is needed.
                It will double the space used.
            */
            void resize();
    };
}