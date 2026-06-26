#include "enemy.h"
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
    /* Configuration initiale selon le type d'ennemi */
    switch (m_type)
    {
        case EnemyType::Slime:
            m_health = 20.0f;
            m_maxHealth = 20.0f;
            m_damage = 5.0f; /* Un quart de coeur */
            m_speed = 60.0f;
            m_chaseRadius = 120.0f;
            break;
        case EnemyType::Octorok:
            m_health = 15.0f;
            m_maxHealth = 15.0f;
            m_damage = 5.0f;
            m_speed = 40.0f;
            m_chaseRadius = 0.0f; /* N'essaie pas de courir après le joueur, préfère tirer */
            m_shootInterval = 2.5f;
            break;
        case EnemyType::Moblin:
            m_health = 40.0f;
            m_maxHealth = 40.0f;
            m_damage = 15.0f; /* Presque un coeur */
            m_speed = 75.0f;
            m_chaseRadius = 180.0f;
            m_width = 36.0f;
            m_height = 36.0f;
            break;
    }
}

void Enemy::Update(float deltaTime, const TileMap& tileMap, const Vector2 playerPos)
{
    if (m_state == EnemyState::Dead)
    {
        return;
    }

    /* Décrémenter les timers de dégâts */
    if (m_damageFlashTimer > 0.0f)
    {
        m_damageFlashTimer -= deltaTime;
    }
    if (m_hitCooldownTimer > 0.0f)
    {
        m_hitCooldownTimer -= deltaTime;
    }

    /* Mise à jour du mouvement et du comportement */
    UpdateMovement(deltaTime, tileMap, playerPos);

    /* Octorok : Tirer des projectiles */
    if (m_type == EnemyType::Octorok)
    {
        UpdateOctorokShooting(deltaTime, playerPos);
    }

    /* Mettre à jour les projectiles actifs (Octorok) */
    for (auto& proj : m_projectiles)
    {
        if (proj.active)
        {
            proj.position = Vector2Add(proj.position, Vector2Scale(proj.direction, proj.speed * deltaTime));
            
            /* Collision projectile contre les murs solides */
            Rectangle projRect = { proj.position.x - proj.radius, proj.position.y - proj.radius, proj.radius * 2.0f, proj.radius * 2.0f };
            if (tileMap.CheckCollision(projRect))
            {
                proj.active = false;
            }
        }
    }
}

void Enemy::UpdateMovement(float deltaTime, const TileMap& tileMap, const Vector2 playerPos)
{
    float distToPlayer = Vector2Distance(m_position, playerPos);

    /* Évaluation de la transition d'état Patrolling <=> Chasing */
    if (m_chaseRadius > 0.0f)
    {
        if (distToPlayer <= m_chaseRadius)
        {
            m_state = EnemyState::Chasing;
        }
        else if (m_state == EnemyState::Chasing && distToPlayer > m_chaseRadius + 40.0f)
        {
            m_state = EnemyState::Patrolling;
            m_patrolTarget = m_position; /* Recommencer la patrouille ici */
        }
    }

    if (m_state == EnemyState::Chasing)
    {
        /* Poursuivre le joueur de façon agressive */
        Vector2 dir = Vector2Normalize(Vector2Subtract(playerPos, m_position));
        float actualSpeed = m_speed;
        if (m_type == EnemyType::Moblin)
        {
            actualSpeed *= 1.3f; /* Moblin court plus vite en chasse ! */
        }

        Vector2 oldPos = m_position;
        m_position = Vector2Add(m_position, Vector2Scale(dir, actualSpeed * deltaTime));

        if (tileMap.CheckCollision(GetCollisionRect()))
        {
            /* Glissement simple sur les murs si collision */
            m_position = oldPos;
            
            /* Essai horizontal seul */
            m_position.x += dir.x * actualSpeed * deltaTime;
            if (tileMap.CheckCollision(GetCollisionRect()))
            {
                m_position.x = oldPos.x;
            }
            
            /* Essai vertical seul */
            m_position.y += dir.y * actualSpeed * deltaTime;
            if (tileMap.CheckCollision(GetCollisionRect()))
            {
                m_position.y = oldPos.y;
            }
        }
    }
    else /* EnemyState::Patrolling */
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
            if (m_waypoints.empty())
            {
                return;
            }

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
}

void Enemy::UpdateOctorokShooting(float deltaTime, const Vector2 playerPos)
{
    if (m_shootCooldownTimer > 0.0f)
    {
        m_shootCooldownTimer -= deltaTime;
    }

    float distToPlayer = Vector2Distance(m_position, playerPos);
    if (distToPlayer < 240.0f && m_shootCooldownTimer <= 0.0f)
    {
        /* Tirer un caillou vers le joueur ! */
        EnemyProjectile rock;
        rock.position = m_position;
        rock.direction = Vector2Normalize(Vector2Subtract(playerPos, m_position));
        rock.speed = 180.0f;
        rock.radius = 6.0f;
        rock.active = true;

        m_projectiles.push_back(rock);
        m_shootCooldownTimer = m_shootInterval;
    }
}

void Enemy::Draw() const
{
    if (m_state == EnemyState::Dead)
    {
        return;
    }

    /* Dessiner les projectiles actifs de l'Octorok */
    for (const auto& proj : m_projectiles)
    {
        if (proj.active)
        {
            DrawCircleV(proj.position, proj.radius, DARKBROWN);
            DrawCircleLines((int)proj.position.x, (int)proj.position.y, (int)proj.radius, BLACK);
        }
    }

    /* Effet de clignotement rouge lors de la réception de dégâts */
    Color bodyColor = WHITE;
    if (m_damageFlashTimer > 0.0f)
    {
        bodyColor = RED;
    }

    switch (m_type)
    {
        case EnemyType::Slime:
        {
            /* Dessin procédural du Slime (vert gluant compressible) */
            float pulse = std::sin(GetTime() * 8.0f) * 2.0f;
            float slimeWidth = m_width + pulse;
            float slimeHeight = m_height - pulse;

            if (bodyColor.r == 255 && bodyColor.g == 0) // Flashing red
            {
                DrawEllipse((int)m_position.x, (int)m_position.y + 4, slimeWidth / 2.0f, slimeHeight / 2.0f, RED);
            }
            else
            {
                DrawEllipse((int)m_position.x, (int)m_position.y + 4, slimeWidth / 2.0f, slimeHeight / 2.0f, LIME);
                DrawEllipseLines((int)m_position.x, (int)m_position.y + 4, slimeWidth / 2.0f, slimeHeight / 2.0f, DARKGREEN);
                
                /* Yeux drôles */
                DrawCircle((int)m_position.x - 5, (int)m_position.y + 1, 2.0f, WHITE);
                DrawCircle((int)m_position.x + 5, (int)m_position.y + 1, 2.0f, WHITE);
                DrawCircle((int)m_position.x - 5, (int)m_position.y + 1, 1.0f, BLACK);
                DrawCircle((int)m_position.x + 5, (int)m_position.y + 1, 1.0f, BLACK);
            }
            break;
        }
        case EnemyType::Octorok:
        {
            /* Dessin de l'Octorok (Créature rouge avec museau soufflant) */
            if (bodyColor.r == 255 && bodyColor.g == 0)
            {
                DrawCircleV(m_position, m_width / 2.0f, RED);
            }
            else
            {
                DrawCircleV(m_position, m_width / 2.0f, RED);
                DrawCircleLines((int)m_position.x, (int)m_position.y, (int)(m_width / 2.0f), MAROON);

                /* Museau en cylindre */
                DrawRectangle((int)m_position.x - 4, (int)m_position.y, 8, 12, ORANGE);
                DrawRectangleLines((int)m_position.x - 4, (int)m_position.y, 8, 12, BLACK);

                /* Yeux */
                DrawCircle((int)m_position.x - 6, (int)m_position.y - 3, 3.0f, WHITE);
                DrawCircle((int)m_position.x + 6, (int)m_position.y - 3, 3.0f, WHITE);
                DrawCircle((int)m_position.x - 6, (int)m_position.y - 3, 1.5f, BLACK);
                DrawCircle((int)m_position.x + 6, (int)m_position.y - 3, 1.5f, BLACK);
            }
            break;
        }
        case EnemyType::Moblin:
        {
            /* Dessin du Moblin (Bulldog orange-brun armé) */
            Color moblinColor = (bodyColor.r == 255 && bodyColor.g == 0) ? RED : ORANGE;
            
            /* Tête */
            DrawRectangle((int)m_position.x - 14, (int)m_position.y - 14, 28, 28, moblinColor);
            DrawRectangleLines((int)m_position.x - 14, (int)m_position.y - 14, 28, 28, BLACK);

            /* Cornes blanches */
            DrawTriangle({ m_position.x - 12, m_position.y - 14 }, { m_position.x - 12, m_position.y - 24 }, { m_position.x - 4, m_position.y - 14 }, WHITE);
            DrawTriangle({ m_position.x + 4, m_position.y - 14 }, { m_position.x + 12, m_position.y - 24 }, { m_position.x + 12, m_position.y - 14 }, WHITE);

            /* Yeux fâchés */
            DrawLine(m_position.x - 10, m_position.y - 6, m_position.x - 2, m_position.y - 2, BLACK);
            DrawLine(m_position.x + 10, m_position.y - 6, m_position.x + 2, m_position.y - 2, BLACK);
            DrawCircle(m_position.x - 6, m_position.y - 1, 2.0f, RED);
            DrawCircle(m_position.x + 6, m_position.y - 1, 2.0f, RED);
            break;
        }
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
