#pragma once
#include "compound.h"
#include "molecule.h"
#include "entity.h"

namespace bunshi {

    //a way to set a unique id to a type
    class TypeId {
        public:

            //this function takes a type as argument and returns a unique id to that specific type
            template<typename T>
            static size_t from() {
                static size_t type_id = type_id_counter++;
                return type_id;
            }

        private:
            static int type_id_counter;
    };

    int TypeId::type_id_counter = 0;
}