#include "hud.h"
#include <raylib.h>

HUD::HUD()
    : m_notificationText("")
    , m_notificationTimer(0.0f)
{
}

void HUD::Update(float deltaTime)
{
    if (m_notificationTimer > 0.0f)
    {
        m_notificationTimer -= deltaTime;
    }
}

void HUD::Draw(const Player& player, const TileMap& tileMap, 
               const std::vector<Destructible>& destructibles, 
               const std::vector<GroundPickup>& pickups) const
{
    /* Dessiner la notification d'objet collecté en espace écran */
    if (m_notificationTimer > 0.0f)
    {
        const int textWidth = MeasureText(m_notificationText.c_str(), 18);
        const int boxWidth = textWidth + 40;
        
        DrawRectangle(400 - boxWidth / 2, 40, boxWidth, 40, Fade(BLACK, 0.85f));
        DrawRectangleLines(400 - boxWidth / 2, 40, boxWidth, 40, GREEN);
        DrawText(m_notificationText.c_str(), 400 - textWidth / 2, 51, 18, GREEN);
    }

    /* Dessiner le HUD complet du joueur */
    DrawHearts(player);
    DrawMagicBar(player);
    DrawEquippedItems(player);
    DrawRupeePouch(player);
    DrawMiniMap(tileMap, destructibles, pickups, player);
}

void HUD::TriggerNotification(const std::string& text, float duration)
{
    m_notificationText = text;
    m_notificationTimer = duration;
}

void HUD::Reset()
{
    m_notificationText = "";
    m_notificationTimer = 0.0f;
}

void HUD::DrawHearts(const Player& player) const
{
    float startX = 610.0f;
    float startY = 20.0f;
    float heartSize = 18.0f;
    float spacing = 22.0f;

    int maxHearts = (int)(player.GetMaxHealth() / 20.0f);
    float currentHp = player.GetHealth();

    for (int i = 0; i < maxHearts; ++i)
    {
        float heartHp = currentHp - (i * 20.0f);
        float fillPercent = 0.0f;
        if (heartHp >= 20.0f)
        {
            fillPercent = 1.0f;
        }
        else if (heartHp > 0.0f)
        {
            fillPercent = heartHp / 20.0f;
        }

        DrawHeart(startX + i * spacing, startY, heartSize, fillPercent);
    }
}

void HUD::DrawHeart(float x, float y, float size, float fillPercent) const
{
    float r = size * 0.28f;
    Vector2 leftCircle = { x - r, y - r * 0.4f };
    Vector2 rightCircle = { x + r, y - r * 0.4f };

    DrawCircleV(leftCircle, r + 1.5f, BLACK);
    DrawCircleV(rightCircle, r + 1.5f, BLACK);
    DrawTriangle({ x, y + r * 1.5f + 1.5f }, { x + r * 2.0f + 1.5f, y - r * 0.4f }, { x - r * 2.0f - 1.5f, y - r * 0.4f }, BLACK);

    Color bgColor = { 50, 10, 10, 255 };
    DrawCircleV(leftCircle, r, bgColor);
    DrawCircleV(rightCircle, r, bgColor);
    DrawTriangle({ x, y + r * 1.5f }, { x + r * 2.0f, y - r * 0.4f }, { x - r * 2.0f, y - r * 0.4f }, bgColor);

    if (fillPercent > 0.0f)
    {
        Color fillColor = RED;
        if (fillPercent >= 1.0f)
        {
            DrawCircleV(leftCircle, r, fillColor);
            DrawCircleV(rightCircle, r, fillColor);
            DrawTriangle({ x, y + r * 1.5f }, { x + r * 2.0f, y - r * 0.4f }, { x - r * 2.0f, y - r * 0.4f }, fillColor);
        }
        else
        {
            BeginScissorMode((int)(x - size), (int)(y - size), (int)size, (int)(size * 2.5f));
            DrawCircleV(leftCircle, r, fillColor);
            DrawCircleV(rightCircle, r, fillColor);
            DrawTriangle({ x, y + r * 1.5f }, { x + r * 2.0f, y - r * 0.4f }, { x - r * 2.0f, y - r * 0.4f }, fillColor);
            EndScissorMode();
        }
    }

    if (fillPercent > 0.0f)
    {
        DrawCircleV({ leftCircle.x - r * 0.4f, leftCircle.y - r * 0.4f }, r * 0.25f, WHITE);
    }
}

void HUD::DrawMagicBar(const Player& player) const
{
    float magicBarX = 610.0f;
    float magicBarY = 45.0f;
    float magicBarW = 150.0f;
    float magicBarH = 14.0f;

    DrawText("MAGIC", (int)magicBarX - 45, (int)magicBarY + 2, 10, GREEN);

    DrawRectangle((int)magicBarX, (int)magicBarY, (int)magicBarW, (int)magicBarH, Color{ 25, 45, 25, 255 });
    
    float magicPercent = player.GetMagic() / player.GetMaxMagic();
    if (magicPercent < 0.0f) magicPercent = 0.0f;
    if (magicPercent > 1.0f) magicPercent = 1.0f;
    
    DrawRectangle((int)magicBarX + 2, (int)magicBarY + 2, (int)((magicBarW - 4.0f) * magicPercent), (int)magicBarH - 4, GREEN);
    
    if (magicPercent > 0.05f)
    {
        DrawRectangle((int)magicBarX + 2, (int)magicBarY + 2, (int)((magicBarW - 4.0f) * magicPercent), 2, Color{ 150, 255, 150, 255 });
    }

    DrawRectangleLines((int)magicBarX, (int)magicBarY, (int)magicBarW, (int)magicBarH, WHITE);
}

void HUD::DrawEquippedItems(const Player& player) const
{
    float itemsX = 420.0f;
    float itemsY = 16.0f;
    DrawEquippedItemBox(itemsX, itemsY, "I", "sword", "ESPACE", player);
    DrawEquippedItemBox(itemsX + 54.0f, itemsY, "II", "boomerang", "B", player);
    DrawEquippedItemBox(itemsX + 108.0f, itemsY, "III", "shield", "C", player);
}

void HUD::DrawEquippedItemBox(float x, float y, const std::string& label, const std::string& itemId, const std::string& keyName, const Player& player) const
{
    float size = 44.0f;
    Rectangle box = { x, y, size, size };

    bool hasItem = false;
    if (itemId == "shield")
    {
        hasItem = true;
    }
    else
    {
        hasItem = player.GetInventory().HasItem(itemId);
    }

    DrawRectangleRec(box, Fade(BLACK, 0.75f));
    
    Color borderColor = hasItem ? GOLD : Color{ 60, 60, 60, 255 };
    DrawRectangleLinesEx(box, 2.0f, borderColor);

    if (hasItem)
    {
        float cx = x + size / 2.0f;
        float cy = y + size / 2.0f;

        if (itemId == "sword")
        {
            DrawLineEx({ cx - 10, cy + 10 }, { cx + 10, cy - 10 }, 3.0f, LIGHTGRAY);
            DrawLineEx({ cx - 12, cy + 12 }, { cx - 8, cy + 8 }, 3.5f, BROWN);
            DrawCircleV({ cx + 10, cy - 10 }, 1.5f, WHITE);
            
            const Item* sword = player.GetInventory().GetItem("sword");
            if (sword != nullptr && sword->element != ElementType::None)
            {
                Color elemColor = RED;
                if (sword->element == ElementType::Fire) elemColor = ORANGE;
                else if (sword->element == ElementType::Ice) elemColor = SKYBLUE;
                else if (sword->element == ElementType::Lightning) elemColor = GOLD;
                DrawCircleV({ x + 8, y + 8 }, 3.0f, elemColor);
            }
        }
        else if (itemId == "boomerang")
        {
            Color boomColor = SKYBLUE;
            const Item* boom = player.GetInventory().GetItem("boomerang");
            if (boom != nullptr && boom->element != ElementType::None)
            {
                if (boom->element == ElementType::Fire) boomColor = ORANGE;
                else if (boom->element == ElementType::Ice) boomColor = SKYBLUE;
                else if (boom->element == ElementType::Lightning) boomColor = GOLD;
            }
            DrawCircleSector({ cx, cy }, 10.0f, 45.0f, 225.0f, 4, boomColor);
            DrawCircleLinesV({ cx, cy }, 10.0f, WHITE);
        }
        else if (itemId == "shield")
        {
            Vector2 p1 = { cx - 10, cy - 10 };
            Vector2 p2 = { cx + 10, cy - 10 };
            Vector2 p3 = { cx + 10, cy + 2 };
            Vector2 p4 = { cx, cy + 12 };
            Vector2 p5 = { cx - 10, cy + 2 };
            
            DrawTriangle(p1, p3, p2, BLUE);
            DrawTriangle(p1, p4, p3, BLUE);
            DrawTriangle(p1, p5, p4, BLUE);
            
            DrawLineV(p1, p2, WHITE);
            DrawLineV(p2, p3, WHITE);
            DrawLineV(p3, p4, WHITE);
            DrawLineV(p4, p5, WHITE);
            DrawLineV(p5, p1, WHITE);

            DrawLineEx({ cx, cy - 6 }, { cx, cy + 6 }, 1.5f, GOLD);
            DrawLineEx({ cx - 5, cy }, { cx + 5, cy }, 1.5f, GOLD);
        }
    }
    else
    {
        DrawText("?", (int)x + 18, (int)y + 14, 16, Color{ 80, 80, 80, 255 });
    }

    DrawText(keyName.c_str(), (int)x + (int)size / 2 - MeasureText(keyName.c_str(), 10) / 2, (int)y + (int)size + 3, 10, LIGHTGRAY);
}

void HUD::DrawRupeePouch(const Player& player) const
{
    DrawRectangle(20, 500, 100, 35, Fade(BLACK, 0.75f));
    DrawRectangleLines(20, 500, 100, 35, GREEN);
    
    float rx = 35.0f;
    float ry = 517.0f;
    float rw = 10.0f;
    float rh = 16.0f;
    float rh2 = rh * 0.3f;
    Color rupeeColor = GREEN;
    DrawRectangle((int)(rx - rw/2), (int)(ry - rh/2 + rh2), (int)rw, (int)(rh - 2*rh2), rupeeColor);
    DrawTriangle({ rx - rw/2, ry - rh/2 + rh2 }, { rx + rw/2, ry - rh/2 + rh2 }, { rx, ry - rh/2 }, rupeeColor);
    DrawTriangle({ rx, ry + rh/2 }, { rx + rw/2, ry + rh/2 - rh2 }, { rx - rw/2, ry + rh/2 - rh2 }, rupeeColor);
    
    DrawLineEx({ rx, ry - rh/2 }, { rx - rw/2, ry - rh/2 + rh2 }, 1.0f, WHITE);
    DrawLineEx({ rx - rw/2, ry - rh/2 + rh2 }, { rx - rw/2, ry + rh/2 - rh2 }, 1.0f, WHITE);
    DrawLineEx({ rx - rw/2, ry + rh/2 - rh2 }, { rx, ry + rh/2 }, 1.0f, WHITE);
    DrawLineEx({ rx, ry + rh/2 }, { rx + rw/2, ry + rh/2 - rh2 }, 1.0f, WHITE);
    DrawLineEx({ rx + rw/2, ry + rh/2 - rh2 }, { rx + rw/2, ry - rh/2 + rh2 }, 1.0f, WHITE);
    DrawLineEx({ rx + rw/2, ry - rh/2 + rh2 }, { rx, ry - rh/2 }, 1.0f, WHITE);

    DrawText(TextFormat("x %03d", player.GetRupees()), 52, 510, 16, RAYWHITE);
}

void HUD::DrawMiniMap(const TileMap& tileMap, 
                      const std::vector<Destructible>& destructibles, 
                      const std::vector<GroundPickup>& pickups,
                      const Player& player) const
{
    float x = 650.0f;
    float y = 430.0f;
    float w = 130.0f;
    float h = 110.0f;

    DrawRectangle(x, y, w, h, Fade(BLACK, 0.75f));
    DrawRectangleLinesEx({ x, y, w, h }, 2.0f, GOLD);

    int mapW = tileMap.GetWidth();
    int mapH = tileMap.GetHeight();
    if (mapW <= 0 || mapH <= 0)
    {
        return;
    }

    const std::vector<bool>& collisions = tileMap.GetCollisions();

    float pad = 6.0f;
    float mapDrawW = w - 2 * pad;
    float mapDrawH = h - 2 * pad;
    float drawX = x + pad;
    float drawY = y + pad;

    float tileW = mapDrawW / mapW;
    float tileH = mapDrawH / mapH;

    for (int ty = 0; ty < mapH; ++ty)
    {
        for (int tx = 0; tx < mapW; ++tx)
        {
            int idx = (ty * mapW) + tx;
            if (idx >= 0 && idx < (int)collisions.size())
            {
                Rectangle tileRect = { drawX + tx * tileW, drawY + ty * tileH, tileW - 0.5f, tileH - 0.5f };
                if (collisions[idx])
                {
                    DrawRectangleRec(tileRect, Color{ 70, 80, 95, 255 });
                }
                else
                {
                    DrawRectangleRec(tileRect, Color{ 30, 90, 45, 255 });
                }
            }
        }
    }

    for (const auto& dest : destructibles)
    {
        if (dest.IsAlive())
        {
            Vector2 destPos = dest.GetPosition();
            float destTx = destPos.x / TileMap::kTileSize;
            float destTy = destPos.y / TileMap::kTileSize;
            
            float dx = drawX + destTx * tileW;
            float dy = drawY + destTy * tileH;
            
            DrawCircleV({ dx, dy }, 2.0f, BROWN);
        }
    }

    for (const auto& pickup : pickups)
    {
        if (pickup.active)
        {
            float pTx = pickup.position.x / TileMap::kTileSize;
            float pTy = pickup.position.y / TileMap::kTileSize;
            
            float px = drawX + pTx * tileW;
            float py = drawY + pTy * tileH;
            
            DrawCircleV({ px, py }, 2.0f, GOLD);
        }
    }

    Vector2 playerPos = player.GetPosition();
    float playerTx = playerPos.x / TileMap::kTileSize;
    float playerTy = playerPos.y / TileMap::kTileSize;

    float pMiniX = drawX + playerTx * tileW;
    float pMiniY = drawY + playerTy * tileH;

    float time = (float)GetTime();
    bool blink = ((int)(time * 4.0f) % 2) == 0;
    Color playerDotColor = blink ? RED : WHITE;

    DrawCircleV({ pMiniX, pMiniY }, 3.5f, playerDotColor);
    DrawCircleLines((int)pMiniX, (int)pMiniY, 3.5f, BLACK);
}
