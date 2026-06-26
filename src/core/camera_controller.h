#pragma once
#include <raylib.h>

class CameraController
{
public:
    CameraController();

    void Initialize(Vector2 startTarget, int screenWidth = 800, int screenHeight = 600);
    
    /* Met à jour le mouvement lissé de la caméra vers sa cible */
    void Update(float deltaTime, Vector2 targetPos);

    /* Démarre le rendu dans l'espace Monde (affecté par la caméra) */
    void BeginMode() const;
    
    /* Termine le rendu de l'espace Monde pour revenir à l'espace Écran (HUD) */
    void EndMode() const;

private:
    Camera2D m_camera;
};
