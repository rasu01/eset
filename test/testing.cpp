#include <iostream>
#include <typeinfo>
#include <string>
#include <chrono>
#include <functional>
#include <stdlib.h>
#include <eset.h>

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
    eset::Set set;
    eset::Entity entity = 1;

    //remove should be false
    return !set.remove(entity);
}

bool test_create_entity() {
    eset::Set set;
    eset::Entity entity = set.create();
    return entity == 1;
}

bool test_entity_and_size_t_id() {

    //since a eset::Entity is a size_t underneath,
    //this expression should return true
    return typeid(eset::Entity) == typeid(size_t);
}

bool test_created_exist() {
    eset::Set set;
    eset::Entity entity = set.create();
    return set.exist(entity);
}

bool test_created_remove() {
    eset::Set set;
    eset::Entity entity = set.create();
    return set.remove(entity);
}

bool test_component_get() {
    eset::Set set;
    eset::Entity entity = set.create();

    set.insert_component<float>(entity, 1024.0);
    set.insert_component<size_t>(entity, 512);
    
    float decimal = *set.get_component<float>(entity);    
    size_t number = *set.get_component<size_t>(entity);

    return number == 512 && decimal == 1024.0;
}

bool test_iteration() {

    eset::Set set;

    struct Position {
        float x;
        float y;
    };

    struct Unit {
        std::string name;
    };

    for(int i = 0; i < 1000; i++) {
        eset::Entity entity = set.create();
        Position pos = {42.0, 0.0};
        Unit unit = {"yo"};
        set.insert_component<Position>(entity, pos);
        set.insert_component<Unit>(entity, unit);
    }

    size_t count = 0;
    for(auto [entity_id, pos, unit] : set.iterator<Position, Unit>()) {
        if(pos.x == 42.0) {
            count++;
        }
    }

    return count == 1000;
}

bool test_null() {
    return eset::null == 0;
}

bool test_copy() {

    struct TestComponent {
        std::string string;
    };

    eset::Set set;
    eset::Entity entity = set.create();
    {
        TestComponent tc;
        tc.string = "Testing to construct a new string here!!";
        set.insert_component<float>(entity, 10.0);
        set.insert_component<TestComponent>(entity, tc);
    }

    return set.get_component<TestComponent>(entity)->string == "Testing to construct a new string here!!";
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
        eset::Set set;
        eset::Entity entity = set.create();
        set.insert_component<TestComponent>(entity, TestComponent());
        set.insert_component<float>(entity, 0.0f);
        //<--- should delete all the components when the set goes out of scope!
    }

    return TestComponent::cnt == 0;
}

void test(eset::Entity entity) {
    std::cout << "hm\n";
}

bool test_signals_on_destroy() {

    eset::Set set;
    eset::Entity entity = set.create();
    set.insert_component<float>(entity, 0.0f);
    set.insert_component<int>(entity, 1);
    set.connect_on_remove<float>(test);
    set.disconnect_on_remove<float>(test);
    set.remove(entity);

    return true;
}

int main() {

    run_test(test_single_non_existing_removal, "Removing a nonexisting entity");
    run_test(test_create_entity, "Create entity id check");
    run_test(test_created_exist, "Create entity exist check");
    run_test(test_component_get, "Insert and get a component from an Entity");
    run_test(test_created_remove, "Create entity and remove");
    run_test(test_entity_and_size_t_id, "size_t and eset::Entity type");
    run_test(test_iteration, "Iteration");
    run_test(test_null, "Null");
    run_test(test_copy, "Copy with underlying data");
    run_test(test_destructor, "Running destructor when removing entity");
    run_test(test_signals_on_destroy, "Signals on destroy");

    //print all the results
    std::cout << "Tests: " << total << "; Passes: " << successes << "; Fails: " << fails << "\n";
    return 0;
}