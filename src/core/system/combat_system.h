#pragma once
#include <raylib.h>

class Player;
class EntityManager;
struct BoomerangProjectile;
class TileMap;
class HUD;

namespace CombatSystem
{
    void ResolvePlayerSwordAttacks(
        Player& player,
        EntityManager& entityManager
    );

    void ResolvePlayerBoomerangAttacks(
        Player& player,
        BoomerangProjectile& boomerang,
        EntityManager& entityManager,
        const TileMap& tileMap,
        float deltaTime
    );

    void ResolveEnemyDamageToPlayer(
        Player& player,
        EntityManager& entityManager,
        float& playerHitCooldown,
        HUD& hud,
        float deltaTime
    );
}
