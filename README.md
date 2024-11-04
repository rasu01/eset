# eset
([日本語版はこちら](READMEJP.md))<br/>
eset is an abbreviation for "entity set". This project's most important points are simplicity and speed.
In order to iterate over a large amount of entities as fast as possible, each entity type has it's own archetype. Inside this archetype, only entities that has the same matching type as the archetype are included. They are also tightly packed in memory.
Components can be added to every entity and which archetype a specific entity belongs to is decided based on which components it has. Creating new entities, adding or removing(TODO) components will automatically move it to another archetype.
Any struct, class or base-type is valid as a component.
Right before iterating, only entities that has the specified components included in them will be iterated over.
Because the entities that exist in every archetype have the same type, only the archetypes need to be checked.

# How to iterate
Specifty desired components that exist inside the entities and iterate over them like a normal for loop. This program will output the components' content. The entities that does not contain the specified components, will be ignored.
```C++
//declaration of components
class ClassComponent {
  float a;
  float b;
};

struct StructComponent {
  int c;
  int d;
};

//creating the set
eset::Set set;

/* Here we add entities and their components */

//Iterate!
for(auto [entity_id, class_comp, struct_comp] : set.iterator<ClassComponent, StructComponent>()) {
  std::cout << class_comp.a << ", " << class_comp.b << "; " << struct_comp.c << ", " << struct_comp.d << "\n";
}
```
