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

size_t signal_delete_count = 0;
void test(eset::Entity entity) {
    signal_delete_count++;
}

bool test_signals_on_destroy() {

    eset::Set set;
    set.connect_on_remove<float>(test);

    eset::Entity entity = set.create();
    set.insert_component<float>(entity, 0.0f);
    set.remove(entity);

    entity = set.create();
    set.insert_component<float>(entity, 0.0f);
    
    eset::Entity entity2 = set.create();
    set.insert_component<float>(entity2, 0.0f);

    set.remove(entity);
    set.disconnect_on_remove<float>(test);
    set.remove(entity2);

    return signal_delete_count == 2;
}

bool test_references_validity() {

    bool test_return = true;
    void* old_pointer = nullptr;

    eset::Set set;
    eset::Entity entity = set.create();
    set.insert_component<float>(entity, 10.0f);

    eset::Ref<float> ref = set.get<float>(entity);
    test_return = test_return && ref.valid() && *ref.get() == 10.0f && ref.get() != nullptr && ref.get() != old_pointer && ref.entity() == entity; old_pointer = ref.get();
    set.insert_component<int>(entity, 1);
    test_return = test_return && ref.valid() && *ref.get() == 10.0f && ref.get() != nullptr && ref.get() != old_pointer && ref.entity() == entity; old_pointer = ref.get();

    //here the ref.get() shouldn't change
    {
        eset::Ref<float> ref2 = set.get<float>(entity);
        test_return = test_return && ref2.valid() && *ref2.get() == 10.0f && ref2.get() != nullptr && ref2.get() == old_pointer && ref2.entity() == entity;
    }

    eset::Entity entity2 = set.create();
    set.insert_component<float>(entity2, 20.0f);
    eset::Ref<float> ref3 = set.get<float>(entity2);
    test_return = test_return && ref.valid() && *ref.get() == 10.0f && ref.get() != nullptr && ref.entity() == entity;
    test_return = test_return && ref3.valid() && *ref3.get() == 20.0f && ref3.get() != nullptr && ref3.entity() == entity2;

    set.remove(entity);
    test_return = test_return && !ref.valid() && ref.entity() == eset::null;

    set.remove(entity2);
    test_return = test_return && !ref3.valid() && ref3.entity() == eset::null;

    return test_return;
}

bool test_reference_count() {
    eset::Set set;
    eset::Entity entity = set.create();
    set.insert_component<float>(entity, 10.0f);

    eset::Ref<float> ref1 = set.get<float>(entity);
    {
        eset::Ref<float> ref2 = set.get<float>(entity);
        eset::Ref<float> ref3 = ref2;
        ref3 = ref1;
        ref2 = ref3;
    }
    eset::Ref<float> ref4 = std::move(ref1);

    return ref4.reference_count() == 1 && *ref4.get() == 10.0f;
}

bool test_reference_set_pointer() {

    bool test_return = true;
    eset::Ref<float> ref;
    {
        eset::Set set;
        eset::Entity entity = set.create();
        set.insert_component<float>(entity, 10.0f);
        ref = set.get<float>(entity);
        test_return = test_return && ref.set() != nullptr;
    }
    test_return = test_return && ref.reference_count() == 1 && ref.set() == nullptr;

    return test_return;
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
    run_test(test_references_validity, "Reference validity");
    run_test(test_reference_count, "Reference count");
    run_test(test_reference_set_pointer, "Reference set pointer");

    //print all the results
    std::cout << "Tests: " << total << "; Passes: " << successes << "; Fails: " << fails << "\n";
    return 0;
}