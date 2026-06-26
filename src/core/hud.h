#pragma once
#include "player.h"
#include "map/tile_map.h"
#include "destructible.h"
#include "ground_pickup.h"
#include <raylib.h>
#include <string>
#include <vector>

class HUD
{
public:
    HUD();

    void Update(float deltaTime);
    void Draw(const Player& player, const TileMap& tileMap, 
              const std::vector<Destructible>& destructibles, 
              const std::vector<GroundPickup>& pickups) const;

    void TriggerNotification(const std::string& text, float duration = 3.0f);
    void Reset();

private:
    void DrawHearts(const Player& player) const;
    void DrawHeart(float x, float y, float size, float fillPercent) const;
    void DrawMagicBar(const Player& player) const;
    void DrawEquippedItems(const Player& player) const;
    void DrawEquippedItemBox(float x, float y, const std::string& label, const std::string& itemId, const std::string& keyName, const Player& player) const;
    void DrawRupeePouch(const Player& player) const;
    void DrawMiniMap(const TileMap& tileMap, 
                     const std::vector<Destructible>& destructibles, 
                     const std::vector<GroundPickup>& pickups,
                     const Player& player) const;

    std::string m_notificationText;
    float m_notificationTimer;
};
