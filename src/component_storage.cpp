#include "component_storage.h"
#include <string>

using namespace bunshi;

ComponentStorage::ComponentStorage(size_t a_component_size) {
    component_size = a_component_size;
    size = a_component_size * 10; //default size is to atleast make sure 10 components fits inside
    data = (uint8_t*)malloc(size);
    component_count = 0;
}

ComponentStorage::~ComponentStorage() {
    free(data);
}

void* ComponentStorage::get_component_pointer(size_t offset) {
    return data + offset*component_size;
}

void ComponentStorage::copy_end(size_t desination_offset) {
    memcpy(data + desination_offset*component_size, data + component_count*component_size, component_size);
}

void ComponentStorage::insert_end(void* pointer) {

    //if we are trying to overwrite the data, resize it before copying
    if(component_count * component_size > size) {
        resize();
    }
    memcpy(data + component_count * component_size, (uint8_t*)pointer, component_size);
    component_count++;
}

void ComponentStorage::remove_end() {
    component_count--;
}

//private
void ComponentStorage::resize() {
    if(data) {
        uint8_t* new_data = (uint8_t*)malloc(size * 2);
        memcpy(new_data, data, size);
        size *= 2;
        free(data);
        data = new_data;
    }
}