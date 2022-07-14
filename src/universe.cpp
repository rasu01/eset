#include "universe.h"

using namespace bunshi;

Universe::Universe() {

    //create an empty archetype, so we can assign newly created entities to it
    molecules.emplace_back();
} 

bool Universe::remove(Entity entity) {
    auto molecule_index_it = entities.find(entity);
    if(molecule_index_it != entities.end()) {
        size_t molecule_index = molecule_index_it->second;
        molecules[molecule_index].remove_entity(entity);
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
    molecules[0].insert_entity(new_id);
    return new_id;
}

size_t Universe::find_molecule(MoleculeSignature& signature) {

    for(size_t i = 0; i < molecules.size(); i++) {
        if(molecules[i].get_molecule_signature() == signature) {
            return i;
        }
    }

    return -1;
}