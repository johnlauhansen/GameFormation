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
                    bool wasAlive = dest.IsAlive();
                    /* L'épée inflige ses dégâts avec son type physique et son élément magique actifs */
                    if (dest.TakeDamage(sword->damage, sword->damageType, sword->element))
                    {
                        if (wasAlive && !dest.IsAlive())
                        {
                            /* Spawner un rubis vert */
                            GroundPickup rupee;
                            rupee.itemId = "rupee";
                            rupee.name = "RUBIS VERT";
                            rupee.position = dest.GetPosition();
                            rupee.active = true;
                            m_pickups.push_back(rupee);
                        }
                    }
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
            /* Le lancer du boomerang consomme 15 points de magie */
            if (m_player.GetMagic() >= 15.0f)
            {
                m_player.SetMagic(m_player.GetMagic() - 15.0f);
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
                        bool wasAlive = dest.IsAlive();
                        if (dest.TakeDamage(boomStats->damage, boomStats->damageType, boomStats->element))
                        {
                            if (wasAlive && !dest.IsAlive())
                            {
                                /* Spawner un rubis vert */
                                GroundPickup rupee;
                                rupee.itemId = "rupee";
                                rupee.name = "RUBIS VERT";
                                rupee.position = dest.GetPosition();
                                rupee.active = true;
                                m_pickups.push_back(rupee);
                            }
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
                if (pickup.itemId == "rupee")
                {
                    m_player.AddRupees(1);
                    m_notificationText = "+1 RUBIS !";
                    m_notificationTimer = 1.0f;
                }
                else
                {
                    m_player.GetInventory().AddItem(pickup.itemId);
                    /* Déclenchement de la bannière de notification */
                    m_notificationText = "Vous avez obtenu : " + pickup.name + " !";
                    m_notificationTimer = 3.0f;
                }
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
            else if (pickup.itemId == "rupee")
            {
                /* Dessin d'un rubis vert brillant au sol */
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

    /* Dessiner le HUD complet du joueur */
    DrawHUD();
}

void GameWorld::DrawHUD() const
{
    // --- 1. HEALTH POINTS (HEARTS) TOP-RIGHT ---
    // On dessine de droite à gauche ou aligné à gauche de la zone top-right
    float startX = 610.0f;
    float startY = 20.0f;
    float heartSize = 18.0f;
    float spacing = 22.0f;

    int maxHearts = (int)(m_player.GetMaxHealth() / 20.0f);
    float currentHp = m_player.GetHealth();

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

    // --- 2. MAGIC BAR BELOW HEARTS ---
    float magicBarX = 610.0f;
    float magicBarY = 45.0f;
    float magicBarW = 150.0f;
    float magicBarH = 14.0f;

    DrawText("MAGIC", (int)magicBarX - 45, (int)magicBarY + 2, 10, GREEN);

    DrawRectangle((int)magicBarX, (int)magicBarY, (int)magicBarW, (int)magicBarH, Color{ 25, 45, 25, 255 });
    
    float magicPercent = m_player.GetMagic() / m_player.GetMaxMagic();
    if (magicPercent < 0.0f) magicPercent = 0.0f;
    if (magicPercent > 1.0f) magicPercent = 1.0f;
    
    DrawRectangle((int)magicBarX + 2, (int)magicBarY + 2, (int)((magicBarW - 4.0f) * magicPercent), (int)magicBarH - 4, GREEN);
    
    if (magicPercent > 0.05f)
    {
        DrawRectangle((int)magicBarX + 2, (int)magicBarY + 2, (int)((magicBarW - 4.0f) * magicPercent), 2, Color{ 150, 255, 150, 255 });
    }

    DrawRectangleLines((int)magicBarX, (int)magicBarY, (int)magicBarW, (int)magicBarH, WHITE);

    // --- 3. EQUIPPED ITEMS TOP-RIGHT ---
    // Aligné à gauche de la zone de vie et magie
    float itemsX = 420.0f;
    float itemsY = 16.0f;
    DrawEquippedItemBox(itemsX, itemsY, "I", "sword", "ESPACE");
    DrawEquippedItemBox(itemsX + 54.0f, itemsY, "II", "boomerang", "B");
    DrawEquippedItemBox(itemsX + 108.0f, itemsY, "III", "shield", "C");

    // --- 4. RUPEE POUCH BOTTOM-LEFT ---
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

    DrawText(TextFormat("x %03d", m_player.GetRupees()), 52, 510, 16, RAYWHITE);

    // --- 5. MINIMAP BOTTOM-RIGHT ---
    DrawMiniMap(650.0f, 430.0f, 130.0f, 110.0f);
}

void GameWorld::DrawHeart(float x, float y, float size, float fillPercent) const
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

void GameWorld::DrawEquippedItemBox(float x, float y, const std::string& label, const std::string& itemId, const std::string& keyName) const
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
        hasItem = m_player.GetInventory().HasItem(itemId);
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
            
            const Item* sword = m_player.GetInventory().GetItem("sword");
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
            const Item* boom = m_player.GetInventory().GetItem("boomerang");
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

void GameWorld::DrawMiniMap(float x, float y, float w, float h) const
{
    DrawRectangle(x, y, w, h, Fade(BLACK, 0.75f));
    DrawRectangleLinesEx({ x, y, w, h }, 2.0f, GOLD);

    int mapW = m_tileMap.GetWidth();
    int mapH = m_tileMap.GetHeight();
    if (mapW <= 0 || mapH <= 0)
    {
        return;
    }

    const std::vector<bool>& collisions = m_tileMap.GetCollisions();

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

    for (const auto& dest : m_destructibles)
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

    for (const auto& pickup : m_pickups)
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

    Vector2 playerPos = m_player.GetPosition();
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

void GameWorld::Reset()
{
    LoadMap("assets/maps/game/overworld.json");
}
