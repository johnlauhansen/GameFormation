#include "game_world.h"
#include "map/map_loader.h"
#include <cmath>

GameWorld::GameWorld()
    : m_player({ (float)TileMap::kTileSize * 2.5f, (float)TileMap::kTileSize * 2.5f })
    , m_playerHitCooldown(0.0f)
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

    /* 4. Génération des objets destructibles (mélange de spawner dynamique et d'objets Tiled) */
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

    // Chargement dynamique depuis le fichier de carte Tiled JSON (Data-Driven)
    for (const auto& spawn : levelOpt.value().spawns)
    {
        if (spawn.type == "Crate" || spawn.subType == "Crate")
        {
            m_destructibles.push_back(Destructible(DestructibleType::Crate, spawn.position));
        }
        else if (spawn.type == "Plant" || spawn.subType == "Plant")
        {
            m_destructibles.push_back(Destructible(DestructibleType::Plant, spawn.position));
        }
        else if (spawn.type == "Custom" || spawn.subType == "Custom")
        {
            Destructible custom(DestructibleType::Custom, spawn.position);
            custom.AddVulnerableDamageType(DamageType::Blunt);
            custom.AddVulnerableElement(ElementType::Fire);
            custom.SetMaxHealth(50.0f);
            m_destructibles.push_back(custom);
        }
    }

    /* 5. Désactivation de tout boomerang actif */
    m_boomerang = BoomerangProjectile();

    /* 6. Génération des PNJ */
    m_npcs.clear();
    
    // Un villageois sympathique qui patrouille
    Npc villager("Jean le Villageois", NpcType::Villager, { spawnPos.x - 120.0f, spawnPos.y + 120.0f });
    villager.SetPatrolZone(100.0f, 40.0f);
    m_npcs.push_back(villager);

    // Un donneur de quête statique à côté des caisses
    Npc questGiver("Bucheron Bourru", NpcType::QuestGiver, { spawnPos.x + 160.0f, spawnPos.y - 80.0f });
    questGiver.SetStatic();
    questGiver.ConfigureQuest("crate_hunt", "Detruire les caisses encombrantes", 5, 80, 2);
    m_npcs.push_back(questGiver);

    // Un marchand ambulant avec des objets à vendre
    Npc merchant("Marchand Ambulant", NpcType::Merchant, { spawnPos.x - 180.0f, spawnPos.y - 120.0f });
    merchant.SetStatic();
    merchant.AddMerchantItem("heal_potion", "Potion de Sante", 10, "Restaure completement vos coeurs.");
    merchant.AddMerchantItem("forge_point", "Infu de Point de Forge", 25, "Ajoute 1 point de forge precieux.");
    merchant.AddMerchantItem("boomerang", "Boomerang d'acier", 40, "Arme de jet rotative secondaire.");
    m_npcs.push_back(merchant);

    /* 7. Génération des Ennemis */
    m_enemies.clear();

    // Quelques Slimes patrouilleurs
    Enemy slime1("Slime Vert", EnemyType::Slime, { spawnPos.x + 200.0f, spawnPos.y + 200.0f });
    slime1.SetPatrolZone(80.0f, 50.0f);
    m_enemies.push_back(slime1);

    Enemy slime2("Slime Agile", EnemyType::Slime, { spawnPos.x - 200.0f, spawnPos.y + 250.0f });
    slime2.SetPatrolZone(60.0f, 70.0f);
    m_enemies.push_back(slime2);

    // Un Octorok à distance
    Enemy octorok("Octorok Rouge", EnemyType::Octorok, { spawnPos.x + 300.0f, spawnPos.y - 150.0f });
    octorok.SetPatrolZone(50.0f, 30.0f);
    m_enemies.push_back(octorok);

    // Un Moblin d'élite patrouillant sur un chemin défini
    Enemy moblin("Moblin de Garde", EnemyType::Moblin, { spawnPos.x - 250.0f, spawnPos.y - 200.0f });
    std::vector<Vector2> moblinWaypoints = {
        { spawnPos.x - 250.0f, spawnPos.y - 200.0f },
        { spawnPos.x - 100.0f, spawnPos.y - 200.0f },
        { spawnPos.x - 100.0f, spawnPos.y - 350.0f },
        { spawnPos.x - 250.0f, spawnPos.y - 350.0f }
    };
    moblin.SetDefinedPath(moblinWaypoints, 60.0f);
    m_enemies.push_back(moblin);

    m_playerHitCooldown = 0.0f;

    m_hud.Reset();

    return true;
}

void GameWorld::Update(float deltaTime)
{
    /* Recherche d'un PNJ actif en dialogue */
    Npc* activeNpc = nullptr;
    for (auto& npc : m_npcs)
    {
        if (npc.IsInDialogue())
        {
            activeNpc = &npc;
            break;
        }
    }

    if (activeNpc != nullptr)
    {
        /* On gère les inputs spécifiques de dialogue ou magasin */
        if (activeNpc->IsShopActive())
        {
            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
            {
                int count = (int)activeNpc->GetMerchantItems().size();
                if (count > 0)
                {
                    int idx = activeNpc->GetSelectedShopIndex();
                    activeNpc->SetSelectedShopIndex((idx - 1 + count) % count);
                }
            }
            if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
            {
                int count = (int)activeNpc->GetMerchantItems().size();
                if (count > 0)
                {
                    int idx = activeNpc->GetSelectedShopIndex();
                    activeNpc->SetSelectedShopIndex((idx + 1) % count);
                }
            }
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_E))
            {
                const auto& items = activeNpc->GetMerchantItems();
                int idx = activeNpc->GetSelectedShopIndex();
                if (idx >= 0 && idx < (int)items.size())
                {
                    const auto& item = items[idx];
                    if (m_player.GetRupees() >= item.price)
                    {
                        m_player.AddRupees(-item.price);
                        if (item.itemId == "heal_potion")
                        {
                            m_player.SetHealth(m_player.GetHealth() + 30.0f);
                            m_hud.TriggerNotification("Achete : " + item.name + " (Vie Restauree) !", 2.0f);
                        }
                        else if (item.itemId == "forge_point")
                        {
                            m_player.GetInventory().m_upgradePoints += 1;
                            m_hud.TriggerNotification("Achete : " + item.name + " (+1 Pt de Forge) !", 2.0f);
                        }
                        else if (item.itemId == "boomerang")
                        {
                            m_player.GetInventory().AddItem(item.itemId);
                            m_hud.TriggerNotification("Achete : " + item.name + " (Obtenu) !", 2.0f);
                        }
                    }
                    else
                    {
                        m_hud.TriggerNotification("Pas assez de Rubis !", 1.5f);
                    }
                }
            }
            if (IsKeyPressed(KEY_ESCAPE))
            {
                activeNpc->CloseDialogue();
            }
        }
        else
        {
            if (IsKeyPressed(KEY_E) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))
            {
                activeNpc->Interact(m_player);
            }
        }

        m_hud.Update(deltaTime);
        return;
    }

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

    /* 2. Mettre à jour les objets destructibles, PNJ et Ennemis */
    for (auto& dest : m_destructibles)
    {
        dest.Update(deltaTime);
    }

    for (auto& npc : m_npcs)
    {
        npc.Update(deltaTime, m_tileMap);
    }

    for (auto& enemy : m_enemies)
    {
        enemy.Update(deltaTime, m_tileMap, m_player.GetPosition());
    }

    /* Détecter le bouton pour initier un dialogue */
    if (IsKeyPressed(KEY_E))
    {
        for (auto& npc : m_npcs)
        {
            if (npc.IsPlayerNear(m_player.GetPosition()))
            {
                npc.Interact(m_player);
                break;
            }
        }
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

    /* 4. Détection des attaques à l'épée sur les objets destructibles et les ennemis */
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

                            if (dest.GetType() == DestructibleType::Crate)
                            {
                                for (auto& npc : m_npcs)
                                {
                                    auto* q = npc.GetQuest();
                                    if (q && q->state == QuestState::InProgress && q->id == "crate_hunt")
                                    {
                                        q->currentKillCount++;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            for (auto& enemy : m_enemies)
            {
                if (enemy.IsAlive() && CheckCollisionRecs(attackRect, enemy.GetCollisionRect()))
                {
                    if (enemy.TakeDamage(sword->damage))
                    {
                        if (!enemy.IsAlive())
                        {
                            GroundPickup rupee;
                            rupee.itemId = "rupee";
                            rupee.name = "RUBIS VERT";
                            rupee.position = enemy.GetPosition();
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
            
            /* Collision du boomerang contre les objets destructibles et ennemis */
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

                                if (dest.GetType() == DestructibleType::Crate)
                                {
                                    for (auto& npc : m_npcs)
                                    {
                                        auto* q = npc.GetQuest();
                                        if (q && q->state == QuestState::InProgress && q->id == "crate_hunt")
                                        {
                                            q->currentKillCount++;
                                        }
                                    }
                                }
                            }
                            m_boomerang.returning = true;
                            break;
                        }
                    }
                }

                if (!m_boomerang.returning)
                {
                    for (auto& enemy : m_enemies)
                    {
                        if (enemy.IsAlive() && CheckCollisionRecs(boomRect, enemy.GetCollisionRect()))
                        {
                            if (enemy.TakeDamage(boomStats->damage))
                            {
                                if (!enemy.IsAlive())
                                {
                                    GroundPickup rupee;
                                    rupee.itemId = "rupee";
                                    rupee.name = "RUBIS VERT";
                                    rupee.position = enemy.GetPosition();
                                    rupee.active = true;
                                    m_pickups.push_back(rupee);
                                }
                                m_boomerang.returning = true;
                                break;
                            }
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

    /* 6. Collisions du joueur avec les ennemis ou leurs projectiles */
    if (m_playerHitCooldown > 0.0f)
    {
        m_playerHitCooldown -= deltaTime;
    }

    if (m_playerHitCooldown <= 0.0f)
    {
        bool playerDamaged = false;
        float damageAmount = 0.0f;

        for (const auto& enemy : m_enemies)
        {
            if (enemy.IsAlive())
            {
                /* Dégâts de contact */
                if (CheckCollisionRecs(m_player.GetCollisionRect(), enemy.GetCollisionRect()))
                {
                    playerDamaged = true;
                    damageAmount = enemy.GetDamage();
                    break;
                }

                /* Dégâts de projectile */
                for (const auto& proj : enemy.GetProjectiles())
                {
                    if (proj.active)
                    {
                        Rectangle projRect = { proj.position.x - proj.radius, proj.position.y - proj.radius, proj.radius * 2.0f, proj.radius * 2.0f };
                        if (CheckCollisionRecs(m_player.GetCollisionRect(), projRect))
                        {
                            playerDamaged = true;
                            damageAmount = enemy.GetDamage();
                            break;
                        }
                    }
                }
                
                if (playerDamaged)
                {
                    break;
                }
            }
        }

        if (playerDamaged)
        {
            m_player.SetHealth(m_player.GetHealth() - damageAmount);
            m_playerHitCooldown = 1.0f; /* 1 seconde d'invulnérabilité */
            m_hud.TriggerNotification("AIE !", 1.0f);
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
                    m_hud.TriggerNotification("+1 RUBIS !", 1.0f);
                }
                else
                {
                    m_player.GetInventory().AddItem(pickup.itemId);
                    /* Déclenchement de la bannière de notification */
                    m_hud.TriggerNotification("Vous avez obtenu : " + pickup.name + " !", 3.0f);
                }
            }
        }
    }

    m_hud.Update(deltaTime);

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

    /* Dessiner les PNJ */
    for (const auto& npc : m_npcs)
    {
        npc.Draw();
    }

    /* Dessiner les Ennemis */
    for (const auto& enemy : m_enemies)
    {
        enemy.Draw();
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

    m_hud.Draw(m_player, m_tileMap, m_destructibles, m_pickups);

    /* Recherche d'un PNJ en dialogue actif pour dessiner la boîte de dialogue en espace écran */
    const Npc* activeNpc = nullptr;
    for (const auto& npc : m_npcs)
    {
        if (npc.IsInDialogue())
        {
            activeNpc = &npc;
            break;
        }
    }

    if (activeNpc != nullptr)
    {
        /* 1. Boîte de dialogue standard */
        int boxX = 50;
        int boxY = 440;
        int boxW = 700;
        int boxH = 120;
        
        DrawRectangle(boxX, boxY, boxW, boxH, Fade(BLACK, 0.9f));
        DrawRectangleLines(boxX, boxY, boxW, boxH, BLUE);
        
        /* Nom du PNJ */
        DrawText(activeNpc->GetName().c_str(), boxX + 20, boxY + 15, 18, GOLD);
        
        /* Texte de dialogue */
        std::string currentText = activeNpc->GetCurrentDialogueText();
        DrawText(currentText.c_str(), boxX + 20, boxY + 45, 16, WHITE);

        if (activeNpc->IsShopActive())
        {
            DrawText("[ENTREE/ESPACE] Acheter  [ECHAP] Quitter", boxX + 400, boxY + 15, 12, GRAY);
            
            /* Dessiner la boutique */
            const auto& items = activeNpc->GetMerchantItems();
            int shopSel = activeNpc->GetSelectedShopIndex();
            
            int shopX = 50;
            int shopY = 160;
            int shopW = 700;
            int shopH = 260;
            
            DrawRectangle(shopX, shopY, shopW, shopH, Fade(BLACK, 0.95f));
            DrawRectangleLines(shopX, shopY, shopW, shopH, GOLD);
            DrawText("BOUTIQUE DU MARCHAND", shopX + 20, shopY + 15, 20, GOLD);
            DrawText(TextFormat("Vos Rubis : %d", m_player.GetRupees()), shopX + 500, shopY + 15, 16, GREEN);
            
            for (size_t i = 0; i < items.size(); ++i)
            {
                const auto& item = items[i];
                int itemY = shopY + 60 + (int)i * 50;
                bool isSelected = ((int)i == shopSel);
                
                Color itemColor = isSelected ? YELLOW : WHITE;
                if (isSelected)
                {
                    DrawRectangle(shopX + 15, itemY - 5, shopW - 30, 40, Fade(GRAY, 0.2f));
                    DrawRectangleLines(shopX + 15, itemY - 5, shopW - 30, 40, YELLOW);
                    DrawTriangle({ (float)shopX + 25, (float)itemY + 5 }, { (float)shopX + 25, (float)itemY + 20 }, { (float)shopX + 37, (float)itemY + 12.5f }, YELLOW);
                }
                
                DrawText(item.name.c_str(), shopX + 50, itemY + 5, 16, itemColor);
                DrawText(item.description.c_str(), shopX + 220, itemY + 7, 12, GRAY);
                
                std::string priceText = std::to_string(item.price) + " Rubis";
                Color priceColor = (m_player.GetRupees() >= item.price) ? GREEN : RED;
                DrawText(priceText.c_str(), shopX + 580, itemY + 5, 16, priceColor);
            }
        }
        else
        {
            DrawText("Appuyez sur [E] ou [ENTREE] pour continuer...", boxX + 380, boxY + 95, 12, GRAY);
        }
    }
    else
    {
        /* On cherche si un PNJ est proche pour afficher le prompt d'interaction */
        const Npc* nearNpc = nullptr;
        for (const auto& npc : m_npcs)
        {
            if (npc.IsPlayerNear(m_player.GetPosition()))
            {
                nearNpc = &npc;
                break;
            }
        }
        
        if (nearNpc != nullptr)
        {
            std::string prompt = "Appuyez sur [E] pour parler a " + nearNpc->GetName();
            int textW = MeasureText(prompt.c_str(), 14);
            DrawRectangle(400 - textW/2 - 15, 520, textW + 30, 30, Fade(BLACK, 0.8f));
            DrawRectangleLines(400 - textW/2 - 15, 520, textW + 30, 30, GREEN);
            DrawText(prompt.c_str(), 400 - textW/2, 528, 14, WHITE);
        }
    }
}

void GameWorld::Reset()
{
    LoadMap("assets/maps/game/overworld.json");
}
