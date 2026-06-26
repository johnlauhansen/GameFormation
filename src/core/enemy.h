#pragma once
#include <raylib.h>
#include <vector>
#include <string>
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

class Enemy
{
public:
    Enemy(const std::string& name, EnemyType type, Vector2 startPos);

    void Update(float deltaTime, const class TileMap& tileMap, const Vector2 playerPos);
    void Draw() const;

    [[nodiscard]] Rectangle GetCollisionRect() const;
    [[nodiscard]] Vector2 GetPosition() const
    {
        return m_position;
    }

    [[nodiscard]] bool IsAlive() const
    {
        return m_state != EnemyState::Dead;
    }

    [[nodiscard]] float GetHealth() const
    {
        return m_health;
    }

    [[nodiscard]] float GetMaxHealth() const
    {
        return m_maxHealth;
    }

    [[nodiscard]] float GetDamage() const
    {
        return m_damage;
    }

    [[nodiscard]] EnemyType GetType() const
    {
        return m_type;
    }

    [[nodiscard]] const std::string& GetName() const
    {
        return m_name;
    }

    bool TakeDamage(float amount);

    /* Configuration des mouvements */
    void SetStatic();
    void SetPatrolZone(float radius, float speed);
    void SetDefinedPath(const std::vector<Vector2>& waypoints, float speed);

    /* Projectiles de l'ennemi (pour collision joueur) */
    [[nodiscard]] std::vector<EnemyProjectile>& GetProjectiles()
    {
        return m_projectiles;
    }

    [[nodiscard]] const std::vector<EnemyProjectile>& GetProjectiles() const
    {
        return m_projectiles;
    }

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

    /* Comportement de poursuite (Moblin / Slime) */
    float m_chaseRadius;

    /* Projectiles (Octorok) */
    float m_shootCooldownTimer;
    float m_shootInterval;
    std::vector<EnemyProjectile> m_projectiles;

    void UpdateMovement(float deltaTime, const class TileMap& tileMap, const Vector2 playerPos);
    void UpdateOctorokShooting(float deltaTime, const Vector2 playerPos);
};
