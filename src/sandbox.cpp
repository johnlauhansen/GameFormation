#include <raylib.h>
#include "core/player/player.h"
#include "core/game_world.h"
#include <cmath>
#include <filesystem>
#include <vector>
#include <string>

namespace fs = std::filesystem;

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
    const Vector2 rawMousePos = GetMousePosition();
    const Vector2 mousePos = {
        rawMousePos.x * (800.0f / GetScreenWidth()),
        rawMousePos.y * (600.0f / GetScreenHeight())
    };

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
    const Vector2 rawMousePos = GetMousePosition();
    const Vector2 mousePos = {
        rawMousePos.x * (800.0f / GetScreenWidth()),
        rawMousePos.y * (600.0f / GetScreenHeight())
    };

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
    /* Activer la fenêtre redimensionnable */
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    /* Initialisation de la fenêtre réelle (adaptée pour écran 15 pouces) */
    const int screenWidth = 1600;
    const int screenHeight = 1200;
    InitWindow(screenWidth, screenHeight, "gameFormation - Sandbox Gameplay : Boomerang Prototype");

    /* Écran virtuel interne de 800x600 pour la mise à l'échelle retro */
    RenderTexture2D target = LoadRenderTexture(800, 600);

    GameWorld sandboxWorld;

    /* Variables de contrôle de l'écran de sélection de carte */
    bool isMapLoaded = false;
    std::vector<std::string> availableMaps = GetSandboxMaps();
    int selectedMapIdx = 0;
    std::string currentLoadedMapName = "";

    bool isInventoryOpen = false;

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

            const Vector2 rawMousePos = GetMousePosition();
            const Vector2 mousePos = {
                rawMousePos.x * (800.0f / GetScreenWidth()),
                rawMousePos.y * (600.0f / GetScreenHeight())
            };
            for (size_t i = 0; i < availableMaps.size(); ++i)
            {
                Rectangle optionRect = {
                    (float)800.0f / 2.0f - 200.0f,
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
                        if (sandboxWorld.LoadMap(selectedFile))
                        {
                            currentLoadedMapName = availableMaps[i];
                            isMapLoaded = true;
                            isInventoryOpen = false;
                        }
                    }
                }
            }

            if ((IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) && !availableMaps.empty())
            {
                const std::string selectedFile = g_mapsPathRoot + availableMaps[selectedMapIdx];
                if (sandboxWorld.LoadMap(selectedFile))
                {
                    currentLoadedMapName = availableMaps[selectedMapIdx];
                    isMapLoaded = true;
                    isInventoryOpen = false;
                }
            }

            /* Rendu de l'écran de sélection de carte sur la texture virtuelle */
            BeginTextureMode(target);
            ClearBackground({ 10, 20, 30, 255 });

            DrawText("BAC A SABLE GAMEPLAY - CHOIX DE LA MAP", 800 / 2 - MeasureText("BAC A SABLE GAMEPLAY - CHOIX DE LA MAP", 20) / 2, 80, 20, SKYBLUE);
            DrawText("Veuillez selectionner une carte .json pour lancer le sandbox :", 800 / 2 - MeasureText("Veuillez selectionner une carte .json pour lancer le sandbox :", 14) / 2, 130, 14, LIGHTGRAY);

            if (availableMaps.empty())
            {
                const std::string errorText = "Aucune carte trouvee dans " + g_mapsPathRoot;
                DrawText(errorText.c_str(), 800 / 2 - MeasureText(errorText.c_str(), 14) / 2, 250, 14, RED);
            }
            else
            {
                for (size_t i = 0; i < availableMaps.size(); ++i)
                {
                    Rectangle optionRect = {
                        (float)800.0f / 2.0f - 200.0f,
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

            DrawText("ZQSD / Fleches : Naviguer, Entree / Clic : Charger la carte", 800 / 2 - MeasureText("ZQSD / Fleches : Naviguer, Entree / Clic : Charger la carte", 11) / 2, 600 - 60, 11, GRAY);
            EndTextureMode();
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
                UpdateSandboxInventoryMenu(sandboxWorld.GetPlayer());
            }
            else
            {
                /* Tout l'update du joueur, de la caméra, des destructibles, pickups et du boomerang est centralisé ! */
                sandboxWorld.Update(deltaTime);

                /* Retour à l'écran de sélection de map avec ECHAP */
                if (IsKeyPressed(KEY_ESCAPE))
                {
                    isMapLoaded = false;
                    availableMaps = GetSandboxMaps();
                }
            }

            /* Rendu du jeu sandbox sur la texture virtuelle */
            BeginTextureMode(target);
            ClearBackground(DARKBLUE);

            /* Le rendu de la carte, des objets, des destructibles, du joueur et du boomerang est centralisé ! */
            sandboxWorld.Draw();

            /* Si l'inventaire de la forge est ouvert, on le dessine par-dessus */
            if (isInventoryOpen)
            {
                DrawRectangle(0, 0, 800, 600, Fade(BLACK, 0.82f));
                DrawSandboxInventoryMenu(sandboxWorld.GetPlayer());
            }
            else
            {
                /* HUD du Sandbox */
                DrawRectangle(10, 10, 350, 150, Fade(BLACK, 0.8f));
                DrawRectangleLines(10, 10, 350, 150, SKYBLUE);
                DrawText("SANDBOX - TEST DU BOOMERANG DYNAMIQUE (UNIFIE)", 20, 20, 12, SKYBLUE);
                DrawText(TextFormat("Carte active : %s", currentLoadedMapName.c_str()), 20, 40, 11, GREEN);
                DrawText("ZQSD / Fleches : Se deplacer", 20, 55, 11, LIGHTGRAY);
                DrawText("Marchez sur l'EPEE et le BOOMERANG pour les ramasser !", 20, 72, 11, GOLD);
                DrawText("Touche [ ESPACE ] : Attaque (si EPEE collectee)", 20, 90, 11, LIGHTGRAY);
                DrawText("Touche [ B ] : Boomerang (si BOOMERANG collecte)", 20, 105, 11, LIGHTGRAY);
                DrawText("Touche [ I ] / [ SELECT ] : Ouvrir la FORGE & AMELIORATIONS", 20, 122, 11, SKYBLUE);
                DrawText("Touche [ ECHAP ] : Choisir une autre carte", 20, 137, 11, YELLOW);

                DrawFPS(800 - 100, 10);
            }
            EndTextureMode();
        }

        /* Rendu final : Mise à l'échelle sur l'écran réel */
        BeginDrawing();
        ClearBackground(BLACK);
        DrawTexturePro(target.texture,
                       Rectangle{ 0.0f, 0.0f, (float)target.texture.width, -(float)target.texture.height },
                       Rectangle{ 0.0f, 0.0f, (float)GetScreenWidth(), (float)GetScreenHeight() },
                       Vector2{ 0.0f, 0.0f },
                       0.0f,
                       WHITE);
        EndDrawing();
    }

    UnloadRenderTexture(target);
    CloseWindow();
    return 0;
}
