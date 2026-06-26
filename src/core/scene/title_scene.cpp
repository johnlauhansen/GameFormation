#include "title_scene.h"
#include "scene_manager.h"
#include <cmath>

void TitleScene::Update(float deltaTime, SceneManager& manager)
{
    m_titlePulseTimer += deltaTime * 5.0f;

    bool selectionTriggered = false;

    /* 1. Navigation au clavier (Haut / Bas) */
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
    {
        m_selectedTitleOption = (m_selectedTitleOption - 1 + 2) % 2;
    }
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
    {
        m_selectedTitleOption = (m_selectedTitleOption + 1) % 2;
    }

    /* 2. Navigation à la souris */
    const Vector2 rawMousePos = GetMousePosition();
    const Vector2 mousePos = {
        rawMousePos.x * (800.0f / GetScreenWidth()),
        rawMousePos.y * (600.0f / GetScreenHeight())
    };

    Rectangle playButton = { 800 / 2 - 100, 300, 200, 50 };
    Rectangle optionsButton = { 800 / 2 - 100, 380, 200, 50 };

    if (CheckCollisionPointRec(mousePos, playButton))
    {
        m_selectedTitleOption = 0;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            selectionTriggered = true;
        }
    }
    else if (CheckCollisionPointRec(mousePos, optionsButton))
    {
        m_selectedTitleOption = 1;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            selectionTriggered = true;
        }
    }

    /* 3. Déclencheur (Enter, Espace ou clic) */
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || selectionTriggered)
    {
        if (m_selectedTitleOption == 0)
        {
            manager.ChangeScene("Gameplay");
        }
        else if (m_selectedTitleOption == 1)
        {
            manager.ChangeScene("Options");
        }
    }
}

void TitleScene::Draw() const
{
    ClearBackground(DARKBLUE);

    const char* titleText = "gameFormation : Zelda-Like C++";
    const int titleWidth = MeasureText(titleText, 30);
    float titleYOffset = std::sin(m_titlePulseTimer) * 10.0f;
    DrawText(titleText, 800 / 2 - titleWidth / 2, 120 + (int)titleYOffset, 30, GOLD);

    Rectangle playButton = { 800 / 2 - 100, 300, 200, 50 };
    Rectangle optionsButton = { 800 / 2 - 100, 380, 200, 50 };

    /* Option Jouer */
    if (m_selectedTitleOption == 0)
    {
        DrawRectangleRec(playButton, LIGHTGRAY);
        DrawRectangleLinesEx(playButton, 2.0f, WHITE);
        DrawText("JOUER", (int)playButton.x + 60, (int)playButton.y + 15, 20, DARKBLUE);
    }
    else
    {
        DrawRectangleRec(playButton, GRAY);
        DrawText("JOUER", (int)playButton.x + 60, (int)playButton.y + 15, 20, WHITE);
    }

    /* Option Options */
    if (m_selectedTitleOption == 1)
    {
        DrawRectangleRec(optionsButton, LIGHTGRAY);
        DrawRectangleLinesEx(optionsButton, 2.0f, WHITE);
        DrawText("OPTIONS", (int)optionsButton.x + 50, (int)optionsButton.y + 15, 20, DARKBLUE);
    }
    else
    {
        DrawRectangleRec(optionsButton, GRAY);
        DrawText("OPTIONS", (int)optionsButton.x + 50, (int)optionsButton.y + 15, 20, WHITE);
    }

    DrawText("Utilisez les fleches ou la souris pour naviguer, Entree/Clic pour valider.", 800 / 2 - MeasureText("Utilisez les fleches ou la souris pour naviguer, Entree/Clic pour valider.", 12) / 2, 550, 12, LIGHTGRAY);
}
