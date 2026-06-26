#include "entity_manager.h"

void EntityManager::Update(float deltaTime, const TileMap& tileMap, const Vector2& playerPos)
{
    for (auto& dest : m_destructibles)
    {
        dest.Update(deltaTime);
    }

    for (auto& npc : m_npcs)
    {
        npc.Update(deltaTime, tileMap);
    }

    for (auto& enemy : m_enemies)
    {
        enemy.Update(deltaTime, tileMap, playerPos);
    }
}

void EntityManager::Draw() const
{
    for (const auto& dest : m_destructibles)
    {
        dest.Draw();
    }

    for (const auto& npc : m_npcs)
    {
        npc.Draw();
    }

    for (const auto& enemy : m_enemies)
    {
        enemy.Draw();
    }

    for (const auto& pickup : m_pickups)
    {
        if (pickup.active)
        {
            if (pickup.itemId == "sword")
            {
                DrawLineEx({ pickup.position.x - 8, pickup.position.y + 8 }, { pickup.position.x + 8, pickup.position.y - 8 }, 3.5f, LIGHTGRAY);
                DrawLineEx({ pickup.position.x - 10, pickup.position.y + 10 }, { pickup.position.x - 6, pickup.position.y + 6 }, 4.0f, BROWN);
                DrawCircleV({ pickup.position.x + 8, pickup.position.y - 8 }, 2.5f, WHITE);
            }
            else if (pickup.itemId == "boomerang")
            {
                DrawCircleSector(pickup.position, 10.0f, 45.0f, 225.0f, 4, SKYBLUE);
                DrawCircleLinesV(pickup.position, 10.0f, WHITE);
            }
            else if (pickup.itemId == "rupee")
            {
                float rx = pickup.position.x;
                float ry = pickup.position.y;
                float rw = 10.0f;
                float rh = 16.0f;
                float rh2 = rh * 0.3f;
                Color color = GREEN;
                DrawRectangle((int)(rx - rw/2), (int)(ry - rh/2 + rh2), (int)rw, (int)(rh - 2*rh2), color);
                DrawTriangle({ rx - rw/2, ry - rh/2 + rh2 }, { rx + rw/2, ry - rh/2 + rh2 }, { rx, ry - rh/2 }, color);
                DrawTriangle({ rx, ry + rh/2 }, { rx + rw/2, ry + rh/2 - rh2 }, { rx - rw/2, ry + rh/2 - rh2 }, color);
                
                DrawLineEx({ rx, ry - rh/2 }, { rx - rw/2, ry - rh/2 + rh2 }, 1.0f, WHITE);
                DrawLineEx({ rx - rw/2, ry - rh/2 + rh2 }, { rx - rw/2, ry + rh/2 - rh2 }, 1.0f, WHITE);
                DrawLineEx({ rx - rw/2, ry + rh/2 - rh2 }, { rx, ry + rh/2 }, 1.0f, WHITE);
                DrawLineEx({ rx, ry + rh/2 }, { rx + rw/2, ry + rh/2 - rh2 }, 1.0f, WHITE);
                DrawLineEx({ rx + rw/2, ry + rh/2 - rh2 }, { rx + rw/2, ry - rh/2 + rh2 }, 1.0f, WHITE);
                DrawLineEx({ rx + rw/2, ry - rh/2 + rh2 }, { rx, ry - rh/2 }, 1.0f, WHITE);
            }
        }
    }
}

void EntityManager::Clear()
{
    m_npcs.clear();
    m_enemies.clear();
    m_destructibles.clear();
    m_pickups.clear();
}

void EntityManager::AddNpc(Npc&& npc)
{
    m_npcs.push_back(std::move(npc));
}

void EntityManager::AddEnemy(Enemy&& enemy)
{
    m_enemies.push_back(std::move(enemy));
}

void EntityManager::AddDestructible(Destructible&& destructible)
{
    m_destructibles.push_back(std::move(destructible));
}

void EntityManager::AddPickup(GroundPickup&& pickup)
{
    m_pickups.push_back(std::move(pickup));
}
