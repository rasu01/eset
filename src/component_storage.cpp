#include "component_storage.h"
#include <cstring>

using namespace bunshi;

ComponentStorage::ComponentStorage(size_t a_component_size) {
    component_size = a_component_size;
    size = a_component_size * 10; //default size is to atleast make sure 10 components fits inside
    data = (uint8_t*)malloc(size);
    component_count = 0;
}

ComponentStorage::ComponentStorage(ComponentStorage& ot) {
    component_size = ot.component_size;
    size = ot.size;
    if(size != 0) {
        data = (uint8_t*)malloc(size);
        memcpy(data, ot.data, size);
    }
    component_count = ot.component_count;
}

ComponentStorage::ComponentStorage(ComponentStorage&& ot) {
    component_size = ot.component_size; ot.component_size = 0;
    size = ot.size; ot.size = 0;
    data = ot.data; ot.data = nullptr;
    component_count = ot.component_count; ot.component_count = 0;
}

ComponentStorage& ComponentStorage::operator=(const ComponentStorage& ot) {
    component_size = ot.component_size;
    size = ot.size;
    if(size != 0) {
        data = (uint8_t*)malloc(size);
        memcpy(data, ot.data, size);
    }
    component_count = ot.component_count;
    return *this;
}

ComponentStorage::~ComponentStorage() {
    if(size != 0) {
        free(data); 
    }
}

void ComponentStorage::copy_end(size_t desination_offset) {
    memcpy(data + desination_offset*component_size, data + component_count*component_size, component_size);
}

void ComponentStorage::insert_end(void* pointer) {

    //if we are trying to overwrite the data, resize it before copying
    if((component_count+1) * component_size > size) {
        resize();
    }
    memcpy(data + component_count * component_size, (uint8_t*)pointer, component_size);
    component_count++;
}

void ComponentStorage::insert_default_end() {
    //if we are trying to overwrite the data, resize it before copying
    if((component_count+1) * component_size > size) {
        resize();
    }
    memset(data + component_count * component_size, 0, component_size);
    component_count++;
}

void ComponentStorage::remove_end() {
    component_count--;
}

size_t ComponentStorage::get_component_size() {
    return component_size;
}

void ComponentStorage::set_component(size_t offset, void* pointer) {
    memcpy(data + offset * component_size, (uint8_t*)pointer, component_size);
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