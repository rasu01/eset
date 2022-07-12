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
    return entity == 1;
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

    universe.insert_component<float>(entity, 1024.0);
    universe.insert_component<size_t>(entity, 512);
    
    float decimal = *universe.get_component<float>(entity);    
    size_t number = *universe.get_component<size_t>(entity);

    return number == 512 && decimal == 1024.0;
}

bool test_iteration() {

    bunshi::Universe universe;

    struct Position {
        float x;
        float y;
    };

    struct Unit {
        std::string name;
    };

    for(int i = 0; i < 1000000; i++) {
        bunshi::Entity ent = universe.create();
        universe.insert_component<float>(ent, 1.0f);
        universe.insert_component<std::string>(ent, "hej");
        universe.insert_component<double>(ent, 1.0);
        universe.insert_component<size_t>(ent, 100);
        universe.insert_component<bool>(ent, false);
    }

    for(int i = 0; i < 1; i++) {
        bunshi::Entity entity = universe.create();
        Position pos = {0.0, 0.0};
        Unit unit = {"yo"};
        universe.insert_component<Position>(entity, pos);
        universe.insert_component<Unit>(entity, unit);
        universe.insert_component<float>(entity, 1.0);
    }

    size_t count = 0;
    while(1) {
        auto start = std::chrono::steady_clock::now();
        count = 0;
        for(auto [entity_id, pos, unit] : universe.iterator<Position, Unit>()) {
            if(pos.x == 0.0) {
                count++;
            }
        }
        double time = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - start).count();
        std::cout << "time " << time << ": " << count << " nanoseconds\n";
    }

    return count == 1000;
}

bool test_null() {
    return bunshi::null == 0;
}

bool test_copy() {

    struct TestComponent {
        std::string string;
    };

    bunshi::Universe universe;
    bunshi::Entity entity = universe.create();
    {
        TestComponent tc;
        tc.string = "Testing to construct a new string here!!";
        universe.insert_component<TestComponent>(entity, tc);
        universe.insert_component<float>(entity, 10.0);
    }

    return universe.get_component<TestComponent>(entity)->string == "Testing to construct a new string here!!";
}

int main() {

    run_test(test_single_non_existing_removal, "Removing a nonexisting entity");
    run_test(test_create_entity, "Create entity id check");
    run_test(test_created_exist, "Create entity exist check");
    run_test(test_component_get, "Insert and get a component from an Entity");
    run_test(test_created_remove, "Create entity and remove");
    run_test(test_entity_and_size_t_id, "size_t and bunshi::Entity type");
    run_test(test_iteration, "Iteration");
    run_test(test_null, "Null");
    run_test(test_copy, "Copy with underlying data");

    //print all the results
    std::cout << "Tests: " << total << "; Passes: " << successes << "; Fails: " << fails << "\n";
    return 0;
}