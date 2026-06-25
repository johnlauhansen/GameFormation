#include "game_world.h"

GameWorld::GameWorld()
    : m_player({ (float)TileMap::kTileSize * 2.5f, (float)TileMap::kTileSize * 2.5f })
    , m_notificationText("")
    , m_notificationTimer(0.0f)
{
    m_camera = { 0 };
    m_camera.target = m_player.GetPosition();
    m_camera.offset = { 800.0f / 2.0f, 600.0f / 2.0f };
    m_camera.rotation = 0.0f;
    m_camera.zoom = 1.2f;

    Reset();
}

void GameWorld::Update(float deltaTime)
{
    const Vector2 oldPos = m_player.GetPosition();

    m_player.Update(deltaTime);

    const Vector2 currentPos = m_player.GetPosition();
    const Rectangle collisionRect = m_player.GetCollisionRect();

    if (m_tileMap.CheckCollision(collisionRect))
    {
        const Vector2 resolvedPos = m_tileMap.ResolveCollision(currentPos, oldPos, collisionRect.width, collisionRect.height);
        m_player.SetPosition(resolvedPos);
    }

    /* Détection de collecte d'objets */
    for (auto& pickup : m_pickups)
    {
        if (pickup.active)
        {
            Rectangle pickupRect = { pickup.position.x - 12.0f, pickup.position.y - 12.0f, 24.0f, 24.0f };
            if (CheckCollisionRecs(collisionRect, pickupRect))
            {
                pickup.active = false;
                m_player.GetInventory().AddItem(pickup.itemId);
                
                /* Déclenchement de la bannière de notification */
                m_notificationText = "Vous avez obtenu : " + pickup.name + " !";
                m_notificationTimer = 3.0f;
            }
        }
    }

    if (m_notificationTimer > 0.0f)
    {
        m_notificationTimer -= deltaTime;
    }

    const Vector2 targetPos = m_player.GetPosition();
    m_camera.target.x += (targetPos.x - m_camera.target.x) * 0.1f;
    m_camera.target.y += (targetPos.y - m_camera.target.y) * 0.1f;
}

void GameWorld::Draw() const
{
    BeginMode2D(m_camera);

    m_tileMap.Draw();

    /* Dessiner les objets au sol */
    for (const auto& pickup : m_pickups)
    {
        if (pickup.active)
        {
            if (pickup.itemId == "sword")
            {
                /* Dessin d'une petite épée grise inclinée au sol */
                DrawLineEx({ pickup.position.x - 8, pickup.position.y + 8 }, { pickup.position.x + 8, pickup.position.y - 8 }, 3.5f, LIGHTGRAY);
                DrawLineEx({ pickup.position.x - 10, pickup.position.y + 10 }, { pickup.position.x - 6, pickup.position.y + 6 }, 4.0f, BROWN);
                DrawCircleV({ pickup.position.x + 8, pickup.position.y - 8 }, 2.5f, WHITE);
            }
            else if (pickup.itemId == "boomerang")
            {
                /* Dessin d'un boomerang bleu ciel rotatif doux */
                DrawCircleSector(pickup.position, 10.0f, 45.0f, 225.0f, 4, SKYBLUE);
                DrawCircleLinesV(pickup.position, 10.0f, WHITE);
            }
        }
    }

    m_player.Draw();

    EndMode2D();

    /* Dessiner la notification d'objet collecté en espace écran */
    if (m_notificationTimer > 0.0f)
    {
        const int textWidth = MeasureText(m_notificationText.c_str(), 18);
        const int boxWidth = textWidth + 40;
        
        DrawRectangle(400 - boxWidth / 2, 40, boxWidth, 40, Fade(BLACK, 0.85f));
        DrawRectangleLines(400 - boxWidth / 2, 40, boxWidth, 40, GREEN);
        DrawText(m_notificationText.c_str(), 400 - textWidth / 2, 51, 18, GREEN);
    }
}

void GameWorld::Reset()
{
    m_player.SetPosition({ (float)TileMap::kTileSize * 2.5f, (float)TileMap::kTileSize * 2.5f });
    
    /* Vider l'inventaire lors d'un reset */
    m_player.GetInventory().RemoveItem("sword");
    m_player.GetInventory().RemoveItem("boomerang");
    m_player.GetInventory().m_upgradePoints = 5;

    /* Repopuler les objets au sol */
    m_pickups.clear();
    
    GroundPickup swordPickup;
    swordPickup.itemId = "sword";
    swordPickup.name = "EPEE DE LEGENDE";
    swordPickup.position = { (float)TileMap::kTileSize * 4.5f, (float)TileMap::kTileSize * 2.5f };
    swordPickup.active = true;

    GroundPickup boomerangPickup;
    boomerangPickup.itemId = "boomerang";
    boomerangPickup.name = "BOOMERANG VENT";
    boomerangPickup.position = { (float)TileMap::kTileSize * 10.5f, (float)TileMap::kTileSize * 2.5f };
    boomerangPickup.active = true;

    m_pickups.push_back(swordPickup);
    m_pickups.push_back(boomerangPickup);

    m_camera.target = m_player.GetPosition();
    m_notificationTimer = 0.0f;
}
