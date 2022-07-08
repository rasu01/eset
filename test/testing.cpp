#include <iostream>
#include <typeinfo>
#include <string>
#include <chrono>
#include <functional>
#include <stdlib.h>
#include <bunshi.h>

/* FOREGROUND */
#define RESET  "\x1B[0m"
#define RED  "\x1B[31m"
#define GREEN  "\x1B[32m"

#define RED_TEXT(x) RED x RESET
#define GREEN_TEXT(x) GREEN x RESET

static size_t total = 0;
static size_t successes = 0;
static size_t fails = 0;

void run_test(std::function<bool()> test_function, std::string&& test_name) {

    auto start = std::chrono::high_resolution_clock::now();
    bool result = test_function();
    double time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count();

    std::cout << "[" << ++total << "] " << test_name;
    if(result) {
        std::cout << GREEN_TEXT(" passed");
        successes++;
    } else {
        std::cout << RED_TEXT(" failed");
        fails++;
    }
    std::cout << " and took " << time << " microseconds.\n";
}

bool test_single_non_existing_removal() {
    bunshi::Universe universe;
    bunshi::Entity entity = 1;

    //remove should be false
    return !universe.remove(entity);
}

bool test_create_entity() {
    bunshi::Universe universe;
    bunshi::Entity entity = universe.create();
    return entity == 0;
}

bool test_entity_and_size_t_id() {

    //since a bunshi::Entity is a size_t underneath,
    //this expression should return true
    return typeid(bunshi::Entity) == typeid(size_t);
}

bool test_created_exist() {
    bunshi::Universe universe;
    bunshi::Entity entity = universe.create();
    return universe.exist(entity);
}

bool test_created_remove() {
    bunshi::Universe universe;
    bunshi::Entity entity = universe.create();
    return universe.remove(entity);
}

bool test_component_get() {
    bunshi::Universe universe;
    bunshi::Entity entity = universe.create();

    float component = 10001.0;
    size_t component2 = 111112129;
    universe.insert_component<float>(entity, component);
    universe.insert_component<size_t>(entity, component2);
    
    float number = *universe.get_component<float>(entity);

    std::cout << number << " create\n";
    return false;// number == 1000.0;
}

int main() {

    run_test(test_single_non_existing_removal, "Removing a nonexisting entity");
    run_test(test_create_entity, "Create entity id check");
    run_test(test_created_exist, "Create entity exist check");
    run_test(test_created_remove, "Create entity and remove");
    run_test(test_entity_and_size_t_id, "size_t and bunshi::Entity type");
    run_test(test_component_get, "Insert and get a component from an Entity");

    //print all the results
    std::cout << "Tests: " << total << "; Passes: " << successes << "; Fails: " << fails << "\n";
    return 0;
}