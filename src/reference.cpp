#include "component_storage.h"
#include "reference.h"
#include "set.h"

using namespace eset;

Set* BaseReference::set() {
    return m_reference_data->m_set;
}

Entity BaseReference::entity() {
    return m_reference_data->m_entity;
}

size_t BaseReference::reference_count() {
    return m_reference_data->m_reference_count;
}

bool BaseReference::valid() {
    if(m_reference_data) {
        return m_reference_data->m_storage != nullptr;
    } else {
        return false;
    }
}

void BaseReference::let_set_delete_reference_data() {
    m_reference_data->m_set->delete_reference_pointer(m_reference_data);
}