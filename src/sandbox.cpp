#include <raylib.h>
#include "core/player.h"
#include "core/destructible.h"
#include "map/tile_map.h"
#include "map/map_loader.h"
#include <cmath>
#include <filesystem>
#include <vector>
#include <string>

namespace fs = std::filesystem;

struct SandboxBoomerang
{
    Vector2 position;
    Vector2 velocity;
    bool active;
    bool returning;
    float speed;
    float rotation;
    float maxRange;
    Vector2 originPos;
};

struct SandboxGroundPickup
{
    std::string itemId;
    std::string name;
    Vector2 position;
    bool active = true;
};

/* Racine active résolue pour charger les cartes (par défaut relative au dossier de travail) */
std::string g_mapsPathRoot = "assets/maps/sandbox/";

/* Scanner le dossier assets/maps/sandbox avec prise en charge de replis (fallbacks) */
std::vector<std::string> GetSandboxMaps(void)
{
    std::vector<std::string> mapFiles;
    
    /* Chemins à tester (travail, build/, Debug/) */
    std::vector<std::string> pathsToTry = {
        "assets/maps/sandbox",
        "../assets/maps/sandbox",
        "../../assets/maps/sandbox"
    };

    for (const auto& path : pathsToTry)
    {
        try
        {
            if (fs::exists(path) && fs::is_directory(path))
            {
                for (const auto& entry : fs::directory_iterator(path))
                {
                    if (entry.is_regular_file() && entry.path().extension() == ".json")
                    {
                        mapFiles.push_back(entry.path().filename().string());
                    }
                }
                
                if (!mapFiles.empty())
                {
                    g_mapsPathRoot = path + "/";
                    break; /* On a trouvé des cartes dans ce chemin ! */
                }
            }
        }
        catch (const std::exception& e)
        {
            (void)e;
        }
    }

    return mapFiles;
}

/* Fonctions de gestion de la forge (copie isolée pour le Sandbox) */
void UpdateSandboxInventoryMenu(Player& player)
{
    Inventory& inv = player.GetInventory();
    const Vector2 mousePos = GetMousePosition();

    if (inv.m_upgradePoints > 0)
    {
        if (inv.HasItem("sword"))
        {
            Item* sword = inv.GetItem("sword");
            Rectangle dmgBtn = { 510.0f, 175.0f, 110.0f, 24.0f };
            if (CheckCollisionPointRec(mousePos, dmgBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                sword->damage += 5.0f;
                sword->level += 1;
                inv.m_upgradePoints -= 1;
            }
            Rectangle rangeBtn = { 510.0f, 205.0f, 110.0f, 24.0f };
            if (CheckCollisionPointRec(mousePos, rangeBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                sword->range += 8.0f;
                sword->level += 1;
                inv.m_upgradePoints -= 1;
            }
            Rectangle elemBtn = { 510.0f, 235.0f, 110.0f, 24.0f };
            if (CheckCollisionPointRec(mousePos, elemBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                int currentElem = (int)sword->element;
                sword->element = (ElementType)((currentElem + 1) % 4);
                sword->level += 1;
                inv.m_upgradePoints -= 1;
            }
        }

        if (inv.HasItem("boomerang"))
        {
            Item* boom = inv.GetItem("boomerang");
            Rectangle dmgBtn = { 510.0f, 355.0f, 110.0f, 24.0f };
            if (CheckCollisionPointRec(mousePos, dmgBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                boom->damage += 2.5f;
                boom->level += 1;
                inv.m_upgradePoints -= 1;
            }
            Rectangle speedBtn = { 510.0f, 385.0f, 110.0f, 24.0f };
            if (CheckCollisionPointRec(mousePos, speedBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                boom->speed += 50.0f;
                boom->level += 1;
                inv.m_upgradePoints -= 1;
            }
            Rectangle rangeBtn = { 510.0f, 415.0f, 110.0f, 24.0f };
            if (CheckCollisionPointRec(mousePos, rangeBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                boom->range += 30.0f;
                boom->level += 1;
                inv.m_upgradePoints -= 1;
            }
            Rectangle elemBtn = { 510.0f, 445.0f, 110.0f, 24.0f };
            if (CheckCollisionPointRec(mousePos, elemBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                int currentElem = (int)boom->element;
                boom->element = (ElementType)((currentElem + 1) % 4);
                boom->level += 1;
                inv.m_upgradePoints -= 1;
            }
        }
    }
}

void DrawSandboxInventoryMenu(const Player& player)
{
    const Inventory& inv = player.GetInventory();
    const Vector2 mousePos = GetMousePosition();

    Rectangle panel = { 100, 50, 600, 500 };
    DrawRectangleRec(panel, { 10, 15, 25, 255 });
    DrawRectangleLinesEx(panel, 2.0f, SKYBLUE);

    DrawText("FORGE DU BAC A SABLE (SANDBOX)", 400 - MeasureText("FORGE DU BAC A SABLE (SANDBOX)", 20) / 2, 75, 20, SKYBLUE);
    DrawText("Points de forge : " , 280, 120, 16, RAYWHITE);
    DrawText(TextFormat("%d", inv.m_upgradePoints), 280 + MeasureText("Points de forge : ", 16), 120, 16, (inv.m_upgradePoints > 0) ? GREEN : RED);

    /* Épée */
    Rectangle swordBox = { 130, 165, 540, 110 };
    DrawRectangleRec(swordBox, { 20, 25, 35, 255 });
    DrawRectangleLinesEx(swordBox, 1.0f, inv.HasItem("sword") ? GREEN : DARKGRAY);

    if (inv.HasItem("sword"))
    {
        const Item* sword = inv.GetItem("sword");
        DrawText(TextFormat("%s (Nv. %d)", sword->name.c_str(), sword->level), 150, 175, 15, GREEN);
        
        const char* dmgTypeName = "Inconnu";
        if (sword->damageType == DamageType::Piercing)
        {
            dmgTypeName = "Percant";
        }
        else if (sword->damageType == DamageType::Slashing)
        {
            dmgTypeName = "Tranchant";
        }
        else if (sword->damageType == DamageType::Blunt)
        {
            dmgTypeName = "Contondant";
        }
        DrawText(TextFormat("Degats : %0.1f (%s)", sword->damage, dmgTypeName), 150, 200, 13, LIGHTGRAY);
        DrawText(TextFormat("Portee : %0.1f px", sword->range), 150, 220, 13, LIGHTGRAY);
        
        const char* elemName = "Aucun";
        Color elemColor = WHITE;
        if (sword->element == ElementType::Fire) { elemName = "Feu"; elemColor = ORANGE; }
        else if (sword->element == ElementType::Ice) { elemName = "Glace"; elemColor = SKYBLUE; }
        else if (sword->element == ElementType::Lightning) { elemName = "Foudre"; elemColor = GOLD; }
        DrawText(TextFormat("Element : %s", elemName), 150, 240, 13, elemColor);

        if (inv.m_upgradePoints > 0)
        {
            Rectangle btns[] = {
                { 510.0f, 175.0f, 140.0f, 22.0f },
                { 510.0f, 205.0f, 140.0f, 22.0f },
                { 510.0f, 235.0f, 140.0f, 22.0f }
            };
            const char* btnLabels[] = { "Ameliorer (+5)", "Allonger (+8)", "Changer d'Elmt" };

            for (int i = 0; i < 3; ++i)
            {
                const bool hover = CheckCollisionPointRec(mousePos, btns[i]);
                DrawRectangleRec(btns[i], hover ? GREEN : Color{ 35, 75, 45, 255 });
                DrawRectangleLinesEx(btns[i], 1.0f, hover ? WHITE : DARKGRAY);
                DrawText(btnLabels[i], (int)btns[i].x + (int)btns[i].width / 2 - MeasureText(btnLabels[i], 11) / 2, (int)btns[i].y + 5, 11, hover ? BLACK : RAYWHITE);
            }
        }
    }
    else
    {
        DrawText("??? (EPEE NON COLLECTEE)", 150, 210, 15, DARKGRAY);
    }

    /* Boomerang */
    Rectangle boomBox = { 130, 345, 540, 140 };
    DrawRectangleRec(boomBox, { 20, 25, 35, 255 });
    DrawRectangleLinesEx(boomBox, 1.0f, inv.HasItem("boomerang") ? SKYBLUE : DARKGRAY);

    if (inv.HasItem("boomerang"))
    {
        const Item* boom = inv.GetItem("boomerang");
        DrawText(TextFormat("%s (Nv. %d)", boom->name.c_str(), boom->level), 150, 355, 15, SKYBLUE);
        
        const char* dmgTypeName = "Inconnu";
        if (boom->damageType == DamageType::Piercing)
        {
            dmgTypeName = "Percant";
        }
        else if (boom->damageType == DamageType::Slashing)
        {
            dmgTypeName = "Tranchant";
        }
        else if (boom->damageType == DamageType::Blunt)
        {
            dmgTypeName = "Contondant";
        }
        DrawText(TextFormat("Degats : %0.1f (%s)", boom->damage, dmgTypeName), 150, 380, 13, LIGHTGRAY);
        DrawText(TextFormat("Vitesse: %0.1f px/s", boom->speed), 150, 400, 13, LIGHTGRAY);
        DrawText(TextFormat("Portee : %0.1f px", boom->range), 150, 420, 13, LIGHTGRAY);

        const char* elemName = "Aucun";
        Color elemColor = WHITE;
        if (boom->element == ElementType::Fire) { elemName = "Feu"; elemColor = ORANGE; }
        else if (boom->element == ElementType::Ice) { elemName = "Glace"; elemColor = SKYBLUE; }
        else if (boom->element == ElementType::Lightning) { elemName = "Foudre"; elemColor = GOLD; }
        DrawText(TextFormat("Element : %s", elemName), 150, 440, 13, elemColor);

        if (inv.m_upgradePoints > 0)
        {
            Rectangle btns[] = {
                { 510.0f, 355.0f, 140.0f, 22.0f },
                { 510.0f, 385.0f, 140.0f, 22.0f },
                { 510.0f, 415.0f, 140.0f, 22.0f },
                { 510.0f, 445.0f, 140.0f, 22.0f }
            };
            const char* btnLabels[] = { "Ameliorer (+2.5)", "Accelerer (+50)", "Allonger (+30)", "Changer d'Elmt" };

            for (int i = 0; i < 4; ++i)
            {
                const bool hover = CheckCollisionPointRec(mousePos, btns[i]);
                DrawRectangleRec(btns[i], hover ? SKYBLUE : Color{ 35, 55, 75, 255 });
                DrawRectangleLinesEx(btns[i], 1.0f, hover ? WHITE : DARKGRAY);
                DrawText(btnLabels[i], (int)btns[i].x + (int)btns[i].width / 2 - MeasureText(btnLabels[i], 10) / 2, (int)btns[i].y + 5, 10, hover ? BLACK : RAYWHITE);
            }
        }
    }
    else
    {
        DrawText("??? (BOOMERANG NON COLLECTE)", 150, 400, 15, DARKGRAY);
    }

    DrawText("Pressez [ I ] ou [ SELECT ] pour quitter la forge", 400 - MeasureText("Pressez [ I ] ou [ SELECT ] pour quitter la forge", 11) / 2, 515, 11, GRAY);
}

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "gameFormation - Sandbox Gameplay : Boomerang Prototype");

    TileMap tileMap;
    Player player({ 0.0f, 0.0f });

    /* Configuration de la Caméra 2D pour le Sandbox */
    Camera2D camera = { 0 };
    camera.target = player.GetPosition();
    camera.offset = { (float)screenWidth / 2.0f, (float)screenHeight / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.2f;

    SandboxBoomerang boomerang = {
        { 0.0f, 0.0f },
        { 0.0f, 0.0f },
        false,
        false,
        350.0f,
        0.0f,
        150.0f,
        { 0.0f, 0.0f }
    };

    /* Variables de contrôle de l'écran de sélection de carte */
    bool isMapLoaded = false;
    std::vector<std::string> availableMaps = GetSandboxMaps();
    int selectedMapIdx = 0;
    std::string currentLoadedMapName = "";

    /* Objets au sol dans le Sandbox */
    std::vector<SandboxGroundPickup> sandboxPickups;
    std::vector<Destructible> sandboxDestructibles;
    bool isInventoryOpen = false;

    /* Notification */
    std::string notificationText = "";
    float notificationTimer = 0.0f;

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        const float deltaTime = GetFrameTime();

        if (false == isMapLoaded)
        {
            /* -------------------------------------------------------------
             * ÉCRAN DE SÉLECTION DE CARTE
             * ------------------------------------------------------------- */
            if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
            {
                if (!availableMaps.empty())
                {
                    selectedMapIdx = (selectedMapIdx + 1) % (int)availableMaps.size();
                }
            }
            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
            {
                if (!availableMaps.empty())
                {
                    selectedMapIdx = (selectedMapIdx - 1 + (int)availableMaps.size()) % (int)availableMaps.size();
                }
            }

            const Vector2 mousePos = GetMousePosition();
            for (size_t i = 0; i < availableMaps.size(); ++i)
            {
                Rectangle optionRect = {
                    (float)screenWidth / 2.0f - 200.0f,
                    200.0f + (float)i * 50.0f,
                    400.0f,
                    40.0f
                };

                if (CheckCollisionPointRec(mousePos, optionRect))
                {
                    selectedMapIdx = (int)i;
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                    {
                        const std::string selectedFile = g_mapsPathRoot + availableMaps[i];
                        auto loadedLevel = MapLoader::LoadFromJson(selectedFile);
                        if (loadedLevel.has_value())
                        {
                            tileMap.LoadLevel(loadedLevel.value());
                            currentLoadedMapName = availableMaps[i];
                            
                            Vector2 spawnPos = { (float)TileMap::kTileSize * 5.0f, (float)TileMap::kTileSize * 5.0f };
                            for (const auto& s : loadedLevel.value().spawns)
                            {
                                if (s.type == "PlayerSpawn" || s.subType == "PlayerSpawn")
                                {
                                    spawnPos = s.position;
                                    break;
                                }
                            }
                            player.SetPosition(spawnPos);
                            camera.target = spawnPos; /* Aligne instantanément la caméra ! */

                            /* Remettre l'inventaire à zéro et positionner deux pickups au sol */
                            player.GetInventory().RemoveItem("sword");
                            player.GetInventory().RemoveItem("boomerang");
                            player.GetInventory().m_upgradePoints = 5;

                            sandboxPickups.clear();
                            SandboxGroundPickup s1 = { "sword", "EPEE EN BOIS", { spawnPos.x - 80.0f, spawnPos.y }, true };
                            SandboxGroundPickup s2 = { "boomerang", "BOOMERANG DE TEST", { spawnPos.x + 80.0f, spawnPos.y }, true };
                            sandboxPickups.push_back(s1);
                            sandboxPickups.push_back(s2);

                            /* Repopuler les objets destructibles */
                            sandboxDestructibles.clear();
                            Destructible crate1(DestructibleType::Crate, { spawnPos.x - 40.0f, spawnPos.y + 40.0f });
                            Destructible plant1(DestructibleType::Plant, { spawnPos.x + 40.0f, spawnPos.y + 40.0f });
                            
                            /* Monument magique Custom : vulnérable à Blunt OU au Feu (Fire) */
                            Destructible customObj(DestructibleType::Custom, { spawnPos.x, spawnPos.y - 60.0f });
                            customObj.AddVulnerableDamageType(DamageType::Blunt);
                            customObj.AddVulnerableElement(ElementType::Fire);
                            customObj.SetMaxHealth(50.0f);
                            
                            sandboxDestructibles.push_back(crate1);
                            sandboxDestructibles.push_back(plant1);
                            sandboxDestructibles.push_back(customObj);

                            isMapLoaded = true;
                            isInventoryOpen = false;
                            notificationTimer = 0.0f;
                        }
                    }
                }
            }

            if ((IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) && !availableMaps.empty())
            {
                const std::string selectedFile = g_mapsPathRoot + availableMaps[selectedMapIdx];
                auto loadedLevel = MapLoader::LoadFromJson(selectedFile);
                if (loadedLevel.has_value())
                {
                    tileMap.LoadLevel(loadedLevel.value());
                    currentLoadedMapName = availableMaps[selectedMapIdx];

                    Vector2 spawnPos = { (float)TileMap::kTileSize * 5.0f, (float)TileMap::kTileSize * 5.0f };
                    for (const auto& s : loadedLevel.value().spawns)
                    {
                        if (s.type == "PlayerSpawn" || s.subType == "PlayerSpawn")
                        {
                            spawnPos = s.position;
                            break;
                        }
                    }
                    player.SetPosition(spawnPos);
                    camera.target = spawnPos; /* Aligne instantanément la caméra ! */

                    player.GetInventory().RemoveItem("sword");
                    player.GetInventory().RemoveItem("boomerang");
                    player.GetInventory().m_upgradePoints = 5;

                    sandboxPickups.clear();
                    SandboxGroundPickup s1 = { "sword", "EPEE EN BOIS", { spawnPos.x - 80.0f, spawnPos.y }, true };
                    SandboxGroundPickup s2 = { "boomerang", "BOOMERANG DE TEST", { spawnPos.x + 80.0f, spawnPos.y }, true };
                    sandboxPickups.push_back(s1);
                    sandboxPickups.push_back(s2);

                    /* Repopuler les objets destructibles */
                    sandboxDestructibles.clear();
                    Destructible crate1(DestructibleType::Crate, { spawnPos.x - 40.0f, spawnPos.y + 40.0f });
                    Destructible plant1(DestructibleType::Plant, { spawnPos.x + 40.0f, spawnPos.y + 40.0f });
                    
                    /* Monument magique Custom : vulnérable à Blunt OU au Feu (Fire) */
                    Destructible customObj(DestructibleType::Custom, { spawnPos.x, spawnPos.y - 60.0f });
                    customObj.AddVulnerableDamageType(DamageType::Blunt);
                    customObj.AddVulnerableElement(ElementType::Fire);
                    customObj.SetMaxHealth(50.0f);
                    
                    sandboxDestructibles.push_back(crate1);
                    sandboxDestructibles.push_back(plant1);
                    sandboxDestructibles.push_back(customObj);

                    isMapLoaded = true;
                    isInventoryOpen = false;
                    notificationTimer = 0.0f;
                }
            }

            BeginDrawing();
            ClearBackground({ 10, 20, 30, 255 });

            DrawText("BAC A SABLE GAMEPLAY - CHOIX DE LA MAP", screenWidth / 2 - MeasureText("BAC A SABLE GAMEPLAY - CHOIX DE LA MAP", 20) / 2, 80, 20, SKYBLUE);
            DrawText("Veuillez selectionner une carte .json pour lancer le sandbox :", screenWidth / 2 - MeasureText("Veuillez selectionner une carte .json pour lancer le sandbox :", 14) / 2, 130, 14, LIGHTGRAY);

            if (availableMaps.empty())
            {
                const std::string errorText = "Aucune carte trouvee dans " + g_mapsPathRoot;
                DrawText(errorText.c_str(), screenWidth / 2 - MeasureText(errorText.c_str(), 14) / 2, 250, 14, RED);
            }
            else
            {
                for (size_t i = 0; i < availableMaps.size(); ++i)
                {
                    Rectangle optionRect = {
                        (float)screenWidth / 2.0f - 200.0f,
                        200.0f + (float)i * 50.0f,
                        400.0f,
                        40.0f
                    };

                    const bool isSelected = (selectedMapIdx == (int)i);
                    Color boxColor = isSelected ? Color{ 30, 60, 90, 255 } : Color{ 15, 25, 35, 255 };
                    Color borderColor = isSelected ? SKYBLUE : DARKGRAY;
                    Color textColor = isSelected ? RAYWHITE : LIGHTGRAY;

                    DrawRectangleRec(optionRect, boxColor);
                    DrawRectangleLinesEx(optionRect, 1.5f, borderColor);

                    if (isSelected)
                    {
                        DrawTriangle(
                            { optionRect.x + 10.0f, optionRect.y + 12.0f },
                            { optionRect.x + 10.0f, optionRect.y + 28.0f },
                            { optionRect.x + 22.0f, optionRect.y + 20.0f },
                            SKYBLUE
                        );
                    }

                    const int textWidth = MeasureText(availableMaps[i].c_str(), 14);
                    DrawText(availableMaps[i].c_str(), (int)optionRect.x + (int)optionRect.width / 2 - textWidth / 2, (int)optionRect.y + 13, 14, textColor);
                }
            }

            DrawText("ZQSD / Fleches : Naviguer, Entree / Clic : Charger la carte", screenWidth / 2 - MeasureText("ZQSD / Fleches : Naviguer, Entree / Clic : Charger la carte", 11) / 2, screenHeight - 60, 11, GRAY);
            EndDrawing();
        }
        else
        {
            /* -------------------------------------------------------------
             * ÉCRAN DE JEU (GAMEPLAY DU SANDBOX AVEC MAP DYNAMIQUE)
             * ------------------------------------------------------------- */
            bool toggleInventory = IsKeyPressed(KEY_I);
            if (IsGamepadAvailable(0))
            {
                if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_LEFT))
                {
                    toggleInventory = true;
                }
            }

            if (toggleInventory)
            {
                isInventoryOpen = !isInventoryOpen;
            }

            if (isInventoryOpen)
            {
                UpdateSandboxInventoryMenu(player);
            }
            else
            {
                const Vector2 oldPos = player.GetPosition();

                /* Update Joueur */
                player.Update(deltaTime);

                /* Collisions Joueur / Murs */
                const Vector2 currentPos = player.GetPosition();
                const Rectangle collisionRect = player.GetCollisionRect();
                if (tileMap.CheckCollision(collisionRect))
                {
                    const Vector2 resolvedPos = tileMap.ResolveCollision(currentPos, oldPos, collisionRect.width, collisionRect.height);
                    player.SetPosition(resolvedPos);
                }

                /* Mettre à jour les objets destructibles */
                for (auto& dest : sandboxDestructibles)
                {
                    dest.Update(deltaTime);
                }

                /* Collisions glissantes contre les objets destructibles physiques solides */
                const Vector2 posAfterTileCheck = player.GetPosition();
                Vector2 finalPos = posAfterTileCheck;

                /* Essai sur l'axe X */
                player.SetPosition({ posAfterTileCheck.x, oldPos.y });
                bool collideX = false;
                for (const auto& dest : sandboxDestructibles)
                {
                    if (dest.IsAlive() && CheckCollisionRecs(player.GetCollisionRect(), dest.GetCollisionRect()))
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
                player.SetPosition({ finalPos.x, posAfterTileCheck.y });
                bool collideY = false;
                for (const auto& dest : sandboxDestructibles)
                {
                    if (dest.IsAlive() && CheckCollisionRecs(player.GetCollisionRect(), dest.GetCollisionRect()))
                    {
                        collideY = true;
                        break;
                    }
                }
                if (collideY)
                {
                    finalPos.y = oldPos.y;
                }

                player.SetPosition(finalPos);

                /* Détection des attaques à l'épée sur les objets destructibles */
                if (player.GetState() == PlayerState::Attacking)
                {
                    const Rectangle attackRect = player.GetAttackRect();
                    const Item* sword = player.GetInventory().GetItem("sword");
                    if (sword != nullptr && sword->collected)
                    {
                        for (auto& dest : sandboxDestructibles)
                        {
                            if (dest.IsAlive() && CheckCollisionRecs(attackRect, dest.GetCollisionRect()))
                            {
                                dest.TakeDamage(sword->damage, sword->damageType, sword->element);
                            }
                        }
                    }
                }

                /* Collecte de Pickups sol */
                for (auto& p : sandboxPickups)
                {
                    if (p.active)
                    {
                        Rectangle pickupRect = { p.position.x - 12.0f, p.position.y - 12.0f, 24.0f, 24.0f };
                        if (CheckCollisionRecs(collisionRect, pickupRect))
                        {
                            p.active = false;
                            player.GetInventory().AddItem(p.itemId);
                            notificationText = "Sandbox : " + p.name + " collecte !";
                            notificationTimer = 3.0f;
                        }
                    }
                }

                if (notificationTimer > 0.0f)
                {
                    notificationTimer -= deltaTime;
                }

                /* Retour à l'écran de sélection de map avec ECHAP */
                if (IsKeyPressed(KEY_ESCAPE))
                {
                    boomerang.active = false;
                    isMapLoaded = false;
                    availableMaps = GetSandboxMaps();
                }

                /* Boomerang - Uniquement si collecté et équipé ! */
                if (player.GetInventory().HasItem("boomerang"))
                {
                    const Item* boomStats = player.GetInventory().GetItem("boomerang");
                    
                    bool isBoomerangPressed = IsKeyPressed(KEY_B);
                    if (IsGamepadAvailable(0))
                    {
                        if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT) || 
                            IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_UP))
                        {
                            isBoomerangPressed = true;
                        }
                    }

                    if (isBoomerangPressed && !boomerang.active && player.GetState() != PlayerState::Attacking)
                    {
                        boomerang.active = true;
                        boomerang.returning = false;
                        boomerang.position = player.GetPosition();
                        boomerang.originPos = player.GetPosition();

                        /* Charge de manière dynamique les stats améliorées ! */
                        boomerang.speed = (boomStats != nullptr) ? boomStats->speed : 350.0f;
                        boomerang.maxRange = (boomStats != nullptr) ? boomStats->range : 150.0f;

                        boomerang.velocity = { 0.0f, 0.0f };
                        if (player.GetDirection() == Direction::Up)
                        {
                            boomerang.velocity.y = -boomerang.speed;
                        }
                        else if (player.GetDirection() == Direction::Down)
                        {
                            boomerang.velocity.y = boomerang.speed;
                        }
                        else if (player.GetDirection() == Direction::Left)
                        {
                            boomerang.velocity.x = -boomerang.speed;
                        }
                        else if (player.GetDirection() == Direction::Right)
                        {
                            boomerang.velocity.x = boomerang.speed;
                        }
                    }
                }

                if (boomerang.active)
                {
                    boomerang.rotation += 720.0f * deltaTime;

                    if (!boomerang.returning)
                    {
                        boomerang.position.x += boomerang.velocity.x * deltaTime;
                        boomerang.position.y += boomerang.velocity.y * deltaTime;

                        const float dx = boomerang.position.x - boomerang.originPos.x;
                        const float dy = boomerang.position.y - boomerang.originPos.y;
                        const float distance = std::sqrt((dx * dx) + (dy * dy));

                        const Rectangle boomRect = { boomerang.position.x - 8.0f, boomerang.position.y - 8.0f, 16.0f, 16.0f };
                        
                        /* Collision du boomerang contre les objets destructibles */
                        const Item* boomStats = player.GetInventory().GetItem("boomerang");
                        if (boomStats != nullptr)
                        {
                            for (auto& dest : sandboxDestructibles)
                            {
                                if (dest.IsAlive() && CheckCollisionRecs(boomRect, dest.GetCollisionRect()))
                                {
                                    if (dest.TakeDamage(boomStats->damage, boomStats->damageType, boomStats->element))
                                    {
                                        boomerang.returning = true;
                                        break;
                                    }
                                }
                            }
                        }

                        if (distance >= boomerang.maxRange || tileMap.CheckCollision(boomRect))
                        {
                            boomerang.returning = true;
                        }
                    }
                    else
                    {
                        const Vector2 playerPos = player.GetPosition();
                        const Vector2 dirToPlayer = { playerPos.x - boomerang.position.x, playerPos.y - boomerang.position.y };
                        const float length = std::sqrt((dirToPlayer.x * dirToPlayer.x) + (dirToPlayer.y * dirToPlayer.y));

                        if (length <= 20.0f)
                        {
                            boomerang.active = false;
                        }
                        else
                        {
                            boomerang.position.x += (dirToPlayer.x / length) * boomerang.speed * deltaTime;
                            boomerang.position.y += (dirToPlayer.y / length) * boomerang.speed * deltaTime;
                        }
                    }
                }

                /* Mise à jour fluide de la Caméra 2D (Lerp) */
                const Vector2 playerPos = player.GetPosition();
                camera.target.x += (playerPos.x - camera.target.x) * 0.1f;
                camera.target.y += (playerPos.y - camera.target.y) * 0.1f;
            }

            /* Rendu */
            BeginDrawing();
            ClearBackground(DARKBLUE);

            /* Tout ce qui est dans l'espace monde se dessine par rapport à la caméra */
            BeginMode2D(camera);
                tileMap.Draw();

                /* Dessiner les objets destructibles */
                for (const auto& dest : sandboxDestructibles)
                {
                    dest.Draw();
                }

                /* Dessiner les pickups au sol */
                for (const auto& p : sandboxPickups)
                {
                    if (p.active)
                    {
                        if (p.itemId == "sword")
                        {
                            DrawLineEx({ p.position.x - 8, p.position.y + 8 }, { p.position.x + 8, p.position.y - 8 }, 3.5f, LIGHTGRAY);
                            DrawLineEx({ p.position.x - 10, p.position.y + 10 }, { p.position.x - 6, p.position.y + 6 }, 4.0f, BROWN);
                            DrawCircleV({ p.position.x + 8, p.position.y - 8 }, 2.5f, WHITE);
                        }
                        else if (p.itemId == "boomerang")
                        {
                            DrawCircleSector(p.position, 10.0f, 45.0f, 225.0f, 4, SKYBLUE);
                            DrawCircleLinesV(p.position, 10.0f, WHITE);
                        }
                    }
                }

                player.Draw();

                /* Rendu visuel du boomerang dynamique de test */
                if (boomerang.active)
                {
                    Color boomColor = SKYBLUE;
                    const Item* boomStats = player.GetInventory().GetItem("boomerang");
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

                    DrawCircleSector(boomerang.position, 12.0f, boomerang.rotation, boomerang.rotation + 180.0f, 4, boomColor);
                    DrawCircleLinesV(boomerang.position, 12.0f, WHITE);
                }
            EndMode2D();

            /* L'UI et les bannières se dessinent hors caméra (en coordonnées écran statiques) */
            /* Notification bannière */
            if (notificationTimer > 0.0f && !isInventoryOpen)
            {
                const int textWidth = MeasureText(notificationText.c_str(), 16);
                DrawRectangle(400 - (textWidth + 30) / 2, 40, textWidth + 30, 32, Fade(BLACK, 0.85f));
                DrawRectangleLines(400 - (textWidth + 30) / 2, 40, textWidth + 30, 32, SKYBLUE);
                DrawText(notificationText.c_str(), 400 - textWidth / 2, 49, 16, SKYBLUE);
            }

            /* Si l'inventaire de la forge est ouvert, on le dessine par-dessus */
            if (isInventoryOpen)
            {
                DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.82f));
                DrawSandboxInventoryMenu(player);
            }
            else
            {
                /* HUD du Sandbox */
                DrawRectangle(10, 10, 350, 150, Fade(BLACK, 0.8f));
                DrawRectangleLines(10, 10, 350, 150, SKYBLUE);
                DrawText("SANDBOX - TEST DU BOOMERANG DYNAMIQUE", 20, 20, 12, SKYBLUE);
                DrawText(TextFormat("Carte active : %s", currentLoadedMapName.c_str()), 20, 40, 11, GREEN);
                DrawText("ZQSD / Fleches : Se deplacer", 20, 55, 11, LIGHTGRAY);
                DrawText("Marchez sur l'EPEE et le BOOMERANG pour les ramasser !", 20, 72, 11, GOLD);
                DrawText("Touche [ ESPACE ] : Attaque (si EPEE collectee)", 20, 90, 11, LIGHTGRAY);
                DrawText("Touche [ B ] : Boomerang (si BOOMERANG collecte)", 20, 105, 11, LIGHTGRAY);
                DrawText("Touche [ I ] / [ SELECT ] : Ouvrir la FORGE & AMELIORATIONS", 20, 122, 11, SKYBLUE);
                DrawText("Touche [ ECHAP ] : Choisir une autre carte", 20, 137, 11, YELLOW);

                DrawFPS(screenWidth - 100, 10);
            }

            EndDrawing();
        }
    }

    CloseWindow();
    return 0;
}
