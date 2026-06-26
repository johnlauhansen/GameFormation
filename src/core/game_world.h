#pragma once
#include "player.h"
#include "map/tile_map.h"
#include "hud.h"
#include "dialogue_system.h"
#include "entity_manager.h"
#include "camera_controller.h"
#include <raylib.h>
#include <string>

struct BoomerangProjectile
{
    Vector2 position = { 0.0f, 0.0f };
    Vector2 velocity = { 0.0f, 0.0f };
    bool active = false;
    bool returning = false;
    float speed = 350.0f;
    float rotation = 0.0f;
    float maxRange = 150.0f;
    Vector2 originPos = { 0.0f, 0.0f };
};

class GameWorld
{
public:
    GameWorld();

    void Update(float deltaTime);
    void Draw() const;
    void Reset();

    /*
     * Charge une carte dynamique et réinitialise tous les objets, destructibles et joueurs.
     * @param[in] filePath Chemin du fichier de carte JSON.
     * @return true si le chargement a réussi, false sinon.
     */
    bool LoadMap(const std::string& filePath);

    [[nodiscard]] Player& GetPlayer()
    {
        return m_player;
    }

    [[nodiscard]] const Player& GetPlayer() const
    {
        return m_player;
    }

private:
    TileMap m_tileMap;
    Player m_player;
    CameraController m_cameraController;

    EntityManager m_entityManager;
    float m_playerHitCooldown;
    DialogueSystem m_dialogueSystem;
    BoomerangProjectile m_boomerang;
    
    HUD m_hud;
};
