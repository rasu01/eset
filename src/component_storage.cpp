#include "component_storage.h"
#include <string>

using namespace bunshi;

ComponentStorage::ComponentStorage(size_t a_component_size, size_t a_start_size) {
    component_size = a_component_size;
    size = a_start_size;
    data = (uint8_t*)malloc(size);
}

ComponentStorage::~ComponentStorage() {
    free(data);
}

void* ComponentStorage::get_component_pointer(size_t offset) {
    return 0;
}

void ComponentStorage::resize() {
    if(data) {
        uint8_t* new_data = (uint8_t*)malloc(size * 2);
        memcpy(new_data, data, size);
        size *= 2;
        free(data);
        data = new_data;
    }
}