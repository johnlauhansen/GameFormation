#include "game_world.h"
#include "map/map_loader.h"
#include "combat_system.h"
#include "physics_system.h"
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

    /* Fonction d'aide locale pour trouver un emplacement de spawn sécurisé hors des murs */
    auto findSafeSpawn = [this](Vector2 desiredPos, float width, float height) -> Vector2 {
        Vector2 pos = desiredPos;
        Rectangle rect = { pos.x - width / 2.0f, pos.y - height / 2.0f, width, height };
        
        if (!m_tileMap.CheckCollision(rect))
        {
            return pos;
        }

        /* Si collision, on spirale autour pour trouver une tuile libre */
        float step = 32.0f; /* Taille de tuile */
        int maxAttempts = 24;
        int stepCount = 1;
        int stepLimit = 1;
        int stepDir = 0; /* 0: D, 1: S, 2: G, 3: N */

        while (m_tileMap.CheckCollision(rect) && maxAttempts > 0)
        {
            if (stepDir == 0) pos.x += step;
            else if (stepDir == 1) pos.y += step;
            else if (stepDir == 2) pos.x -= step;
            else if (stepDir == 3) pos.y -= step;

            rect.x = pos.x - width / 2.0f;
            rect.y = pos.y - height / 2.0f;

            stepCount++;
            if (stepCount > stepLimit)
            {
                stepCount = 1;
                if (stepDir == 1 || stepDir == 3)
                {
                    stepLimit++;
                }
                stepDir = (stepDir + 1) % 4;
            }
            maxAttempts--;
        }
        return pos;
    };

    /* 4. Génération des objets destructibles (mélange de spawner dynamique et d'objets Tiled) */
    m_destructibles.clear();

    Vector2 crate1Pos = findSafeSpawn({ spawnPos.x - 40.0f, spawnPos.y + 40.0f }, 32.0f, 32.0f);
    Destructible crate1(DestructibleType::Crate, crate1Pos);
    
    Vector2 plant1Pos = findSafeSpawn({ spawnPos.x + 40.0f, spawnPos.y + 40.0f }, 32.0f, 32.0f);
    Destructible plant1(DestructibleType::Plant, plant1Pos);
    
    Vector2 customObjPos = findSafeSpawn({ spawnPos.x, spawnPos.y - 60.0f }, 32.0f, 32.0f);
    Destructible customObj(DestructibleType::Custom, customObjPos);
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
    Vector2 villagerPos = findSafeSpawn({ spawnPos.x - 120.0f, spawnPos.y + 120.0f }, 32.0f, 32.0f);
    Npc villager("Jean le Villageois", NpcType::Villager, villagerPos);
    villager.SetPatrolZone(100.0f, 40.0f);
    m_npcs.push_back(std::move(villager));

    // Un donneur de quête statique à côté des caisses
    Vector2 questGiverPos = findSafeSpawn({ spawnPos.x + 160.0f, spawnPos.y - 80.0f }, 32.0f, 32.0f);
    Npc questGiver("Bucheron Bourru", NpcType::QuestGiver, questGiverPos);
    questGiver.SetStatic();
    questGiver.ConfigureQuest("crate_hunt", "Detruire les caisses encombrantes", 5, 80, 2);
    m_npcs.push_back(std::move(questGiver));

    // Un marchand ambulant avec des objets à vendre
    Vector2 merchantPos = findSafeSpawn({ spawnPos.x - 180.0f, spawnPos.y - 120.0f }, 32.0f, 32.0f);
    Npc merchant("Marchand Ambulant", NpcType::Merchant, merchantPos);
    merchant.SetStatic();
    merchant.AddMerchantItem("heal_potion", "Potion de Sante", 10, "Restaure completement vos coeurs.");
    merchant.AddMerchantItem("forge_point", "Infu de Point de Forge", 25, "Ajoute 1 point de forge precieux.");
    merchant.AddMerchantItem("boomerang", "Boomerang d'acier", 40, "Arme de jet rotative secondaire.");
    m_npcs.push_back(std::move(merchant));

    /* 7. Génération des Ennemis */
    m_enemies.clear();

    // Quelques Slimes patrouilleurs
    Vector2 slime1Pos = findSafeSpawn({ spawnPos.x + 200.0f, spawnPos.y + 200.0f }, 32.0f, 32.0f);
    Enemy slime1("Slime Vert", EnemyType::Slime, slime1Pos);
    slime1.SetPatrolZone(80.0f, 50.0f);
    m_enemies.push_back(slime1);

    Vector2 slime2Pos = findSafeSpawn({ spawnPos.x - 200.0f, spawnPos.y + 250.0f }, 32.0f, 32.0f);
    Enemy slime2("Slime Agile", EnemyType::Slime, slime2Pos);
    slime2.SetPatrolZone(60.0f, 70.0f);
    m_enemies.push_back(slime2);

    // Un Octorok à distance
    Vector2 octorokPos = findSafeSpawn({ spawnPos.x + 300.0f, spawnPos.y - 150.0f }, 32.0f, 32.0f);
    Enemy octorok("Octorok Rouge", EnemyType::Octorok, octorokPos);
    octorok.SetPatrolZone(50.0f, 30.0f);
    m_enemies.push_back(octorok);

    // Un Moblin d'élite patrouillant sur un chemin défini
    Vector2 moblinPos = findSafeSpawn({ spawnPos.x - 250.0f, spawnPos.y - 200.0f }, 36.0f, 36.0f);
    Enemy moblin("Moblin de Garde", EnemyType::Moblin, moblinPos);
    std::vector<Vector2> moblinWaypoints = {
        moblinPos,
        { moblinPos.x + 150.0f, moblinPos.y },
        { moblinPos.x + 150.0f, moblinPos.y - 150.0f },
        { moblinPos.x, moblinPos.y - 150.0f }
    };
    moblin.SetDefinedPath(moblinWaypoints, 60.0f);
    m_enemies.push_back(moblin);

    m_playerHitCooldown = 0.0f;

    m_hud.Reset();

    return true;
}

void GameWorld::Update(float deltaTime)
{
    /* 1. Si le système de dialogue est actif, on met à jour uniquement celui-ci (jeu figé) */
    if (m_dialogueSystem.IsActive())
    {
        m_dialogueSystem.Update(deltaTime, m_player, m_hud);
        return;
    }

    const Vector2 oldPos = m_player.GetPosition();

    /* 2. Mettre à jour les acteurs */
    m_player.Update(deltaTime);

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

    /* 3. Résoudre les déplacements physiques glissants du joueur via PhysicsSystem */
    const Vector2 resolvedPos = PhysicsSystem::ResolvePlayerMovement(m_player, oldPos, m_tileMap, m_destructibles);
    m_player.SetPosition(resolvedPos);

    /* 4. Initier un dialogue si le joueur appuie sur E à proximité d'un PNJ */
    if (IsKeyPressed(KEY_E))
    {
        for (auto& npc : m_npcs)
        {
            if (npc.IsPlayerNear(m_player.GetPosition()))
            {
                m_dialogueSystem.StartInteraction(npc, m_player);
                break;
            }
        }
    }

    /* 5. Gérer le combat (Épée, Boomerang, et Dégâts ennemis sur joueur) via CombatSystem */
    CombatSystem::ResolvePlayerSwordAttacks(m_player, m_enemies, m_destructibles, m_pickups, m_npcs);

    /* Gestion de l'input et initialisation du boomerang dans GameWorld, puis résolution physique dans CombatSystem */
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

    CombatSystem::ResolvePlayerBoomerangAttacks(m_player, m_boomerang, m_enemies, m_destructibles, m_pickups, m_npcs, m_tileMap, deltaTime);

    CombatSystem::ResolveEnemyDamageToPlayer(m_player, m_enemies, m_playerHitCooldown, m_hud, deltaTime);

    /* 6. Détection de collecte des objets au sol */
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

    /* Rendu du DialogueSystem en espace écran */
    if (m_dialogueSystem.IsActive())
    {
        m_dialogueSystem.Draw(m_player);
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
