#pragma once
#include "npc.h"
#include "enemy.h"
#include "destructible.h"
#include "ground_pickup.h"
#include <vector>
#include <raylib.h>

class TileMap;

class EntityManager
{
public:
    EntityManager() = default;

    void Update(float deltaTime, const TileMap& tileMap, const Vector2& playerPos);
    void Draw() const;
    void Clear();

    /* Ajout d'entités */
    void AddNpc(Npc&& npc);
    void AddEnemy(Enemy&& enemy);
    void AddDestructible(Destructible&& destructible);
    void AddPickup(GroundPickup&& pickup);

    /* Accesseurs pour les sous-systèmes (Combat, Physics, Dialogue) */
    [[nodiscard]] std::vector<Npc>& GetNpcs() { return m_npcs; }
    [[nodiscard]] const std::vector<Npc>& GetNpcs() const { return m_npcs; }

    [[nodiscard]] std::vector<Enemy>& GetEnemies() { return m_enemies; }
    [[nodiscard]] const std::vector<Enemy>& GetEnemies() const { return m_enemies; }

    [[nodiscard]] std::vector<Destructible>& GetDestructibles() { return m_destructibles; }
    [[nodiscard]] const std::vector<Destructible>& GetDestructibles() const { return m_destructibles; }

    [[nodiscard]] std::vector<GroundPickup>& GetPickups() { return m_pickups; }
    [[nodiscard]] const std::vector<GroundPickup>& GetPickups() const { return m_pickups; }

private:
    std::vector<Npc> m_npcs;
    std::vector<Enemy> m_enemies;
    std::vector<Destructible> m_destructibles;
    std::vector<GroundPickup> m_pickups;
};
