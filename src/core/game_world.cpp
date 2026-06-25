#include "game_world.h"
#include "map/map_loader.h"
#include <cmath>

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

bool GameWorld::LoadMap(const std::string& filePath)
{
    auto levelOpt = MapLoader::LoadFromJson(filePath);
    if (!levelOpt.has_value())
    {
        return false;
    }

    m_tileMap.LoadLevel(levelOpt.value());

    /* 1. Recherche du point d'apparition du joueur défini dans la carte Tiled */
    Vector2 spawnPos = { (float)TileMap::kTileSize * 2.5f, (float)TileMap::kTileSize * 2.5f };
    for (const auto& spawn : levelOpt.value().spawns)
    {
        if (spawn.type == "PlayerSpawn" || spawn.subType == "PlayerSpawn")
        {
            spawnPos = spawn.position;
            break;
        }
    }
    m_player.SetPosition(spawnPos);
    m_camera.target = spawnPos;

    /* 2. Réinitialisation de l'inventaire et des statistiques d'amélioration */
    m_player.GetInventory().RemoveItem("sword");
    m_player.GetInventory().RemoveItem("boomerang");
    m_player.GetInventory().m_upgradePoints = 5;

    /* 3. Génération des objets au sol par rapport au spawn joueur (assure la rétrocompatibilité) */
    m_pickups.clear();
    
    GroundPickup swordPickup;
    swordPickup.itemId = "sword";
    swordPickup.name = "EPEE DE DEBUTANT";
    swordPickup.position = { spawnPos.x - 80.0f, spawnPos.y };
    swordPickup.active = true;

    GroundPickup boomerangPickup;
    boomerangPickup.itemId = "boomerang";
    boomerangPickup.name = "BOOMERANG DE VOYAGE";
    boomerangPickup.position = { spawnPos.x + 80.0f, spawnPos.y };
    boomerangPickup.active = true;

    m_pickups.push_back(swordPickup);
    m_pickups.push_back(boomerangPickup);

    /* 4. Génération des objets destructibles par rapport au spawn joueur */
    m_destructibles.clear();

    Destructible crate1(DestructibleType::Crate, { spawnPos.x - 40.0f, spawnPos.y + 40.0f });
    Destructible plant1(DestructibleType::Plant, { spawnPos.x + 40.0f, spawnPos.y + 40.0f });
    
    /* Monument mystique Custom : vulnérable aux attaques contondantes (Blunt) OU au Feu (Fire) ! */
    Destructible customObj(DestructibleType::Custom, { spawnPos.x, spawnPos.y - 60.0f });
    customObj.AddVulnerableDamageType(DamageType::Blunt);
    customObj.AddVulnerableElement(ElementType::Fire);
    customObj.SetMaxHealth(50.0f);

    m_destructibles.push_back(crate1);
    m_destructibles.push_back(plant1);
    m_destructibles.push_back(customObj);

    /* 5. Désactivation de tout boomerang actif */
    m_boomerang = BoomerangProjectile();

    m_notificationTimer = 0.0f;
    m_notificationText = "";

    return true;
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

    /* 5. Lancement et mise à jour du Boomerang */
    if (m_player.GetInventory().HasItem("boomerang"))
    {
        const Item* boomStats = m_player.GetInventory().GetItem("boomerang");
        
        bool isBoomerangPressed = IsKeyPressed(KEY_B);
        if (IsGamepadAvailable(0))
        {
            if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT) || 
                IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_UP))
            {
                isBoomerangPressed = true;
            }
        }

        if (isBoomerangPressed && !m_boomerang.active && m_player.GetState() != PlayerState::Attacking)
        {
            m_boomerang.active = true;
            m_boomerang.returning = false;
            m_boomerang.position = m_player.GetPosition();
            m_boomerang.originPos = m_player.GetPosition();

            m_boomerang.speed = (boomStats != nullptr) ? boomStats->speed : 350.0f;
            m_boomerang.maxRange = (boomStats != nullptr) ? boomStats->range : 150.0f;

            m_boomerang.velocity = { 0.0f, 0.0f };
            if (m_player.GetDirection() == Direction::Up)
            {
                m_boomerang.velocity.y = -m_boomerang.speed;
            }
            else if (m_player.GetDirection() == Direction::Down)
            {
                m_boomerang.velocity.y = m_boomerang.speed;
            }
            else if (m_player.GetDirection() == Direction::Left)
            {
                m_boomerang.velocity.x = -m_boomerang.speed;
            }
            else if (m_player.GetDirection() == Direction::Right)
            {
                m_boomerang.velocity.x = m_boomerang.speed;
            }
        }
    }

    if (m_boomerang.active)
    {
        m_boomerang.rotation += 720.0f * deltaTime;

        if (!m_boomerang.returning)
        {
            m_boomerang.position.x += m_boomerang.velocity.x * deltaTime;
            m_boomerang.position.y += m_boomerang.velocity.y * deltaTime;

            const float dx = m_boomerang.position.x - m_boomerang.originPos.x;
            const float dy = m_boomerang.position.y - m_boomerang.originPos.y;
            const float distance = std::sqrt((dx * dx) + (dy * dy));

            const Rectangle boomRect = { m_boomerang.position.x - 8.0f, m_boomerang.position.y - 8.0f, 16.0f, 16.0f };
            
            /* Collision du boomerang contre les objets destructibles */
            const Item* boomStats = m_player.GetInventory().GetItem("boomerang");
            if (boomStats != nullptr)
            {
                for (auto& dest : m_destructibles)
                {
                    if (dest.IsAlive() && CheckCollisionRecs(boomRect, dest.GetCollisionRect()))
                    {
                        if (dest.TakeDamage(boomStats->damage, boomStats->damageType, boomStats->element))
                        {
                            m_boomerang.returning = true;
                            break;
                        }
                    }
                }
            }

            /* Collision du boomerang contre les murs */
            if (distance >= m_boomerang.maxRange || m_tileMap.CheckCollision(boomRect))
            {
                m_boomerang.returning = true;
            }
        }
        else
        {
            const Vector2 playerPos = m_player.GetPosition();
            const Vector2 dirToPlayer = { playerPos.x - m_boomerang.position.x, playerPos.y - m_boomerang.position.y };
            const float length = std::sqrt((dirToPlayer.x * dirToPlayer.x) + (dirToPlayer.y * dirToPlayer.y));

            if (length <= 20.0f)
            {
                m_boomerang.active = false;
            }
            else
            {
                m_boomerang.position.x += (dirToPlayer.x / length) * m_boomerang.speed * deltaTime;
                m_boomerang.position.y += (dirToPlayer.y / length) * m_boomerang.speed * deltaTime;
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

    /* Rendu visuel du boomerang actif */
    if (m_boomerang.active)
    {
        Color boomColor = SKYBLUE;
        const Item* boomStats = m_player.GetInventory().GetItem("boomerang");
        if (boomStats != nullptr)
        {
            if (boomStats->element == ElementType::Fire)
            {
                boomColor = ORANGE;
            }
            else if (boomStats->element == ElementType::Ice)
            {
                boomColor = SKYBLUE;
            }
            else if (boomStats->element == ElementType::Lightning)
            {
                boomColor = GOLD;
            }
        }

        DrawCircleSector(m_boomerang.position, 12.0f, m_boomerang.rotation, m_boomerang.rotation + 180.0f, 4, boomColor);
        DrawCircleLinesV(m_boomerang.position, 12.0f, WHITE);
    }

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
    LoadMap("assets/maps/game/overworld.json");
}
