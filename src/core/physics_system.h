#pragma once
#include <raylib.h>
#include <vector>

class Player;
class TileMap;
class Destructible;

class PhysicsSystem
{
public:
    static Vector2 ResolvePlayerMovement(
        const Player& player,
        Vector2 oldPos,
        const TileMap& tileMap,
        const std::vector<Destructible>& destructibles
    );
};
