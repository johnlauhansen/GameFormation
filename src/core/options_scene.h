#pragma once
#include "scene.h"

class OptionsScene : public Scene
{
public:
    OptionsScene() : m_soundVolumePercent(100), m_controlsShown(false) {}

    void Update(float deltaTime, SceneManager& manager) override;
    void Draw() const override;

private:
    int m_soundVolumePercent;
    bool m_controlsShown;
};
