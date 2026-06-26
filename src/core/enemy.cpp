#include "enemy.h"
#include "enemy_behavior.h"
#include "map/tile_map.h"
#include <cmath>
#include <raymath.h>

Enemy::Enemy(const std::string& name, EnemyType type, Vector2 startPos)
    : m_name(name)
    , m_type(type)
    , m_state(EnemyState::Patrolling)
    , m_position(startPos)
    , m_startPosition(startPos)
    , m_width(32.0f)
    , m_height(32.0f)
    , m_damageFlashTimer(0.0f)
    , m_hitCooldownTimer(0.0f)
    , m_movementType(MovementType::Static)
    , m_speed(50.0f)
    , m_patrolRadius(0.0f)
    , m_patrolTarget(startPos)
    , m_patrolWaitTimer(0.0f)
    , m_currentWaypointIndex(0)
    , m_pathForward(true)
    , m_chaseRadius(150.0f)
    , m_shootCooldownTimer(0.0f)
    , m_shootInterval(2.0f)
{
    /* Configuration initiale selon le type d'ennemi et instanciation du comportement */
    switch (m_type)
    {
        case EnemyType::Slime:
            m_health = 20.0f;
            m_maxHealth = 20.0f;
            m_damage = 5.0f; /* Un quart de coeur */
            m_speed = 60.0f;
            m_chaseRadius = 120.0f;
            m_behavior = std::make_unique<SlimeBehavior>();
            break;
        case EnemyType::Octorok:
            m_health = 15.0f;
            m_maxHealth = 15.0f;
            m_damage = 5.0f;
            m_speed = 40.0f;
            m_chaseRadius = 0.0f; /* N'essaie pas de courir après le joueur, préfère tirer */
            m_shootInterval = 2.5f;
            m_behavior = std::make_unique<OctorokBehavior>();
            break;
        case EnemyType::Moblin:
            m_health = 40.0f;
            m_maxHealth = 40.0f;
            m_damage = 15.0f; /* Presque un coeur */
            m_speed = 75.0f;
            m_chaseRadius = 180.0f;
            m_width = 36.0f;
            m_height = 36.0f;
            m_behavior = std::make_unique<MoblinBehavior>();
            break;
    }
}

Enemy::~Enemy() = default;

Enemy::Enemy(Enemy&&) noexcept = default;
Enemy& Enemy::operator=(Enemy&&) noexcept = default;

void Enemy::Update(float deltaTime, const TileMap& tileMap, const Vector2 playerPos)
{
    if (m_state == EnemyState::Dead)
    {
        return;
    }

    if (m_damageFlashTimer > 0.0f) m_damageFlashTimer -= deltaTime;
    if (m_hitCooldownTimer > 0.0f) m_hitCooldownTimer -= deltaTime;

    /* Délégation du comportement d'IA (Strategy Pattern) */
    if (m_behavior)
    {
        m_behavior->Update(*this, deltaTime, tileMap, playerPos);
    }

    /* Mettre à jour les projectiles actifs (générés potentiellement par l'OctorokBehavior) */
    for (auto& proj : m_projectiles)
    {
        if (proj.active)
        {
            proj.position = Vector2Add(proj.position, Vector2Scale(proj.direction, proj.speed * deltaTime));
            
            Rectangle projRect = { proj.position.x - proj.radius, proj.position.y - proj.radius, proj.radius * 2.0f, proj.radius * 2.0f };
            if (tileMap.CheckCollision(projRect))
            {
                proj.active = false;
            }
        }
    }
}

void Enemy::UpdatePatrolMovement(float deltaTime, const TileMap& tileMap)
{
    if (m_movementType == MovementType::Static)
    {
        return;
    }

    if (m_movementType == MovementType::PatrolZone)
    {
        if (m_patrolWaitTimer > 0.0f)
        {
            m_patrolWaitTimer -= deltaTime;
            return;
        }

        float dist = Vector2Distance(m_position, m_patrolTarget);
        if (dist < 5.0f)
        {
            m_patrolWaitTimer = (float)GetRandomValue(15, 40) / 10.0f; /* 1.5s à 4.0s */
            float angle = (float)GetRandomValue(0, 360) * DEG2RAD;
            float r = (float)GetRandomValue(0, (int)m_patrolRadius);
            m_patrolTarget.x = m_startPosition.x + std::cos(angle) * r;
            m_patrolTarget.y = m_startPosition.y + std::sin(angle) * r;
        }
        else
        {
            Vector2 dir = Vector2Normalize(Vector2Subtract(m_patrolTarget, m_position));
            Vector2 oldPos = m_position;
            m_position = Vector2Add(m_position, Vector2Scale(dir, m_speed * deltaTime));

            if (tileMap.CheckCollision(GetCollisionRect()))
            {
                m_position = oldPos;
                m_patrolWaitTimer = 0.5f;
                m_patrolTarget = m_startPosition;
            }
        }
    }
    else if (m_movementType == MovementType::DefinedPath)
    {
        if (m_waypoints.empty()) return;

        Vector2 target = m_waypoints[m_currentWaypointIndex];
        float dist = Vector2Distance(m_position, target);

        if (dist < 5.0f)
        {
            if (m_waypoints.size() > 1)
            {
                if (m_pathForward)
                {
                    m_currentWaypointIndex++;
                    if (m_currentWaypointIndex >= m_waypoints.size())
                    {
                        m_currentWaypointIndex = m_waypoints.size() - 2;
                        m_pathForward = false;
                    }
                }
                else
                {
                    if (m_currentWaypointIndex == 0)
                    {
                        m_currentWaypointIndex = 1;
                        m_pathForward = true;
                    }
                    else
                    {
                        m_currentWaypointIndex--;
                    }
                }
            }
        }
        else
        {
            Vector2 dir = Vector2Normalize(Vector2Subtract(target, m_position));
            Vector2 oldPos = m_position;
            m_position = Vector2Add(m_position, Vector2Scale(dir, m_speed * deltaTime));

            if (tileMap.CheckCollision(GetCollisionRect()))
            {
                m_position = oldPos;
            }
        }
    }
}

void Enemy::Draw() const
{
    if (m_state == EnemyState::Dead)
    {
        return;
    }

    /* Dessiner les projectiles actifs */
    for (const auto& proj : m_projectiles)
    {
        if (proj.active)
        {
            DrawCircleV(proj.position, proj.radius, DARKBROWN);
            DrawCircleLines((int)proj.position.x, (int)proj.position.y, (int)proj.radius, BLACK);
        }
    }

    Color bodyColor = WHITE;
    if (m_damageFlashTimer > 0.0f)
    {
        bodyColor = RED;
    }

    /* Délégation du dessin procédural */
    if (m_behavior)
    {
        m_behavior->Draw(*this, bodyColor);
    }

    /* Petite barre de vie au-dessus de la tête si blessé */
    if (m_health < m_maxHealth)
    {
        float hpPercent = m_health / m_maxHealth;
        DrawRectangle((int)m_position.x - 16, (int)m_position.y - m_height / 2.0f - 10, 32, 4, RED);
        DrawRectangle((int)m_position.x - 16, (int)m_position.y - m_height / 2.0f - 10, (int)(32 * hpPercent), 4, GREEN);
        DrawRectangleLines((int)m_position.x - 16, (int)m_position.y - m_height / 2.0f - 10, 32, 4, BLACK);
    }
}

Rectangle Enemy::GetCollisionRect() const
{
    return { m_position.x - m_width / 2.0f, m_position.y - m_height / 2.0f, m_width, m_height };
}

bool Enemy::TakeDamage(float amount)
{
    if (m_state == EnemyState::Dead || m_hitCooldownTimer > 0.0f)
    {
        return false;
    }

    m_health -= amount;
    m_damageFlashTimer = 0.2f;
    m_hitCooldownTimer = 0.4f;

    if (m_health <= 0.0f)
    {
        m_health = 0.0f;
        m_state = EnemyState::Dead;
    }

    return true;
}

void Enemy::SetStatic()
{
    m_movementType = MovementType::Static;
}

void Enemy::SetPatrolZone(float radius, float speed)
{
    m_movementType = MovementType::PatrolZone;
    m_patrolRadius = radius;
    m_speed = speed;
    m_patrolTarget = m_position;
}

void Enemy::SetDefinedPath(const std::vector<Vector2>& waypoints, float speed)
{
    m_movementType = MovementType::DefinedPath;
    m_waypoints = waypoints;
    m_speed = speed;
    m_currentWaypointIndex = 0;
    m_pathForward = true;
}
