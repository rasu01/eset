#include "universe.h"

using namespace bunshi;

bool Universe::remove(Entity entity) {
    std::unordered_map<Entity, size_t>::iterator molecule_index_it = entities.find(entity);
    if(molecule_index_it != entities.end()) {
        size_t molecule_index = molecule_index_it->second;
        return molecules[molecule_index].remove_entity(entity);
    } else {
        return false;
    }
}

bool Universe::exist(Entity entity) {
    return entities.find(entity) != entities.end();
}