#pragma once
#include "scene.h"
#include <memory>

class Player;

class InventoryScene : public Scene
{
public:
    InventoryScene(Player& player);

    void Update(float deltaTime, SceneManager& manager) override;
    void Draw() const override;

private:
    Player& m_player;
};
