#include <iostream>
#include <bunshi.h>

int main() {

    size_t float_id = bunshi::TypeId::from<float>();
    size_t double_id = bunshi::TypeId::from<double>();

    std::cout << double_id << "\n";

    return 0;
}