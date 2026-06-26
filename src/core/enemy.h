#pragma once
#include <raylib.h>
#include <vector>
#include <string>
#include <memory>
#include "npc.h" // For MovementType sharing

enum class EnemyType
{
    Slime,
    Octorok,
    Moblin
};

enum class EnemyState
{
    Patrolling,
    Chasing,
    Dead
};

struct EnemyProjectile
{
    Vector2 position;
    Vector2 direction;
    float speed;
    float radius;
    bool active = true;
};

class EnemyBehavior;

class Enemy
{
public:
    Enemy(const std::string& name, EnemyType type, Vector2 startPos);

    /* Constructeur de copie supprimé, déplacement autorisé (car unique_ptr) */
    Enemy(const Enemy&) = delete;
    Enemy& operator=(const Enemy&) = delete;
    Enemy(Enemy&&) noexcept = default;
    Enemy& operator=(Enemy&&) noexcept = default;

    void Update(float deltaTime, const class TileMap& tileMap, const Vector2 playerPos);
    void Draw() const;

    [[nodiscard]] Rectangle GetCollisionRect() const;
    [[nodiscard]] Vector2 GetPosition() const { return m_position; }
    void SetPosition(Vector2 pos) { m_position = pos; }

    [[nodiscard]] bool IsAlive() const { return m_state != EnemyState::Dead; }
    [[nodiscard]] float GetHealth() const { return m_health; }
    [[nodiscard]] float GetMaxHealth() const { return m_maxHealth; }
    [[nodiscard]] float GetDamage() const { return m_damage; }
    [[nodiscard]] EnemyType GetType() const { return m_type; }
    [[nodiscard]] const std::string& GetName() const { return m_name; }

    [[nodiscard]] EnemyState GetState() const { return m_state; }
    void SetState(EnemyState state) { m_state = state; }

    [[nodiscard]] float GetChaseRadius() const { return m_chaseRadius; }
    [[nodiscard]] float GetSpeed() const { return m_speed; }
    [[nodiscard]] float GetWidth() const { return m_width; }
    [[nodiscard]] float GetHeight() const { return m_height; }

    [[nodiscard]] float GetShootCooldownTimer() const { return m_shootCooldownTimer; }
    void SetShootCooldownTimer(float t) { m_shootCooldownTimer = t; }
    [[nodiscard]] float GetShootInterval() const { return m_shootInterval; }

    bool TakeDamage(float amount);

    /* Configuration des mouvements */
    void SetStatic();
    void SetPatrolZone(float radius, float speed);
    void SetDefinedPath(const std::vector<Vector2>& waypoints, float speed);
    void ResetPatrolTargetToCurrent() { m_patrolTarget = m_position; }
    void UpdatePatrolMovement(float deltaTime, const class TileMap& tileMap);

    /* Projectiles de l'ennemi (pour collision joueur) */
    [[nodiscard]] std::vector<EnemyProjectile>& GetProjectiles() { return m_projectiles; }
    [[nodiscard]] const std::vector<EnemyProjectile>& GetProjectiles() const { return m_projectiles; }
    void AddProjectile(const EnemyProjectile& proj) { m_projectiles.push_back(proj); }

private:
    std::string m_name;
    EnemyType m_type;
    EnemyState m_state;
    Vector2 m_position;
    Vector2 m_startPosition;
    float m_width;
    float m_height;

    float m_health;
    float m_maxHealth;
    float m_damage;

    float m_damageFlashTimer;
    float m_hitCooldownTimer;

    /* Comportement de mouvement */
    MovementType m_movementType;
    float m_speed;

    /* Zone de patrouille */
    float m_patrolRadius;
    Vector2 m_patrolTarget;
    float m_patrolWaitTimer;

    /* Chemin prédéfini */
    std::vector<Vector2> m_waypoints;
    size_t m_currentWaypointIndex;
    bool m_pathForward;

    /* Paramètres IA */
    float m_chaseRadius;
    float m_shootCooldownTimer;
    float m_shootInterval;
    std::vector<EnemyProjectile> m_projectiles;

    /* Pattern Stratégie (Behavior) */
    std::unique_ptr<EnemyBehavior> m_behavior;
};
