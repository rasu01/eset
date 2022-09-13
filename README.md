# eset
Eset is supposed to be short for "Entity Set" and is an Entity Component System written in C++. It's goal is to be fast and simple!

The actual ECS is using archetypes, to store different types of entities that shares the same unique signature.
A universe is just a set filled with all these entities that may have different archetypes. 
This universe is the main class in this library that allows you to for example create new entities and either add, delete or get components from them.
