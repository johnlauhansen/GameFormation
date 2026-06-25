#pragma once
#include "player.h"
#include "destructible.h"
#include "map/tile_map.h"
#include <raylib.h>
#include <vector>
#include <string>

struct GroundPickup
{
    std::string itemId;       /* "sword", "boomerang" */
    std::string name;         /* Nom affiché lors de la collecte */
    Vector2 position;
    bool active = true;
};

class GameWorld
{
public:
    GameWorld();

    void Update(float deltaTime);
    void Draw() const;
    void Reset();

    [[nodiscard]] Player& GetPlayer()
    {
        return m_player;
    }

    [[nodiscard]] const Player& GetPlayer() const
    {
        return m_player;
    }

private:
    TileMap m_tileMap;
    Player m_player;
    Camera2D m_camera;

    std::vector<GroundPickup> m_pickups;
    std::vector<Destructible> m_destructibles;
    
    /* Notification d'objet collecté temporaire */
    std::string m_notificationText;
    float m_notificationTimer;
};
