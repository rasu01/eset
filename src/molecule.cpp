#include "molecule.h"
#include <cassert>
#include <iostream>

using namespace bunshi;

Molecule::Molecule(MoleculeSignature& signature) {

    for(size_t i = 0; i < signature.count(); i++) {

        size_t id = signature.m_ids[i];
        size_t size = signature.m_sizes[i];
        compound[id] = ComponentStorage(size);
        compound_indices.push_back(id);
        fast_signature.add(id);
    }
}

void Molecule::remove_entity(Entity entity) {

    //since we check if the entity exist in the universe, it should exist here too.
    //therefore, we don't need to check again inside this molecule
    size_t offset = entity_to_offset[entity];

    //remove all the components and swap end components
    for(size_t id : compound_indices) {
        compound[id].copy_end(offset);
        compound[id].remove_end();
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

void Molecule::insert_entity(Entity entity) {
    size_t new_offset = offset_to_entity.size();
    entity_to_offset.emplace(entity, new_offset);
    offset_to_entity.push_back(entity);

    for(size_t id : compound_indices) {
        compound[id].insert_default_end();
    }
}

MoleculeSignature Molecule::get_molecule_signature() {
    MoleculeSignature signature;

    //remove all the components and swap end components
    for(size_t id : compound_indices) {
        signature.add(id, compound[id].get_component_size());
    }

    return signature;
}

FastSignature Molecule::get_fast_signature() {
    return fast_signature;
}

//signature functions
size_t MoleculeSignature::count() {
    return m_count;
}

void MoleculeSignature::add(size_t id, size_t size) {

    //assert(m_count < 32);

    m_ids[m_count] = id;
    m_sizes[m_count] = size;
    m_count++;
}

bool MoleculeSignature::operator==(MoleculeSignature& rhs) {

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

bool MoleculeSignature::contains(MoleculeSignature& other) {

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