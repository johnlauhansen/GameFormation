#pragma once
#include <raylib.h>

class SceneManager; // Forward declaration

class Scene
{
public:
    virtual ~Scene() = default;

    /* Cycle de vie standard de la scène */
    virtual void OnEnter() {}
    virtual void OnExit() {}

    /* Logique et Rendu (reçoivent le SceneManager pour déclencher des transitions) */
    virtual void Update(float deltaTime, SceneManager& manager) = 0;
    virtual void Draw() const = 0;
};
