#include "gameplay_scene.h"
#include "scene_manager.h"
#include "game_world.h"

GameplayScene::GameplayScene(std::shared_ptr<GameWorld> world)
    : m_world(std::move(world))
{
}

void GameplayScene::Update(float deltaTime, SceneManager& manager)
{
    /* Transition vers le menu (Forge / Inventaire) */
    bool openInventory = IsKeyPressed(KEY_I);
    if (IsGamepadAvailable(0))
    {
        if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_LEFT))
        {
            openInventory = true;
        }
    }

    if (openInventory)
    {
        manager.ChangeScene("Inventory");
        return;
    }

    /* Retour au menu titre (Optionnel, ex via Start) */
    if (IsGamepadAvailable(0) && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_RIGHT))
    {
        manager.ChangeScene("Title");
        return;
    }

    if (m_world)
    {
        m_world->Update(deltaTime);
    }
}

void GameplayScene::Draw() const
{
    ClearBackground(DARKBLUE);

    if (m_world)
    {
        m_world->Draw();
    }
}
