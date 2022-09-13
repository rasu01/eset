#include "archetype.h"
#include <cassert>
#include <iostream>

using namespace eset;

Archetype::Archetype(Archetype&& other) {
    entity_to_offset = std::move(other.entity_to_offset);
    offset_to_entity = std::move(other.offset_to_entity);
    compound_indices = std::move(other.compound_indices);
    for(size_t compound_id : compound_indices) {
        compound[compound_id] = other.compound[compound_id];
        other.compound[compound_id] = nullptr;
    }
    fast_signature = other.fast_signature;
    other.fast_signature.clear();
}

Archetype::Archetype(std::vector<BaseStorage*>& storage_pointers) {

    for(BaseStorage* storage : storage_pointers) {
        size_t id = storage->get_component_type_id();
        compound[id] = storage;
        compound_indices.push_back(id);
        fast_signature.add(id);
    }
}

Archetype::~Archetype() {

    //Remove all the compounds.
    for(size_t compound_index : compound_indices) {
        delete compound[compound_index];
        compound[compound_index] = nullptr;
    }
}

void Archetype::remove_entity(Entity entity, Signal<Entity>* on_destroy_signals) {

    //since we check if the entity exist in the universe, it should exist here too.
    //therefore, we don't need to check again inside this Archetype
    size_t offset = entity_to_offset[entity];

    //we are at the end
    if(offset == offset_to_entity.size() - 1) {
        //remove all the components and swap end components
        for(size_t id : compound_indices) {
            compound[id]->remove_end();
            if(on_destroy_signals) {
                on_destroy_signals[id].emit(entity);
            }
        }
        entity_to_offset.erase(entity);
        offset_to_entity.pop_back();

    } else {

        //remove all the components and swap end components
        for(size_t id : compound_indices) {
            compound[id]->move_from_end(offset);
            compound[id]->remove_end();
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
}

void Archetype::insert_entity(Entity entity) {
    size_t new_offset = offset_to_entity.size();
    entity_to_offset.emplace(entity, new_offset);
    offset_to_entity.push_back(entity);

    /*for(size_t id : compound_indices) {
        compound[id]->insert_default_end();
    }*/
}

ArchetypeSignature Archetype::get_archetype_signature() {
    ArchetypeSignature signature;

    //remove all the components and swap end components
    for(size_t id : compound_indices) {
        signature.add(id, compound[id]->get_component_size());
    }

    return signature;
}

FastSignature Archetype::get_fast_signature() {
    return fast_signature;
}

//signature functions
size_t ArchetypeSignature::count() {
    return m_count;
}

void ArchetypeSignature::add(size_t id, size_t size) {

    //assert(m_count < 32);

    m_ids[m_count] = id;
    m_sizes[m_count] = size;
    m_count++;
}

bool ArchetypeSignature::operator==(ArchetypeSignature& rhs) {

    if(m_count != rhs.m_count) {
        return false;
    }

    for(size_t self = 0; self < m_count; self++) {

        bool exist = false;
        for(size_t other = 0; other < rhs.m_count; other++) {
            if(m_ids[self] == rhs.m_ids[other]) {
                exist = true;
                break;
            }
        }

        if(!exist) {
            return false;
        }
    }

    return true;
}

bool ArchetypeSignature::contains(ArchetypeSignature& other) {

    if(m_count < other.m_count) {
        return false;
    }

    for(size_t o = 0; o < other.m_count; o++) {

        bool exist = false;
        for(size_t s = 0; s < m_count; s++) {
            if(m_ids[s] == other.m_ids[o]) {
                exist = true;
                break;
            }
        }

        if(!exist) {
            return false;
        }
    }

    return true;
}