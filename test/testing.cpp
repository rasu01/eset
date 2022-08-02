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
    float time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count();

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

    for(int i = 0; i < 1000; i++) {
        bunshi::Entity entity = universe.create();
        Position pos = {42.0, 0.0};
        Unit unit = {"yo"};
        universe.insert_component<Position>(entity, pos);
        universe.insert_component<Unit>(entity, unit);
    }

    size_t count = 0;
    for(auto [entity_id, pos, unit] : universe.iterator<Position, Unit>()) {
        if(pos.x == 42.0) {
            count++;
        }
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
        universe.insert_component<float>(entity, 10.0);
        universe.insert_component<TestComponent>(entity, tc);
    }

    return universe.get_component<TestComponent>(entity)->string == "Testing to construct a new string here!!";
}

struct TestComponent {
    static int cnt;
    TestComponent() {cnt++;}
    TestComponent& operator=(const TestComponent& other) {cnt++; return *this;};
    TestComponent(const TestComponent& other) {cnt++;};
    ~TestComponent() {cnt--;}
};
int TestComponent::cnt = 0;

bool test_destructor() {

    {
        bunshi::Universe universe;
        bunshi::Entity entity = universe.create();
        universe.insert_component<TestComponent>(entity, TestComponent());
        universe.insert_component<float>(entity, 0.0f);
        //<--- should delete all the components when the universe goes out of scope!
    }

    return TestComponent::cnt == 0;
}

void test(bunshi::Entity entity) {
    std::cout << "hm\n";
}

bool test_signals_on_destroy() {

    bunshi::Universe universe;
    bunshi::Entity entity = universe.create();
    universe.insert_component<float>(entity, 0.0f);
    universe.insert_component<int>(entity, 1);
    universe.connect_on_remove<float>(test);
    universe.disconnect_on_remove<float>(test);
    universe.remove(entity);

    return true;
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
    run_test(test_destructor, "Running destructor when removing entity");
    run_test(test_signals_on_destroy, "Signals on destroy");

    //print all the results
    std::cout << "Tests: " << total << "; Passes: " << successes << "; Fails: " << fails << "\n";
    return 0;
}