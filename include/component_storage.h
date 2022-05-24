#pragma once
#include "types.h"

namespace bunshi {

    //a strorage used for a single component inside a molecule
    class ComponentStorage {
        public:

            void* get_component_pointer(size_t offset);
        private:
            uint8_t* data;
            size_t size;
            size_t component_size;
    };
}