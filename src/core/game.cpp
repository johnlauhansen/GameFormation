#include "game.h"
#include "game_world.h"
#include "player.h"
#include "title_scene.h"
#include "gameplay_scene.h"
#include "options_scene.h"
#include "inventory_scene.h"
#include <cmath>
#include <algorithm>

Game::Game()
    : m_shouldKeepRunning(true)
{
    /* Création de la RenderTexture pour le rendu off-screen */
    m_target = LoadRenderTexture(kScreenWidth, kScreenHeight);
    SetTextureFilter(m_target.texture, TEXTURE_FILTER_BILINEAR);

    /* Initialisation du GameWorld (Partagé) */
    m_world = std::make_shared<GameWorld>();

    /* Enregistrement des scènes dans le SceneManager */
    m_sceneManager.RegisterScene("Title", std::make_unique<TitleScene>());
    m_sceneManager.RegisterScene("Gameplay", std::make_unique<GameplayScene>(m_world));
    m_sceneManager.RegisterScene("Options", std::make_unique<OptionsScene>());
    m_sceneManager.RegisterScene("Inventory", std::make_unique<InventoryScene>(m_world->GetPlayer()));

    /* Lancement de la scène de titre par défaut */
    m_sceneManager.ChangeScene("Title");
}

Game::~Game()
{
    UnloadRenderTexture(m_target);
}

void Game::Run()
{
    while (!WindowShouldClose() && m_shouldKeepRunning)
    {
        const float deltaTime = GetFrameTime();

        /* Mise à jour logique de la scène courante */
        m_sceneManager.Update(deltaTime);

        /* Phase de Rendu */
        BeginTextureMode(m_target);
        m_sceneManager.Draw();
        EndTextureMode();

        /* Rendu final avec mise à l'échelle (Letterboxing / Pillarboxing) */
        BeginDrawing();
        ClearBackground(BLACK);

        const float scale = std::min((float)GetScreenWidth() / kScreenWidth, (float)GetScreenHeight() / kScreenHeight);
        
        Rectangle sourceRec = { 0.0f, 0.0f, (float)m_target.texture.width, (float)-m_target.texture.height };
        Rectangle destRec = {
            (GetScreenWidth() - ((float)kScreenWidth * scale)) * 0.5f,
            (GetScreenHeight() - ((float)kScreenHeight * scale)) * 0.5f,
            (float)kScreenWidth * scale,
            (float)kScreenHeight * scale
        };

        DrawTexturePro(m_target.texture, sourceRec, destRec, { 0.0f, 0.0f }, 0.0f, WHITE);
        
        DrawFPS(10, 10);
        EndDrawing();
    }
}
