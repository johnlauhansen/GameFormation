#pragma once
#include "scene.h"
#include <string>

class TitleScene : public Scene
{
public:
    TitleScene() : m_selectedTitleOption(0), m_titlePulseTimer(0.0f) {}

    void Update(float deltaTime, SceneManager& manager) override;
    void Draw() const override;

private:
    int m_selectedTitleOption;
    float m_titlePulseTimer;
};
