#include "component_storage.h"

using namespace eset;

uint16_t BaseStorage::m_storage_count = 0;

uint16_t BaseStorage::storage_id() {
    return m_storage_id;
}

uint64_t BaseStorage::storage_offset_identifier(uint64_t offset) {
    return ((uint64_t)storage_id() << 48) | offset;
}