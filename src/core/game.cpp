#include "game.h"
#include "game_world.h"
#include <cmath>

Game::Game()
    : m_currentScreen(GameScreen::Title)
    , m_shouldKeepRunning(true)
    , m_world(std::make_unique<GameWorld>())
    , m_selectedTitleOption(0)
    , m_titlePulseTimer(0.0f)
    , m_soundVolumePercent(100)
    , m_controlsShown(true)
    , m_isInventoryOpen(false)
{
}

Game::~Game()
{
}

void Game::Run()
{
    while (m_shouldKeepRunning && !WindowShouldClose())
    {
        const float deltaTime = GetFrameTime();
        Update(deltaTime);
        Draw();
    }
}

void Game::Update(float deltaTime)
{
    switch (m_currentScreen)
    {
        case GameScreen::Title:
        {
            UpdateTitleScreen(deltaTime);
            break;
        }
        case GameScreen::Gameplay:
        {
            UpdateGameplayScreen(deltaTime);
            break;
        }
        case GameScreen::Options:
        {
            UpdateOptionsScreen(deltaTime);
            break;
        }
    }
}

void Game::Draw() const
{
    BeginDrawing();
    ClearBackground(BLACK);

    switch (m_currentScreen)
    {
        case GameScreen::Title:
        {
            DrawTitleScreen();
            break;
        }
        case GameScreen::Gameplay:
        {
            DrawGameplayScreen();
            break;
        }
        case GameScreen::Options:
        {
            DrawOptionsScreen();
            break;
        }
    }

    EndDrawing();
}

void Game::UpdateTitleScreen(float deltaTime)
{
    m_titlePulseTimer += deltaTime;

    const int totalOptions = 4;
    bool selectionTriggered = false;

    /* 1. Clavier et D-Pad */
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
    {
        m_selectedTitleOption = (m_selectedTitleOption + 1) % totalOptions;
    }
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
    {
        m_selectedTitleOption = (m_selectedTitleOption - 1 + totalOptions) % totalOptions;
    }

    if (IsGamepadAvailable(0))
    {
        if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN))
        {
            m_selectedTitleOption = (m_selectedTitleOption + 1) % totalOptions;
        }
        if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_UP))
        {
            m_selectedTitleOption = (m_selectedTitleOption - 1 + totalOptions) % totalOptions;
        }
    }

    /* 2. Souris (Hover et Sélection) */
    const Vector2 mousePos = GetMousePosition();
    for (int i = 0; i < totalOptions; ++i)
    {
        Rectangle optionRect = {
            (float)kScreenWidth / 2.0f - 150.0f,
            (float)kScreenHeight / 2.0f - 20.0f + (float)i * 50.0f,
            300.0f,
            40.0f
        };

        if (CheckCollisionPointRec(mousePos, optionRect))
        {
            m_selectedTitleOption = i;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                selectionTriggered = true;
            }
        }
    }

    /* 3. Déclencheur (Enter, Espace ou bouton Manette A/X) */
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))
    {
        selectionTriggered = true;
    }
    if (IsGamepadAvailable(0))
    {
        if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN) || 
            IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_LEFT))
        {
            selectionTriggered = true;
        }
    }

    /* 4. Actionner l'option sélectionnée */
    if (selectionTriggered)
    {
        switch (m_selectedTitleOption)
        {
            case 0: /* Lancer une nouvelle partie */
            {
                m_world->Reset();
                m_currentScreen = GameScreen::Gameplay;
                m_isInventoryOpen = false;
                break;
            }
            case 1: /* Continuer la partie */
            {
                m_currentScreen = GameScreen::Gameplay;
                m_isInventoryOpen = false;
                break;
            }
            case 2: /* Options */
            {
                m_currentScreen = GameScreen::Options;
                break;
            }
            case 3: /* Quitter */
            {
                m_shouldKeepRunning = false;
                break;
            }
        }
    }
}

void Game::DrawTitleScreen() const
{
    const int gridSize = 40;
    for (int x = 0; x < kScreenWidth; x += gridSize)
    {
        DrawLine(x, 0, x, kScreenHeight, { 20, 45, 20, 255 });
    }
    for (int y = 0; y < kScreenHeight; y += gridSize)
    {
        DrawLine(0, y, kScreenWidth, y, { 20, 45, 20, 255 });
    }

    const float pulse = std::sin(m_titlePulseTimer * 3.0f) * 4.0f;
    const char* titleText = "ZELDA 2D : HYRULE STARTER";
    const int titleFontSize = 40;
    const int titleWidth = MeasureText(titleText, titleFontSize);
    const int titleX = kScreenWidth / 2 - titleWidth / 2;
    const int titleY = 120 + (int)pulse;

    DrawText(titleText, titleX + 3, titleY + 3, titleFontSize, DARKGREEN);
    DrawText(titleText, titleX, titleY, titleFontSize, GREEN);

    DrawText("UN PROJET DE REMISE A NIVEAU C++", kScreenWidth / 2 - MeasureText("UN PROJET DE REMISE A NIVEAU C++", 14) / 2, titleY + 50, 14, GRAY);

    const char* options[] = {
        "Lancer une nouvelle partie",
        "Continuer la partie",
        "Acceder aux options",
        "Quitter le jeu"
    };

    for (int i = 0; i < 4; ++i)
    {
        Rectangle optionRect = {
            (float)kScreenWidth / 2.0f - 180.0f,
            (float)kScreenHeight / 2.0f - 20.0f + (float)i * 50.0f,
            360.0f,
            40.0f
        };

        const bool isSelected = (m_selectedTitleOption == i);
        Color boxColor = isSelected ? Color{ 40, 40, 40, 255 } : Color{ 15, 15, 15, 255 };
        Color borderColor = isSelected ? GREEN : DARKGRAY;
        Color textColor = isSelected ? RAYWHITE : LIGHTGRAY;

        DrawRectangleRec(optionRect, boxColor);
        DrawRectangleLinesEx(optionRect, 1.5f, borderColor);

        if (isSelected)
        {
            DrawTriangle(
                { optionRect.x + 10.0f, optionRect.y + 12.0f },
                { optionRect.x + 10.0f, optionRect.y + 28.0f },
                { optionRect.x + 22.0f, optionRect.y + 20.0f },
                GREEN
            );
        }

        const int fontSize = 16;
        const int textWidth = MeasureText(options[i], fontSize);
        DrawText(options[i], (int)optionRect.x + (int)optionRect.width / 2 - textWidth / 2, (int)optionRect.y + 12, fontSize, textColor);
    }

    DrawText("Utilisez les Fleches / Joystick pour naviguer, Entree / Bouton A pour valider", kScreenWidth / 2 - MeasureText("Utilisez les Fleches / Joystick pour naviguer, Entree / Bouton A pour valider", 12) / 2, kScreenHeight - 40, 12, DARKGRAY);
}

void Game::UpdateGameplayScreen(float deltaTime)
{
    /* Touche 'I' ou bouton SELECT de la manette pour basculer l'inventaire */
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
        m_isInventoryOpen = !m_isInventoryOpen;
    }

    if (m_isInventoryOpen)
    {
        /* On gère les inputs de la forge d'armes */
        UpdateInventoryMenu(m_world->GetPlayer());
    }
    else
    {
        /* Boucle de jeu standard */
        m_world->Update(deltaTime);

        if (IsKeyPressed(KEY_ESCAPE))
        {
            m_currentScreen = GameScreen::Title;
        }
        if (IsGamepadAvailable(0))
        {
            if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_RIGHT))
            {
                m_currentScreen = GameScreen::Title;
            }
        }
    }
}

void Game::DrawGameplayScreen() const
{
    m_world->Draw();

    if (m_isInventoryOpen)
    {
        /* Assombrissement progressif en fondu */
        DrawRectangle(0, 0, kScreenWidth, kScreenHeight, Fade(BLACK, 0.82f));
        DrawInventoryMenu(m_world->GetPlayer());
    }
    else
    {
        /* HUD interactif standard */
        DrawRectangle(10, kScreenHeight - 45, 450, 35, Fade(BLACK, 0.7f));
        DrawRectangleLines(10, kScreenHeight - 45, 450, 35, RED);
        DrawText("Pressez [ ECHAP ] pour quitter | Pressez [ I ] ou [ SELECT ] pour FORGER & AMELIORER", 18, kScreenHeight - 33, 10, RAYWHITE);
    }
}

void Game::UpdateOptionsScreen(float deltaTime)
{
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))
    {
        m_currentScreen = GameScreen::Title;
    }

    if (IsGamepadAvailable(0))
    {
        if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN) || 
            IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_RIGHT))
        {
            m_currentScreen = GameScreen::Title;
        }
    }

    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A))
    {
        m_soundVolumePercent = (m_soundVolumePercent > 0) ? (m_soundVolumePercent - 10) : 0;
    }
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D))
    {
        m_soundVolumePercent = (m_soundVolumePercent < 100) ? (m_soundVolumePercent + 10) : 100;
    }

    const Vector2 mousePos = GetMousePosition();
    Rectangle backRect = { (float)kScreenWidth / 2.0f - 100.0f, (float)kScreenHeight - 100.0f, 200.0f, 40.0f };
    if (CheckCollisionPointRec(mousePos, backRect))
    {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            m_currentScreen = GameScreen::Title;
        }
    }
}

void Game::DrawOptionsScreen() const
{
    const int titleFontSize = 32;
    const int titleWidth = MeasureText("OPTIONS DU JEU", titleFontSize);
    DrawText("OPTIONS DU JEU", kScreenWidth / 2 - titleWidth / 2, 80, titleFontSize, GREEN);

    DrawText("Volume des effets sonores :", 150, 180, 18, RAYWHITE);
    DrawRectangle(150, 215, 300, 20, DARKGRAY);
    DrawRectangle(150, 215, (int)(3.0f * (float)m_soundVolumePercent), 20, GREEN);
    DrawRectangleLines(150, 215, 300, 20, WHITE);

    const char* volumeText = TextFormat("%d %%", m_soundVolumePercent);
    DrawText(volumeText, 470, 215, 18, GREEN);

    DrawText("CONTROLES DU JEU :", 150, 280, 18, GREEN);
    
    DrawText("- Deplacements : Touches ZQSD / Fleches du clavier", 150, 315, 14, LIGHTGRAY);
    DrawText("- Deplacements manette : Joystick Analogique Gauche / D-Pad", 150, 340, 14, LIGHTGRAY);
    DrawText("- Attaque a l'epee : Touche ESPACE / Bouton face OUEST ou face SUD", 150, 365, 14, LIGHTGRAY);
    DrawText("- Quitter une partie en cours : Touche ECHAP / Bouton START", 150, 390, 14, LIGHTGRAY);

    Rectangle backRect = { (float)kScreenWidth / 2.0f - 100.0f, (float)kScreenHeight - 100.0f, 200.0f, 40.0f };
    const bool isHovered = CheckCollisionPointRec(GetMousePosition(), backRect);
    
    DrawRectangleRec(backRect, isHovered ? GRAY : Color{ 20, 20, 20, 255 });
    DrawRectangleLinesEx(backRect, 1.5f, isHovered ? WHITE : GREEN);
    DrawText("Retour au Menu", (int)backRect.x + 100 - MeasureText("Retour au Menu", 16) / 2, (int)backRect.y + 12, 16, isHovered ? BLACK : RAYWHITE);
}

void Game::UpdateInventoryMenu(Player& player)
{
    Inventory& inv = player.GetInventory();
    const Vector2 mousePos = GetMousePosition();

    if (inv.m_upgradePoints > 0)
    {
        /* Boutons Épée */
        if (inv.HasItem("sword"))
        {
            Item* sword = inv.GetItem("sword");
            /* 1. Améliorer Dégâts */
            Rectangle dmgBtn = { 510.0f, 175.0f, 110.0f, 24.0f };
            if (CheckCollisionPointRec(mousePos, dmgBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                sword->damage += 5.0f;
                sword->level += 1;
                inv.m_upgradePoints -= 1;
            }
            /* 2. Améliorer Portée */
            Rectangle rangeBtn = { 510.0f, 205.0f, 110.0f, 24.0f };
            if (CheckCollisionPointRec(mousePos, rangeBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                sword->range += 8.0f;
                sword->level += 1;
                inv.m_upgradePoints -= 1;
            }
            /* 3. Cycle Élément */
            Rectangle elemBtn = { 510.0f, 235.0f, 110.0f, 24.0f };
            if (CheckCollisionPointRec(mousePos, elemBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                int currentElem = (int)sword->element;
                sword->element = (ElementType)((currentElem + 1) % 4);
                sword->level += 1;
                inv.m_upgradePoints -= 1;
            }
        }

        /* Boutons Boomerang */
        if (inv.HasItem("boomerang"))
        {
            Item* boom = inv.GetItem("boomerang");
            /* 1. Améliorer Dégâts */
            Rectangle dmgBtn = { 510.0f, 355.0f, 110.0f, 24.0f };
            if (CheckCollisionPointRec(mousePos, dmgBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                boom->damage += 2.5f;
                boom->level += 1;
                inv.m_upgradePoints -= 1;
            }
            /* 2. Améliorer Vitesse */
            Rectangle speedBtn = { 510.0f, 385.0f, 110.0f, 24.0f };
            if (CheckCollisionPointRec(mousePos, speedBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                boom->speed += 50.0f;
                boom->level += 1;
                inv.m_upgradePoints -= 1;
            }
            /* 3. Améliorer Portée */
            Rectangle rangeBtn = { 510.0f, 415.0f, 110.0f, 24.0f };
            if (CheckCollisionPointRec(mousePos, rangeBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                boom->range += 30.0f;
                boom->level += 1;
                inv.m_upgradePoints -= 1;
            }
            /* 4. Cycle Élément */
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

void Game::DrawInventoryMenu(const Player& player) const
{
    const Inventory& inv = player.GetInventory();
    const Vector2 mousePos = GetMousePosition();

    /* Panneau central de forge */
    Rectangle panel = { 100, 50, 600, 500 };
    DrawRectangleRec(panel, { 15, 20, 25, 255 });
    DrawRectangleLinesEx(panel, 2.0f, GREEN);

    DrawText("FORGE & ARSENAL DE COMBAT", 400 - MeasureText("FORGE & ARSENAL DE COMBAT", 24) / 2, 75, 24, GOLD);
    DrawText("Cliquez sur les ameliorations pour modifier vos armes", 400 - MeasureText("Cliquez sur les ameliorations pour modifier vos armes", 12) / 2, 110, 12, GRAY);

    /* Affichage des points d'amélioration */
    const char* ptsText = TextFormat("Points de forge : %d", inv.m_upgradePoints);
    Color ptsColor = (inv.m_upgradePoints > 0) ? GREEN : RED;
    DrawText(ptsText, 400 - MeasureText(ptsText, 16) / 2, 132, 16, ptsColor);

    /* -----------------------------------------------------------------
     * ÉPÉE
     * ----------------------------------------------------------------- */
    Rectangle swordBox = { 130, 165, 540, 110 };
    DrawRectangleRec(swordBox, { 25, 30, 40, 255 });
    DrawRectangleLinesEx(swordBox, 1.0f, inv.HasItem("sword") ? GREEN : DARKGRAY);

    if (inv.HasItem("sword"))
    {
        const Item* sword = inv.GetItem("sword");
        DrawText(TextFormat("%s (Nv. %d)", sword->name.c_str(), sword->level), 150, 175, 15, GREEN);
        
        /* Stats Épée */
        DrawText(TextFormat("Degats : %0.1f", sword->damage), 150, 200, 13, LIGHTGRAY);
        DrawText(TextFormat("Portee : %0.1f px", sword->range), 150, 220, 13, LIGHTGRAY);
        
        const char* elemName = "Aucun";
        Color elemColor = WHITE;
        if (sword->element == ElementType::Fire) { elemName = "Feu"; elemColor = ORANGE; }
        else if (sword->element == ElementType::Ice) { elemName = "Glace"; elemColor = SKYBLUE; }
        else if (sword->element == ElementType::Lightning) { elemName = "Foudre"; elemColor = GOLD; }
        DrawText(TextFormat("Element : %s", elemName), 150, 240, 13, elemColor);

        /* Boutons Upgrades */
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

    /* -----------------------------------------------------------------
     * BOOMERANG
     * ----------------------------------------------------------------- */
    Rectangle boomBox = { 130, 345, 540, 140 };
    DrawRectangleRec(boomBox, { 25, 30, 40, 255 });
    DrawRectangleLinesEx(boomBox, 1.0f, inv.HasItem("boomerang") ? SKYBLUE : DARKGRAY);

    if (inv.HasItem("boomerang"))
    {
        const Item* boom = inv.GetItem("boomerang");
        DrawText(TextFormat("%s (Nv. %d)", boom->name.c_str(), boom->level), 150, 355, 15, SKYBLUE);
        
        /* Stats Boomerang */
        DrawText(TextFormat("Degats : %0.1f", boom->damage), 150, 380, 13, LIGHTGRAY);
        DrawText(TextFormat("Vitesse: %0.1f px/s", boom->speed), 150, 400, 13, LIGHTGRAY);
        DrawText(TextFormat("Portee : %0.1f px", boom->range), 150, 420, 13, LIGHTGRAY);

        const char* elemName = "Aucun";
        Color elemColor = WHITE;
        if (boom->element == ElementType::Fire) { elemName = "Feu"; elemColor = ORANGE; }
        else if (boom->element == ElementType::Ice) { elemName = "Glace"; elemColor = SKYBLUE; }
        else if (boom->element == ElementType::Lightning) { elemName = "Foudre"; elemColor = GOLD; }
        DrawText(TextFormat("Element : %s", elemName), 150, 440, 13, elemColor);

        /* Boutons Upgrades */
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

    DrawText("Pressez [ I ] ou [ SELECT ] pour fermer l'arsenal et retourner au combat", 400 - MeasureText("Pressez [ I ] ou [ SELECT ] pour fermer l'arsenal et retourner au combat", 11) / 2, 515, 11, GRAY);
}
