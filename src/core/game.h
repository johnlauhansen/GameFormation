#pragma once
#include <raylib.h>
#include <memory>
#include "scene_manager.h"

class GameWorld;

class Game
{
public:
    Game();
    ~Game();

    void Run();

private:
    static constexpr int kScreenWidth = 800;
    static constexpr int kScreenHeight = 600;

    bool m_shouldKeepRunning;
    RenderTexture2D m_target;
    
    SceneManager m_sceneManager;
    std::shared_ptr<GameWorld> m_world; /* Partagé avec la GameplayScene et la InventoryScene si besoin */
};
