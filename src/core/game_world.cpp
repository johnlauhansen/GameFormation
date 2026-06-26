#include "game_world.h"
#include "map/map_loader.h"
#include "combat_system.h"
#include "physics_system.h"
#include <cmath>

GameWorld::GameWorld()
    : m_player({ (float)TileMap::kTileSize * 2.5f, (float)TileMap::kTileSize * 2.5f })
    , m_playerHitCooldown(0.0f)
{
    m_cameraController.Initialize(m_player.GetPosition(), 800, 600);

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
    m_cameraController.Initialize(spawnPos, 800, 600);

    /* 2. Réinitialisation de l'inventaire et des statistiques d'amélioration */
    m_player.GetInventory().RemoveItem("sword");
    m_player.GetInventory().RemoveItem("boomerang");
    m_player.GetInventory().m_upgradePoints = 5;

    /* 3. Génération des objets au sol par rapport au spawn joueur (assure la rétrocompatibilité) */
    m_entityManager.Clear();
    
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

    m_entityManager.AddPickup(std::move(swordPickup));
    m_entityManager.AddPickup(std::move(boomerangPickup));

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

    Database::Initialize();

    /* 4. Génération des objets destructibles, PNJs, et Ennemis depuis les métadonnées (Data-Driven) */
    for (const auto& spawn : levelOpt.value().spawns)
    {
        /* --- Destructibles --- */
        if (spawn.type == "Crate" || spawn.subType == "Crate")
        {
            m_entityManager.AddDestructible(Destructible(DestructibleType::Crate, spawn.position));
        }
        else if (spawn.type == "Plant" || spawn.subType == "Plant")
        {
            m_entityManager.AddDestructible(Destructible(DestructibleType::Plant, spawn.position));
        }
        else if (spawn.type == "Custom" || spawn.subType == "Custom")
        {
            Destructible custom(DestructibleType::Custom, spawn.position);
            custom.AddVulnerableDamageType(DamageType::Blunt);
            custom.AddVulnerableElement(ElementType::Fire);
            custom.SetMaxHealth(50.0f);
            m_entityManager.AddDestructible(std::move(custom));
        }
        /* --- PNJs --- */
        else if (spawn.type == "Npc")
        {
            std::string npcTypeStr = spawn.GetProperty("npc_type", "Villager");
            std::string npcName = spawn.GetProperty("name", "Villageois Anonyme");
            
            NpcType npcType = NpcType::Villager;
            if (npcTypeStr == "QuestGiver") npcType = NpcType::QuestGiver;
            else if (npcTypeStr == "Merchant") npcType = NpcType::Merchant;

            Vector2 safePos = findSafeSpawn(spawn.position, 32.0f, 32.0f);
            Npc npc(npcName, npcType, safePos);

            /* Configuration du mouvement depuis JSON */
            std::string movementStr = spawn.GetProperty("movement", "Static");
            if (movementStr == "PatrolZone")
            {
                float radius = std::stof(spawn.GetProperty("patrol_radius", "100"));
                float speed = std::stof(spawn.GetProperty("patrol_speed", "40"));
                npc.SetPatrolZone(radius, speed);
            }
            else
            {
                npc.SetStatic();
            }

            /* Chargement des dialogues via la Base de Données Globale */
            std::string templateId = spawn.GetProperty("template_id", "");
            std::vector<std::string> dialogues;
            
            if (!templateId.empty())
            {
                dialogues = Database::GetDialogues(templateId);
            }
            
            /* Surcharge possible par des propriétés locales spécifiques Tiled */
            for (int i = 1; i <= 5; ++i)
            {
                std::string line = spawn.GetProperty("dialogue" + std::to_string(i), "");
                if (!line.empty())
                {
                    if (dialogues.size() < (size_t)i) dialogues.resize(i);
                    dialogues[i - 1] = line;
                }
            }
            
            if (!dialogues.empty())
            {
                npc.SetDefaultDialogues(dialogues);
            }

            /* Configuration des quêtes depuis JSON */
            if (npcType == NpcType::QuestGiver)
            {
                std::string questId = spawn.GetProperty("quest_id", "");
                if (!questId.empty())
                {
                    std::string desc = spawn.GetProperty("quest_desc", "Quête mystère");
                    int kills = std::stoi(spawn.GetProperty("quest_req_kills", "5"));
                    int rupees = std::stoi(spawn.GetProperty("quest_rew_rupees", "50"));
                    int pts = std::stoi(spawn.GetProperty("quest_rew_points", "1"));
                    npc.ConfigureQuest(questId, desc, kills, rupees, pts);
                }
            }

            /* Note: Pour un jeu complet on lirait le catalogue du marchand en JSON, ici on hardcode le catalogue par défaut si marchand */
            if (npcType == NpcType::Merchant)
            {
                npc.AddMerchantItem("heal_potion", "Potion de Sante", 10, "Restaure completement vos coeurs.");
                npc.AddMerchantItem("forge_point", "Infu de Point de Forge", 25, "Ajoute 1 point de forge precieux.");
                npc.AddMerchantItem("boomerang", "Boomerang d'acier", 40, "Arme de jet rotative secondaire.");
            }

            m_entityManager.AddNpc(std::move(npc));
        }
        /* --- Ennemis --- */
        else if (spawn.type == "Enemy")
        {
            std::string enemyTypeStr = spawn.GetProperty("enemy_type", "Slime");
            std::string enemyName = spawn.GetProperty("name", enemyTypeStr);
            
            EnemyTemplate tmpl = Database::GetEnemyTemplate(enemyTypeStr);

            Vector2 safePos = findSafeSpawn(spawn.position, tmpl.width, tmpl.height);
            Enemy enemy(enemyName, tmpl, safePos);

            /* Configuration du mouvement depuis JSON */
            std::string movementStr = spawn.GetProperty("movement", "PatrolZone");
            if (movementStr == "Static")
            {
                enemy.SetStatic();
            }
            else if (movementStr == "PatrolZone")
            {
                float radius = std::stof(spawn.GetProperty("patrol_radius", "80"));
                float speed = std::stof(spawn.GetProperty("patrol_speed", "50"));
                enemy.SetPatrolZone(radius, speed);
            }
            // (La lecture des Waypoints pour DefinedPath pourrait être ajoutée de la même manière)

            m_entityManager.AddEnemy(std::move(enemy));
        }
        /* --- Portails --- */
        else if (spawn.type == "Portal" || spawn.subType == "Portal")
        {
            Portal portal;
            portal.rect = { spawn.position.x - 20.0f, spawn.position.y - 20.0f, 40.0f, 40.0f };
            portal.targetMap = spawn.GetProperty("target_map", "");
            
            float tx = std::stof(spawn.GetProperty("target_x", std::to_string(spawn.targetSpawn.x)));
            float ty = std::stof(spawn.GetProperty("target_y", std::to_string(spawn.targetSpawn.y)));
            portal.targetSpawn = { tx, ty };
            portal.name = spawn.GetProperty("name", "Transition");
            portal.portalId = spawn.GetProperty("portal_id", "");
            portal.targetPortalId = spawn.GetProperty("target_portal_id", "");

            m_entityManager.AddPortal(std::move(portal));
        }
    }

    /* Construit l'index d'optimisation spatiale pour les destructions et collisions */
    m_entityManager.BuildSpatialGrid();

    /* 5. Génération par défaut des PNJ et Ennemis (Fallback si la carte JSON n'en contient pas) */
    if (m_entityManager.GetNpcs().empty() && m_entityManager.GetEnemies().empty())
    {
        // Un villageois sympathique qui patrouille
        Vector2 villagerPos = findSafeSpawn({ spawnPos.x - 120.0f, spawnPos.y + 120.0f }, 32.0f, 32.0f);
        Npc villager("Jean le Villageois", NpcType::Villager, villagerPos);
        villager.SetPatrolZone(100.0f, 40.0f);
        m_entityManager.AddNpc(std::move(villager));

        // Un donneur de quête statique à côté des caisses
        Vector2 questGiverPos = findSafeSpawn({ spawnPos.x + 160.0f, spawnPos.y - 80.0f }, 32.0f, 32.0f);
        Npc questGiver("Bucheron Bourru", NpcType::QuestGiver, questGiverPos);
        questGiver.SetStatic();
        questGiver.ConfigureQuest("crate_hunt", "Detruire les caisses encombrantes", 5, 80, 2);
        m_entityManager.AddNpc(std::move(questGiver));

        // Un marchand ambulant avec des objets à vendre
        Vector2 merchantPos = findSafeSpawn({ spawnPos.x - 180.0f, spawnPos.y - 120.0f }, 32.0f, 32.0f);
        Npc merchant("Marchand Ambulant", NpcType::Merchant, merchantPos);
        merchant.SetStatic();
        merchant.AddMerchantItem("heal_potion", "Potion de Sante", 10, "Restaure completement vos coeurs.");
        merchant.AddMerchantItem("forge_point", "Infu de Point de Forge", 25, "Ajoute 1 point de forge precieux.");
        merchant.AddMerchantItem("boomerang", "Boomerang d'acier", 40, "Arme de jet rotative secondaire.");
        m_entityManager.AddNpc(std::move(merchant));

        // Quelques Slimes patrouilleurs
        Vector2 slime1Pos = findSafeSpawn({ spawnPos.x + 200.0f, spawnPos.y + 200.0f }, 32.0f, 32.0f);
        Enemy slime1("Slime Vert", EnemyType::Slime, slime1Pos);
        slime1.SetPatrolZone(80.0f, 50.0f);
        m_entityManager.AddEnemy(std::move(slime1));

        Vector2 slime2Pos = findSafeSpawn({ spawnPos.x - 200.0f, spawnPos.y + 250.0f }, 32.0f, 32.0f);
        Enemy slime2("Slime Agile", EnemyType::Slime, slime2Pos);
        slime2.SetPatrolZone(60.0f, 70.0f);
        m_entityManager.AddEnemy(std::move(slime2));

        // Un Octorok à distance
        Vector2 octorokPos = findSafeSpawn({ spawnPos.x + 300.0f, spawnPos.y - 150.0f }, 32.0f, 32.0f);
        Enemy octorok("Octorok Rouge", EnemyType::Octorok, octorokPos);
        octorok.SetPatrolZone(50.0f, 30.0f);
        m_entityManager.AddEnemy(std::move(octorok));

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
        m_entityManager.AddEnemy(std::move(moblin));
    }

    m_playerHitCooldown = 0.0f;
    m_currentMapPath = filePath;

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
    
    m_entityManager.Update(deltaTime, m_tileMap, m_player.GetPosition());

    /* 3. Résoudre les déplacements physiques glissants du joueur via PhysicsSystem */
    const Vector2 resolvedPos = PhysicsSystem::ResolvePlayerMovement(m_player, oldPos, m_tileMap, m_entityManager);
    m_player.SetPosition(resolvedPos);

    /* 3.5 Vérification des collisions avec les Portails pour les transitions de cartes */
    for (const auto& portal : m_entityManager.GetPortals())
    {
        if (CheckCollisionRecs(m_player.GetCollisionRect(), portal.rect))
        {
            if (portal.targetMap == "previous" || portal.targetMap == "back")
            {
                if (!m_mapHistory.empty())
                {
                    MapHistory prev = m_mapHistory.back();
                    m_mapHistory.pop_back();
                    
                    if (LoadMap(prev.mapPath))
                    {
                        m_player.SetPosition(prev.spawnPosition);
                        m_cameraController.Initialize(prev.spawnPosition, 800, 600);
                        m_hud.TriggerNotification("Retour a l'exterieur !", 1.5f);
                    }
                }
            }
            else if (!portal.targetMap.empty())
            {
                /* On copie les valeurs localement avant d'appeler LoadMap()
                 * car LoadMap réinitialise l'EntityManager et invalide la référence 'portal' ! */
                std::string tMap = portal.targetMap;
                Vector2 tSpawn = portal.targetSpawn;
                std::string tPortalId = portal.targetPortalId;

                /* Sauvegarder la carte et la position de retour du joueur avant transition */
                MapHistory hist;
                hist.mapPath = m_currentMapPath;
                
                // On recule un tout petit peu la position de retour pour éviter une boucle de téléportation infinie en revenant !
                Vector2 returnPos = oldPos;
                if (m_player.GetDirection() == Direction::Up) returnPos.y += 20.0f;
                else if (m_player.GetDirection() == Direction::Down) returnPos.y -= 20.0f;
                else if (m_player.GetDirection() == Direction::Left) returnPos.x += 20.0f;
                else if (m_player.GetDirection() == Direction::Right) returnPos.x -= 20.0f;

                hist.spawnPosition = returnPos;
                m_mapHistory.push_back(hist);

                if (LoadMap(tMap))
                {
                    Vector2 finalSpawn = tSpawn;
                    bool portalMatchFound = false;

                    /* Si un targetPortalId a été spécifié, on recherche le portail cible sur la nouvelle carte ! */
                    if (!tPortalId.empty())
                    {
                        for (const auto& targetPortal : m_entityManager.GetPortals())
                        {
                            if (targetPortal.portalId == tPortalId)
                            {
                                finalSpawn = {
                                    targetPortal.rect.x + targetPortal.rect.width / 2.0f,
                                    targetPortal.rect.y + targetPortal.rect.height / 2.0f
                                };
                                portalMatchFound = true;
                                break;
                            }
                        }
                    }

                    m_player.SetPosition(finalSpawn);
                    m_cameraController.Initialize(finalSpawn, 800, 600);
                    m_hud.TriggerNotification("Entre dans le batiment !", 1.5f);
                }
            }
            break; /* On ne peut prendre qu'un portail à la fois par frame */
        }
    }

    /* 4. Initier un dialogue si le joueur appuie sur E à proximité d'un PNJ */
    if (IsKeyPressed(KEY_E))
    {
        for (auto& npc : m_entityManager.GetNpcs())
        {
            if (npc.IsPlayerNear(m_player.GetPosition()))
            {
                m_dialogueSystem.StartInteraction(npc, m_player);
                break;
            }
        }
    }

    /* 5. Gérer le combat (Épée, Boomerang, et Dégâts ennemis sur joueur) via CombatSystem */
    CombatSystem::ResolvePlayerSwordAttacks(m_player, m_entityManager);

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

    CombatSystem::ResolvePlayerBoomerangAttacks(m_player, m_boomerang, m_entityManager, m_tileMap, deltaTime);

    CombatSystem::ResolveEnemyDamageToPlayer(m_player, m_entityManager, m_playerHitCooldown, m_hud, deltaTime);

    /* 6. Détection de collecte des objets au sol */
    for (auto& pickup : m_entityManager.GetPickups())
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

    m_cameraController.Update(deltaTime, m_player.GetPosition());
}

void GameWorld::Draw() const
{
    m_cameraController.BeginMode();

    m_tileMap.Draw();

    m_entityManager.Draw();

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

    m_cameraController.EndMode();

    m_hud.Draw(m_player, m_tileMap, m_entityManager.GetDestructibles(), m_entityManager.GetPickups());

    /* Rendu du DialogueSystem en espace écran */
    if (m_dialogueSystem.IsActive())
    {
        m_dialogueSystem.Draw(m_player);
    }
    else
    {
        /* On cherche si un PNJ est proche pour afficher le prompt d'interaction */
        const Npc* nearNpc = nullptr;
        for (const auto& npc : m_entityManager.GetNpcs())
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
