#pragma once
#include <raylib.h>

class Player;
class TileMap;
class EntityManager;

class PhysicsSystem
{
public:
    static Vector2 ResolvePlayerMovement(
        const Player& player,
        Vector2 oldPos,
        const TileMap& tileMap,
        const EntityManager& entityManager
    );
};
