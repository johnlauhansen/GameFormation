#include "camera_controller.h"

CameraController::CameraController()
{
    m_camera = { 0 };
    m_camera.zoom = 1.2f;
    m_camera.rotation = 0.0f;
    m_camera.offset = { 800.0f / 2.0f, 600.0f / 2.0f };
}

void CameraController::Initialize(Vector2 startTarget, int screenWidth, int screenHeight)
{
    m_camera.target = startTarget;
    m_camera.offset = { (float)screenWidth / 2.0f, (float)screenHeight / 2.0f };
    m_camera.zoom = 1.2f;
    m_camera.rotation = 0.0f;
}

void CameraController::Update(float deltaTime, Vector2 targetPos)
{
    (void)deltaTime; /* L'interpolation actuelle n'utilise pas le deltaTime formellement, mais on l'injecte pour l'évolutivité */
    
    /* Smooth Scrolling (Lerp) avec facteur d'amortissement de 0.1f */
    m_camera.target.x += (targetPos.x - m_camera.target.x) * 0.1f;
    m_camera.target.y += (targetPos.y - m_camera.target.y) * 0.1f;
}

void CameraController::BeginMode() const
{
    BeginMode2D(m_camera);
}

void CameraController::EndMode() const
{
    EndMode2D();
}
