# cpp-OOP-game-engine

This is a simple 2D game engine implemented in an OOP framework. This is in contrast to our [ECS engine](https://github.com/CLPolo/cpp-ECS-game-engine). Both engines are a part of an exercise in learning game engine development. As first steps in this effort, we've turned to SFML for window creation, input handling, drawing etc.

This project was built in collaboration with [FaiazBN](https://github.com/FaiazBN)

## Repo Structure

```
.
├── bolo
└── engine
```
 - Engine, naturally, contains all of the engine functionality/implementation
 - bolo contains a simple copy of the Apple ][ game, BOLO, built with the engine API

## Thoughts on the OOP model

This was the first of the two engines to be implemented. Given its ubiquity, the OOP pattern is the more familiar of the two. Having worked previously with C# in Unity, it was a natural thing to, for example, create an abstract "GameObject" class and an inheriting "CollisionObject". 

When compared to the ECS implementation, it's easy to see that the OOP design pattern requires some kind of external set of guiding principles to manage the growth of the engine as different object/data types are created. For this system to be extensible woulr require either a very clear, broad, and complete plan at the outset, or at least some idea of the top few levels of an object taxonomy.

Overall I much prefer the ECS design pattern, but it's clear why engines like Unity use an OOP structure: it seems to be much more extensible, as well as a much more intuitive way of thinking about data structures and operating over them. 
