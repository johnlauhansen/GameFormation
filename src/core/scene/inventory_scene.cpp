#include "inventory_scene.h"
#include "scene_manager.h"
#include "player.h"

InventoryScene::InventoryScene(Player& player)
    : m_player(player)
{
}

void InventoryScene::Update(float deltaTime, SceneManager& manager)
{
    (void)deltaTime;

    Inventory& inv = m_player.GetInventory();

    /* Quitter l'inventaire avec ECHAP ou I */
    bool closeInventory = IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_I);
    if (IsGamepadAvailable(0))
    {
        if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_RIGHT) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_LEFT))
        {
            closeInventory = true;
        }
    }

    if (closeInventory)
    {
        manager.ChangeScene("Gameplay");
        return;
    }

    /* Logique d'amélioration de la Forge */
    bool upPressed = IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W) || (IsGamepadAvailable(0) && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_UP));
    bool downPressed = IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S) || (IsGamepadAvailable(0) && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN));

    if (inv.HasItem("sword"))
    {
        Item* sword = inv.GetItem("sword");
        if (sword != nullptr)
        {
            /* Check clic souris sur l'épée */
            Vector2 mousePos = GetMousePosition();
            bool clickedSwordLvl = CheckCollisionPointRec(mousePos, { 150, 160, 200, 30 }) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
            bool clickedSwordEle = CheckCollisionPointRec(mousePos, { 150, 200, 200, 30 }) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

            if (upPressed || clickedSwordLvl)
            {
                if (inv.m_upgradePoints > 0)
                {
                    sword->level += 1;
                    sword->damage += 2.5f;
                    sword->range += 5.0f;
                    inv.m_upgradePoints -= 1;
                }
            }

            if (downPressed || clickedSwordEle)
            {
                if (inv.m_upgradePoints > 0)
                {
                    int nextEle = (int)sword->element + 1;
                    if (nextEle > (int)ElementType::Lightning)
                    {
                        nextEle = (int)ElementType::None;
                    }
                    sword->element = (ElementType)nextEle;
                    inv.m_upgradePoints -= 1;
                }
            }
        }
    }

    if (inv.HasItem("boomerang"))
    {
        Item* boom = inv.GetItem("boomerang");
        if (boom != nullptr)
        {
            /* Check clic souris sur le boomerang */
            Vector2 mousePos = GetMousePosition();
            bool clickedBoomLvl = CheckCollisionPointRec(mousePos, { 450, 160, 200, 30 }) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
            bool clickedBoomEle = CheckCollisionPointRec(mousePos, { 450, 200, 200, 30 }) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

            if (upPressed || clickedBoomLvl)
            {
                if (inv.m_upgradePoints > 0)
                {
                    boom->level += 1;
                    boom->damage += 1.5f;
                    boom->speed += 50.0f;
                    inv.m_upgradePoints -= 1;
                }
            }

            if (downPressed || clickedBoomEle)
            {
                if (inv.m_upgradePoints > 0)
                {
                    int nextEle = (int)boom->element + 1;
                    if (nextEle > (int)ElementType::Lightning)
                    {
                        nextEle = (int)ElementType::None;
                    }
                    boom->element = (ElementType)nextEle;
                    inv.m_upgradePoints -= 1;
                }
            }
        }
    }
}

void InventoryScene::Draw() const
{
    /* On suppose que la scène de gameplay en arrière-plan a déjà été dessinée par le Game
       ou on efface simplement le fond pour l'inventaire plein écran. */
    ClearBackground({ 15, 15, 20, 240 });

    const Inventory& inv = m_player.GetInventory();

    DrawText("FORGE & INVENTAIRE", 800 / 2 - MeasureText("FORGE & INVENTAIRE", 24) / 2, 60, 24, GOLD);

    DrawText("Points de forge : ", 280, 120, 16, WHITE);
    DrawText(TextFormat("%d", inv.m_upgradePoints), 280 + MeasureText("Points de forge : ", 16), 120, 16, (inv.m_upgradePoints > 0) ? GREEN : RED);

    /* --- Colonne Épée --- */
    if (inv.HasItem("sword"))
    {
        const Item* sword = inv.GetItem("sword");
        if (sword != nullptr)
        {
            DrawText(TextFormat("EPEE (Niv %d)", sword->level), 150, 160, 20, LIGHTGRAY);
            DrawText(TextFormat("Degats : %.1f", sword->damage), 160, 190, 14, GRAY);
            DrawText(TextFormat("Allonge : %.0f", sword->range), 160, 210, 14, GRAY);

            std::string eleStr = "Physique";
            Color eleColor = WHITE;
            if (sword->element == ElementType::Fire) { eleStr = "Feu"; eleColor = ORANGE; }
            else if (sword->element == ElementType::Ice) { eleStr = "Glace"; eleColor = SKYBLUE; }
            else if (sword->element == ElementType::Lightning) { eleStr = "Foudre"; eleColor = YELLOW; }

            DrawText("Element : ", 160, 230, 14, GRAY);
            DrawText(eleStr.c_str(), 160 + MeasureText("Element : ", 14), 230, 14, eleColor);

            if (inv.m_upgradePoints > 0)
            {
                DrawRectangle(150, 260, 200, 30, DARKGRAY);
                DrawRectangleLines(150, 260, 200, 30, LIGHTGRAY);
                DrawText("Ameliorer Niveau (Clic)", 160, 268, 12, GREEN);

                DrawRectangle(150, 300, 200, 30, DARKGRAY);
                DrawRectangleLines(150, 300, 200, 30, LIGHTGRAY);
                DrawText("Changer Element (Clic)", 160, 308, 12, SKYBLUE);
            }
        }
    }
    else
    {
        DrawText("EPEE (Non possedee)", 150, 160, 20, DARKGRAY);
    }

    /* --- Colonne Boomerang --- */
    if (inv.HasItem("boomerang"))
    {
        const Item* boom = inv.GetItem("boomerang");
        if (boom != nullptr)
        {
            DrawText(TextFormat("BOOMERANG (Niv %d)", boom->level), 450, 160, 20, LIGHTGRAY);
            DrawText(TextFormat("Degats : %.1f", boom->damage), 460, 190, 14, GRAY);
            DrawText(TextFormat("Vitesse : %.0f", boom->speed), 460, 210, 14, GRAY);

            std::string eleStr = "Physique";
            Color eleColor = WHITE;
            if (boom->element == ElementType::Fire) { eleStr = "Feu"; eleColor = ORANGE; }
            else if (boom->element == ElementType::Ice) { eleStr = "Glace"; eleColor = SKYBLUE; }
            else if (boom->element == ElementType::Lightning) { eleStr = "Foudre"; eleColor = YELLOW; }

            DrawText("Element : ", 460, 230, 14, GRAY);
            DrawText(eleStr.c_str(), 460 + MeasureText("Element : ", 14), 230, 14, eleColor);

            if (inv.m_upgradePoints > 0)
            {
                DrawRectangle(450, 260, 200, 30, DARKGRAY);
                DrawRectangleLines(450, 260, 200, 30, LIGHTGRAY);
                DrawText("Ameliorer Niveau (Clic)", 460, 268, 12, GREEN);

                DrawRectangle(450, 300, 200, 30, DARKGRAY);
                DrawRectangleLines(450, 300, 200, 30, LIGHTGRAY);
                DrawText("Changer Element (Clic)", 460, 308, 12, SKYBLUE);
            }
        }
    }
    else
    {
        DrawText("BOOMERANG (Non possede)", 450, 160, 20, DARKGRAY);
    }

    DrawText("Appuyez sur I ou ECHAP pour fermer", 800 / 2 - MeasureText("Appuyez sur I ou ECHAP pour fermer", 14) / 2, 500, 14, GRAY);
}
