#include "universe.h"

using namespace eset;

Universe::Universe() {

    //create an empty archetype, so we can assign newly created entities to it
    archetypes.emplace_back();
} 

bool Universe::remove(Entity entity) {
    auto archetype_index_it = entities.find(entity);
    if(archetype_index_it != entities.end()) {
        size_t archetype_index = archetype_index_it->second;
        archetypes[archetype_index].remove_entity(entity, on_remove_signals);
        entities.erase(entity);
        return true;
    } else {
        return false;
    }
}

bool Universe::exist(Entity entity) {
    return entities.find(entity) != entities.end();
}

Entity Universe::create() {
    size_t new_id = entity_counter;
    entity_counter++;
    entities.emplace(new_id, 0); //assign the default archetype
    archetypes[0].insert_entity(new_id);
    return new_id;
}

size_t Universe::find_archetype(ArchetypeSignature& signature) {

    for(size_t i = 0; i < archetypes.size(); i++) {
        if(archetypes[i].get_archetype_signature() == signature) {
            return i;
        }
    }

    return -1;
}