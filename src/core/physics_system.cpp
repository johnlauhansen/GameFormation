#include "physics_system.h"
#include "player.h"
#include "map/tile_map.h"
#include "destructible.h"

Vector2 PhysicsSystem::ResolvePlayerMovement(
    const Player& player,
    Vector2 oldPos,
    const TileMap& tileMap,
    const std::vector<Destructible>& destructibles
)
{
    Vector2 currentPos = player.GetPosition();
    Rectangle colRect = player.GetCollisionRect();

    /* 1. Résoudre d'abord la collision contre la grille (TileMap) */
    if (tileMap.CheckCollision(colRect))
    {
        currentPos = tileMap.ResolveCollision(currentPos, oldPos, colRect.width, colRect.height);
    }

    /* 2. Résoudre la collision glissante contre les objets destructibles solides actifs */
    Vector2 finalPos = currentPos;
    
    // Créer une copie temporaire du rectangle de collision à l'ancienne position Y, nouvelle position X
    Rectangle testRectX = colRect;
    testRectX.x = currentPos.x - colRect.width / 2.0f;
    testRectX.y = oldPos.y - colRect.height / 2.0f;

    bool collideX = false;
    for (const auto& dest : destructibles)
    {
        if (dest.IsAlive() && CheckCollisionRecs(testRectX, dest.GetCollisionRect()))
        {
            collideX = true;
            break;
        }
    }
    if (collideX)
    {
        finalPos.x = oldPos.x;
    }

    // Créer une copie temporaire du rectangle de collision à la nouvelle position X, nouvelle position Y
    Rectangle testRectY = colRect;
    testRectY.x = finalPos.x - colRect.width / 2.0f;
    testRectY.y = currentPos.y - colRect.height / 2.0f;

    bool collideY = false;
    for (const auto& dest : destructibles)
    {
        if (dest.IsAlive() && CheckCollisionRecs(testRectY, dest.GetCollisionRect()))
        {
            collideY = true;
            break;
        }
    }
    if (collideY)
    {
        finalPos.y = oldPos.y;
    }

    return finalPos;
}
