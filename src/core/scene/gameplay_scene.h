#pragma once
#include "scene.h"
#include <memory>

class GameWorld;

class GameplayScene : public Scene
{
public:
    GameplayScene(std::shared_ptr<GameWorld> world);

    void Update(float deltaTime, SceneManager& manager) override;
    void Draw() const override;

private:
    std::shared_ptr<GameWorld> m_world;
};
