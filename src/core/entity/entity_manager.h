#pragma once
#include "npc.h"
#include "enemy.h"
#include "destructible.h"
#include "ground_pickup.h"
#include "spatial_grid.h"
#include <vector>
#include <raylib.h>

struct Portal
{
    Rectangle rect;
    std::string targetMap;
    Vector2 targetSpawn;
    std::string name;
    std::string portalId;       /* ID unique de ce portail (ex: "porte_maison", "sortie_maison") */
    std::string targetPortalId; /* ID du portail cible sur la carte de destination */
};

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
    void AddPortal(Portal&& portal);

    /* Actualise la grille spatiale avec les entités statiques (A appeler après le LoadMap) */
    void BuildSpatialGrid();

    /* Accesseurs pour les sous-systèmes (Combat, Physics, Dialogue) */
    [[nodiscard]] std::vector<Npc>& GetNpcs() { return m_npcs; }
    [[nodiscard]] const std::vector<Npc>& GetNpcs() const { return m_npcs; }

    [[nodiscard]] std::vector<Enemy>& GetEnemies() { return m_enemies; }
    [[nodiscard]] const std::vector<Enemy>& GetEnemies() const { return m_enemies; }

    [[nodiscard]] std::vector<Destructible>& GetDestructibles() { return m_destructibles; }
    [[nodiscard]] const std::vector<Destructible>& GetDestructibles() const { return m_destructibles; }

    [[nodiscard]] std::vector<GroundPickup>& GetPickups() { return m_pickups; }
    [[nodiscard]] const std::vector<GroundPickup>& GetPickups() const { return m_pickups; }

    [[nodiscard]] std::vector<Portal>& GetPortals() { return m_portals; }
    [[nodiscard]] const std::vector<Portal>& GetPortals() const { return m_portals; }

    [[nodiscard]] const SpatialGrid& GetSpatialGrid() const { return m_spatialGrid; }

private:
    std::vector<Npc> m_npcs;
    std::vector<Enemy> m_enemies;
    std::vector<Destructible> m_destructibles;
    std::vector<GroundPickup> m_pickups;
    std::vector<Portal> m_portals;

    SpatialGrid m_spatialGrid;
};
