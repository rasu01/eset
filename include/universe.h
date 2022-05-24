#pragma once
#include <vector>
#include <unordered_map>
#include "universe.h"
#include "molecule.h"
#include "types.h"

namespace bunshi {

    //An ECS collection with all the entites stored inside compounds, that are stored in specific molecules(archetypes).
    class Universe {
        public:

            bool exist(Entity entity);
        private:

            //all the molecules
            std::vector<Molecule> molecules;

            //all the entities that exist inside this universe instance.
            //the value is which molecule index it uses in the vector above
            std::unordered_map<Entity, size_t> entities;
    };
}