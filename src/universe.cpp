#include "universe.h"

using namespace bunshi;

Universe::Universe() {

    //create an empty archetype, so we can assign newly created entities to it
    molecules.push_back(Molecule());
}

bool Universe::remove(Entity entity) {
    std::unordered_map<Entity, size_t>::iterator molecule_index_it = entities.find(entity);
    if(molecule_index_it != entities.end()) {
        size_t molecule_index = molecule_index_it->second;
        molecules[molecule_index].remove_entity(entity);
        return true;
    } else {
        return false;
    }
}

bool Universe::exist(Entity entity) {
    return entities.find(entity) != entities.end();
}

Entity Universe::create() {
    Entity entity = entity_counter++;
    entities.insert({entity, 0}); //assign the default archetype
    molecules[0].insert_entity(entity);
    return entity;
}

size_t Universe::find_molecule(MoleculeSignature& signature) {

    for(size_t i = 0; i < molecules.size(); i++) {
        if(molecules[i].get_molecule_signature() == signature) {
            return i;
        }
    }

    return -1;
}