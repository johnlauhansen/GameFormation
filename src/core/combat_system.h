#pragma once
#include <raylib.h>
#include <vector>

class Player;
class Enemy;
class Destructible;
struct GroundPickup;
struct BoomerangProjectile;
class TileMap;
class HUD;
class Npc;

namespace CombatSystem
{
    void ResolvePlayerSwordAttacks(
        Player& player,
        std::vector<Enemy>& enemies,
        std::vector<Destructible>& destructibles,
        std::vector<GroundPickup>& pickups,
        std::vector<Npc>& npcs
    );

    void ResolvePlayerBoomerangAttacks(
        Player& player,
        BoomerangProjectile& boomerang,
        std::vector<Enemy>& enemies,
        std::vector<Destructible>& destructibles,
        std::vector<GroundPickup>& pickups,
        std::vector<Npc>& npcs,
        const TileMap& tileMap,
        float deltaTime
    );

    void ResolveEnemyDamageToPlayer(
        Player& player,
        std::vector<Enemy>& enemies,
        float& playerHitCooldown,
        HUD& hud,
        float deltaTime
    );
}
