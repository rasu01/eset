#include "molecule.h"

using namespace bunshi;

bool Molecule::remove_entity(Entity entity) {

    //check if the entity exists first
    auto offset_it = entity_to_offset.find(entity);
    if(offset_it != entity_to_offset.end()) {
        size_t offset = offset_it->second;
        
        //remove all the components and swap end components
        for(ComponentStorage& storage : compound) {
            storage.copy_end(offset);
            storage.remove_end();
        }

        //copy the last entity to this entity's offset
        size_t last_entity_offset = offset_to_entity.size()-1;
        Entity last_entity = offset_to_entity[last_entity_offset];
        entity_to_offset[last_entity] = offset;
        offset_to_entity[offset] = last_entity;

        //then remove this entity
        entity_to_offset.erase(entity);
        offset_to_entity.pop_back(); //since we "swaped", we delete the last element now

        return true;
    } else {
        return false;
    }
}