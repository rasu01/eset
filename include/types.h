#pragma once
#include <stdlib.h>
#include <cstdint>

namespace bunshi {

    //Entity definition, it's just a size_t
    #define MAX_COMPONENTS 1024
    using Entity = size_t;
    static const Entity null = 0;

    class Types {
        public:
            /*
                Returns a unique id for a certain
                        component.
            */
            template<typename T>
            static size_t type_id() {
                static size_t id = component_counter++;
                return id;
            }

        private:
            //component counter
            static size_t component_counter;
    };
}