# bunshi
This is an Entity Component System written in C++, where the goal is to make it fast and simple!

The word Bunshi, or 分子 as it is written in Japanese, means "molecule" in English. 
The actual ECS is using Archetypes, to store different types of entities that shares the same signature.
Therefore, an Archetype here, is the same thing as a "molecule" type(another word for this is "chemical compound", but it is a rather long word, which is why I didn't name this library after it). 
A universe is just a collection filled with all these entities that may have different molecule types. 
This universe is the main class in this system that allows you to for example create new entities and either add, delete or get components from them.

# examples
