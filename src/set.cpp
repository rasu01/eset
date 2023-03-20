#include "set.h"

using namespace eset;

Set::Set() {

    //create an empty archetype, so we can assign newly created entities to it
    archetypes.emplace_back();
} 

Set::~Set() {

    //remove all set pointers from references
    for(auto [reference_data, component_pointer] : reference_data_to_component) {
        reference_data->m_set = nullptr;
    }
}

bool Set::remove(Entity entity) {
    auto archetype_index_it = entities.find(entity);
    if(archetype_index_it != entities.end()) {
        size_t archetype_index = archetype_index_it->second;
        archetypes[archetype_index].remove_entity(entity, this);
        entities.erase(entity);
        return true;
    } else {
        return false;
    }
}

bool Set::exist(Entity entity) {
    return entities.find(entity) != entities.end();
}

Entity Set::create() {
    size_t new_id = entity_counter;
    entity_counter++;
    entities.emplace(new_id, 0); //assign the default archetype
    archetypes[0].insert_entity(new_id);
    return new_id;
}

size_t Set::find_archetype(ArchetypeSignature& signature) {

    for(size_t i = 0; i < archetypes.size(); i++) {
        if(archetypes[i].get_archetype_signature() == signature) {
            return i;
        }
    }

    return -1;
}

ReferenceData* Set::reference_data(void* component_pointer) {
    auto it = component_to_reference_data.find(component_pointer);
    if(it != component_to_reference_data.end()) {
        return it->second;
    } else {
        //it doesn't exist, so create a new one
        ReferenceData* new_reference_data = new ReferenceData();
        new_reference_data->m_reference_count = 0;
        new_reference_data->m_raw_pointer = component_pointer;
        new_reference_data->m_set = this;

        //insert it and return
        component_to_reference_data.emplace(component_pointer, new_reference_data);
        reference_data_to_component.emplace(new_reference_data, component_pointer);
        return new_reference_data;
    }
}

void Set::swap_reference_data(void* old_component_pointer, void* new_component_pointer) {
    auto crd = component_to_reference_data.find(old_component_pointer);
    if(crd != component_to_reference_data.end()) {
        ReferenceData* reference_data = crd->second;
        reference_data->m_raw_pointer = new_component_pointer;
        component_to_reference_data.erase(old_component_pointer);
        component_to_reference_data.emplace(new_component_pointer, reference_data);
    }
}

void Set::delete_reference_pointer(ReferenceData* reference_data) {
    auto it = reference_data_to_component.find(reference_data);
    if(it != reference_data_to_component.end()) {
        void* component_pointer = it->first->m_raw_pointer;
        delete reference_data;

        //remove from set
        component_to_reference_data.erase(component_pointer);
        reference_data_to_component.erase(reference_data);
    }
}

void Set::make_reference_data_pointer_null(void* component_pointer) {
    auto it = component_to_reference_data.find(component_pointer);
    if(it != component_to_reference_data.end()) {
        it->second->m_raw_pointer = nullptr;
        component_to_reference_data.erase(component_pointer);
    }
}

void Set::make_reference_entity_null(void* component_pointer) {
    auto it = component_to_reference_data.find(component_pointer);
    if(it != component_to_reference_data.end()) {
        it->second->m_entity = 0;
    }
}
