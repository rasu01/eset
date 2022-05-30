#include <iostream>
#include <typeinfo>
#include <bunshi.h>

int main() {

    bunshi::Universe universe;

    bunshi::Entity entity = 1;
    if(!universe.exist(entity)) {
        std::cout << "Passed!\n";
    } else {
        std::cout << "Failed!\n";
    }

    std::cout << typeid(bunshi::Entity).hash_code() << ", " << typeid(size_t).hash_code() << "\n";

    return 0;
}