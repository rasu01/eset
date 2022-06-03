#include "molecule.h"

using namespace bunshi;

void Molecule::remove_entity(Entity entity) {

    //since we check if the entity exist in the universe, it should exist here too.
    //therefore, we don't need to check again inside this molecule
    size_t offset = entity_to_offset[entity];

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
}