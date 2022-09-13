# eset
Eset is supposed to be short for "Entity Set" and is an Entity Component System written in C++. It's goal is to be fast and simple!

The actual ECS is using archetypes, to store different types of entities that shares the same unique signature.
A set contains all the entities that may have different archetypes.
The set is the main class in this library that allows you to for example create new entities and either add, delete or get components from them.
