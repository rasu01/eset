#include "set.h"

using namespace eset;

Set::Set() {

    //create an empty archetype, so we can assign newly created entities to it
    archetypes.emplace_back();
} 

Set::~Set() {

    //remove all set pointers from references
    for(ReferenceData* reference_data : reference_datas) {
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

ReferenceData* Set::reference_data(BaseStorage* storage, size_t offset) {
    uint64_t sid = storage->storage_offset_identifier(offset);
    auto it = sid_to_reference_data.find(sid);
    if(it != sid_to_reference_data.end()) {
        return it->second;
    } else {
        //it doesn't exist, so create a new one
        ReferenceData* new_reference_data = new ReferenceData();
        new_reference_data->m_reference_count = 0;
        new_reference_data->m_storage = storage;
        new_reference_data->m_offset = offset;
        new_reference_data->m_set = this;

        //insert it and return
        sid_to_reference_data.emplace(sid, new_reference_data);
        reference_datas.emplace(new_reference_data);
        return new_reference_data;
    }
}

void Set::swap_reference_data(BaseStorage* old_storage, uint64_t old_offset, BaseStorage* new_storage, uint64_t new_offset) {

    uint64_t old_sid = old_storage->storage_offset_identifier(old_offset);
    uint64_t new_sid = new_storage->storage_offset_identifier(new_offset);
    auto srd = sid_to_reference_data.find(old_sid);
    if(srd != sid_to_reference_data.end()) {
        ReferenceData* reference_data = srd->second;
        reference_data->m_storage = new_storage;
        reference_data->m_offset = new_offset;
        sid_to_reference_data.erase(old_sid);
        sid_to_reference_data.emplace(new_sid, reference_data);

        
    }
}

void Set::delete_reference_pointer(ReferenceData* reference_data) {
    if(reference_datas.find(reference_data) != reference_datas.end()) {
        BaseStorage* storage = reference_data->m_storage;
        uint64_t offset = reference_data->m_offset;
        delete reference_data;

        //remove from set
        if(storage) {
            sid_to_reference_data.erase(storage->storage_offset_identifier(offset));
        }
        if(reference_data) {
            reference_datas.erase(reference_data);
        }
    }
}

void Set::make_reference_data_pointer_null(BaseStorage* storage, uint64_t offset) {
    uint64_t sid = storage->storage_offset_identifier(offset);
    auto it = sid_to_reference_data.find(sid);
    if(it != sid_to_reference_data.end()) {
        it->second->m_storage = nullptr;
        it->second->m_offset = 0;
        sid_to_reference_data.erase(sid);
    }
}

void Set::make_reference_entity_null(BaseStorage* storage, uint64_t offset) {
    uint64_t sid = storage->storage_offset_identifier(offset);
    auto it = sid_to_reference_data.find(sid);
    if(it != sid_to_reference_data.end()) {
        it->second->m_entity = 0;
    }
}
