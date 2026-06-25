#pragma once
#include <raylib.h>
#include <memory>

enum class GameScreen
{
    Title,
    Gameplay,
    Options
};

class GameWorld;
class Player;

class Game
{
public:
    Game();
    ~Game();

    void Run();

private:
    void Update(float deltaTime);
    void Draw() const;

    void UpdateTitleScreen(float deltaTime);
    void DrawTitleScreen() const;

    void UpdateGameplayScreen(float deltaTime);
    void DrawGameplayScreen() const;

    void UpdateOptionsScreen(float deltaTime);
    void DrawOptionsScreen() const;

    /* Menu de Forge & d'Amélioration */
    void UpdateInventoryMenu(Player& player);
    void DrawInventoryMenu(const Player& player) const;

    static constexpr int kScreenWidth = 800;
    static constexpr int kScreenHeight = 600;

    GameScreen m_currentScreen;
    bool m_shouldKeepRunning;

    std::unique_ptr<GameWorld> m_world;

    int m_selectedTitleOption;
    float m_titlePulseTimer;

    /* Options variables */
    int m_soundVolumePercent;
    bool m_controlsShown;

    /* Contrôle d'état d'inventaire */
    bool m_isInventoryOpen;
};
