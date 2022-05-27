#pragma once
#include "types.h"

namespace bunshi {

    //a strorage used for a single component inside a molecule
    class ComponentStorage {
        public:

            ComponentStorage() = default;
            ComponentStorage(size_t component_size, size_t start_size);
            ~ComponentStorage();

            void* get_component_pointer(size_t offset);
            void swap_by_offsets(size_t a, size_t b);
        private:
            uint8_t* data;
            size_t size;
            size_t component_size;

            /*
                //Resizes the data storage if more space is needed.
                It will double the space used.
            */
            void resize();
    };
}