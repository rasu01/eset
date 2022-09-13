# bunshi
This is an Entity Component System written in C++, where the goal is to make it fast and simple!

The actual ECS is using Archetypes, to store different types of entities that shares the same unique signature.
A universe is just a collection filled with all these entities that may have different archetypes. 
This universe is the main class in this system that allows you to for example create new entities and either add, delete or get components from them.
