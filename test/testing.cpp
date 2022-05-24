#include <iostream>
#include <typeinfo>
#include <bunshi.h>

int main() {

    //bunshi::Universe universe;

    std::cout << typeid(bunshi::Entity).hash_code() << ", " << typeid(size_t).hash_code() << "\n";

    return 0;
}