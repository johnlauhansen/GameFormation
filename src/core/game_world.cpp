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

    /* 1. Collisions contre les murs de la carte (TileMap) */
    if (m_tileMap.CheckCollision(collisionRect))
    {
        const Vector2 resolvedPos = m_tileMap.ResolveCollision(currentPos, oldPos, collisionRect.width, collisionRect.height);
        m_player.SetPosition(resolvedPos);
    }

    /* 2. Mettre à jour les objets destructibles */
    for (auto& dest : m_destructibles)
    {
        dest.Update(deltaTime);
    }

    /* 3. Collisions glissantes contre les objets destructibles physiques solides */
    const Vector2 posAfterTileCheck = m_player.GetPosition();
    Vector2 finalPos = posAfterTileCheck;

    /* Essai sur l'axe X */
    m_player.SetPosition({ posAfterTileCheck.x, oldPos.y });
    bool collideX = false;
    for (const auto& dest : m_destructibles)
    {
        if (dest.IsAlive() && CheckCollisionRecs(m_player.GetCollisionRect(), dest.GetCollisionRect()))
        {
            collideX = true;
            break;
        }
    }
    if (collideX)
    {
        finalPos.x = oldPos.x;
    }

    /* Essai sur l'axe Y */
    m_player.SetPosition({ finalPos.x, posAfterTileCheck.y });
    bool collideY = false;
    for (const auto& dest : m_destructibles)
    {
        if (dest.IsAlive() && CheckCollisionRecs(m_player.GetCollisionRect(), dest.GetCollisionRect()))
        {
            collideY = true;
            break;
        }
    }
    if (collideY)
    {
        finalPos.y = oldPos.y;
    }

    m_player.SetPosition(finalPos);

    /* 4. Détection des attaques à l'épée sur les objets destructibles */
    if (m_player.GetState() == PlayerState::Attacking)
    {
        const Rectangle attackRect = m_player.GetAttackRect();
        const Item* sword = m_player.GetInventory().GetItem("sword");
        if (sword != nullptr && sword->collected)
        {
            for (auto& dest : m_destructibles)
            {
                if (dest.IsAlive() && CheckCollisionRecs(attackRect, dest.GetCollisionRect()))
                {
                    /* L'épée inflige ses dégâts avec son type physique et son élément magique actifs */
                    dest.TakeDamage(sword->damage, sword->damageType, sword->element);
                }
            }
        }
    }

    /* Détection de collecte d'objets */
    for (auto& pickup : m_pickups)
    {
        if (pickup.active)
        {
            Rectangle pickupRect = { pickup.position.x - 12.0f, pickup.position.y - 12.0f, 24.0f, 24.0f };
            if (CheckCollisionRecs(m_player.GetCollisionRect(), pickupRect))
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

    /* Dessiner les objets destructibles */
    for (const auto& dest : m_destructibles)
    {
        dest.Draw();
    }

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

    /* Repopuler les objets destructibles */
    m_destructibles.clear();

    Destructible crate1(DestructibleType::Crate, { (float)TileMap::kTileSize * 6.5f, (float)TileMap::kTileSize * 4.5f });
    Destructible plant1(DestructibleType::Plant, { (float)TileMap::kTileSize * 8.5f, (float)TileMap::kTileSize * 4.5f });
    
    /* Monument mystique Custom : vulnérable aux attaques contondantes (Blunt) OU au Feu (Fire) ! */
    Destructible customObj(DestructibleType::Custom, { (float)TileMap::kTileSize * 7.5f, (float)TileMap::kTileSize * 5.5f });
    customObj.AddVulnerableDamageType(DamageType::Blunt);
    customObj.AddVulnerableElement(ElementType::Fire);
    customObj.SetMaxHealth(50.0f);

    m_destructibles.push_back(crate1);
    m_destructibles.push_back(plant1);
    m_destructibles.push_back(customObj);

    m_camera.target = m_player.GetPosition();
    m_notificationTimer = 0.0f;
}
