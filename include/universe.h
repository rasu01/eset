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
        private:

            //counter for all the entites that have been spawned
            size_t entity_counter = 0;

            //all the molecules
            std::vector<Molecule> molecules;

            //all the entities that exist inside this universe instance.
            //the value is which molecule index it uses in the vector above
            std::unordered_map<Entity, size_t> entities;
    };
}