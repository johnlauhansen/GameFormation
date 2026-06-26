#include "enemy_behavior.h"
#include "enemy.h"
#include "map/tile_map.h"
#include <cmath>
#include <raymath.h>

/* Helper pour la logique de mouvement partagée commune (Chasing / Patrolling) */
static void UpdateCommonMovement(Enemy& context, float deltaTime, const TileMap& tileMap, const Vector2& playerPos, float speedMultiplier = 1.0f)
{
    float distToPlayer = Vector2Distance(context.GetPosition(), playerPos);
    float chaseRadius = context.GetChaseRadius();

    if (chaseRadius > 0.0f)
    {
        if (distToPlayer <= chaseRadius)
        {
            context.SetState(EnemyState::Chasing);
        }
        else if (context.GetState() == EnemyState::Chasing && distToPlayer > chaseRadius + 40.0f)
        {
            context.SetState(EnemyState::Patrolling);
            context.ResetPatrolTargetToCurrent();
        }
    }

    if (context.GetState() == EnemyState::Chasing)
    {
        Vector2 dir = Vector2Normalize(Vector2Subtract(playerPos, context.GetPosition()));
        float actualSpeed = context.GetSpeed() * speedMultiplier;
        Vector2 oldPos = context.GetPosition();
        Vector2 newPos = Vector2Add(oldPos, Vector2Scale(dir, actualSpeed * deltaTime));
        context.SetPosition(newPos);

        if (tileMap.CheckCollision(context.GetCollisionRect()))
        {
            context.SetPosition(oldPos);
            context.SetPosition({ oldPos.x + dir.x * actualSpeed * deltaTime, oldPos.y });
            if (tileMap.CheckCollision(context.GetCollisionRect())) context.SetPosition({ oldPos.x, context.GetPosition().y });
            
            context.SetPosition({ context.GetPosition().x, oldPos.y + dir.y * actualSpeed * deltaTime });
            if (tileMap.CheckCollision(context.GetCollisionRect())) context.SetPosition({ context.GetPosition().x, oldPos.y });
        }
    }
    else
    {
        context.UpdatePatrolMovement(deltaTime, tileMap);
    }
}

/* ------------------------------------------------------------------------- */
/* SLIME BEHAVIOR                                                            */
/* ------------------------------------------------------------------------- */
void SlimeBehavior::Update(Enemy& context, float deltaTime, const TileMap& tileMap, const Vector2& playerPos)
{
    UpdateCommonMovement(context, deltaTime, tileMap, playerPos, 1.0f);
}

void SlimeBehavior::Draw(const Enemy& context, Color bodyColor) const
{
    Vector2 pos = context.GetPosition();
    float pulse = std::sin(GetTime() * 8.0f) * 2.0f;
    float slimeWidth = context.GetWidth() + pulse;
    float slimeHeight = context.GetHeight() - pulse;

    if (bodyColor.r == 255 && bodyColor.g == 0) // Flashing red
    {
        DrawEllipse((int)pos.x, (int)pos.y + 4, slimeWidth / 2.0f, slimeHeight / 2.0f, RED);
    }
    else
    {
        DrawEllipse((int)pos.x, (int)pos.y + 4, slimeWidth / 2.0f, slimeHeight / 2.0f, LIME);
        DrawEllipseLines((int)pos.x, (int)pos.y + 4, slimeWidth / 2.0f, slimeHeight / 2.0f, DARKGREEN);
        DrawCircle((int)pos.x - 5, (int)pos.y + 1, 2.0f, WHITE);
        DrawCircle((int)pos.x + 5, (int)pos.y + 1, 2.0f, WHITE);
        DrawCircle((int)pos.x - 5, (int)pos.y + 1, 1.0f, BLACK);
        DrawCircle((int)pos.x + 5, (int)pos.y + 1, 1.0f, BLACK);
    }
}

/* ------------------------------------------------------------------------- */
/* OCTOROK BEHAVIOR                                                          */
/* ------------------------------------------------------------------------- */
void OctorokBehavior::Update(Enemy& context, float deltaTime, const TileMap& tileMap, const Vector2& playerPos)
{
    UpdateCommonMovement(context, deltaTime, tileMap, playerPos, 1.0f);

    /* Logique de tir */
    float cooldown = context.GetShootCooldownTimer();
    if (cooldown > 0.0f)
    {
        context.SetShootCooldownTimer(cooldown - deltaTime);
    }

    float distToPlayer = Vector2Distance(context.GetPosition(), playerPos);
    if (distToPlayer < 240.0f && context.GetShootCooldownTimer() <= 0.0f)
    {
        EnemyProjectile rock;
        rock.position = context.GetPosition();
        rock.direction = Vector2Normalize(Vector2Subtract(playerPos, context.GetPosition()));
        rock.speed = 180.0f;
        rock.radius = 6.0f;
        rock.active = true;

        context.AddProjectile(rock);
        context.SetShootCooldownTimer(context.GetShootInterval());
    }
}

void OctorokBehavior::Draw(const Enemy& context, Color bodyColor) const
{
    Vector2 pos = context.GetPosition();
    float width = context.GetWidth();

    if (bodyColor.r == 255 && bodyColor.g == 0)
    {
        DrawCircleV(pos, width / 2.0f, RED);
    }
    else
    {
        DrawCircleV(pos, width / 2.0f, RED);
        DrawCircleLines((int)pos.x, (int)pos.y, (int)(width / 2.0f), MAROON);
        DrawRectangle((int)pos.x - 4, (int)pos.y, 8, 12, ORANGE);
        DrawRectangleLines((int)pos.x - 4, (int)pos.y, 8, 12, BLACK);
        DrawCircle((int)pos.x - 6, (int)pos.y - 3, 3.0f, WHITE);
        DrawCircle((int)pos.x + 6, (int)pos.y - 3, 3.0f, WHITE);
        DrawCircle((int)pos.x - 6, (int)pos.y - 3, 1.5f, BLACK);
        DrawCircle((int)pos.x + 6, (int)pos.y - 3, 1.5f, BLACK);
    }
}

/* ------------------------------------------------------------------------- */
/* MOBLIN BEHAVIOR                                                           */
/* ------------------------------------------------------------------------- */
void MoblinBehavior::Update(Enemy& context, float deltaTime, const TileMap& tileMap, const Vector2& playerPos)
{
    /* Moblin accélère de 1.3x quand il pourchasse ! */
    UpdateCommonMovement(context, deltaTime, tileMap, playerPos, 1.3f);
}

void MoblinBehavior::Draw(const Enemy& context, Color bodyColor) const
{
    Vector2 pos = context.GetPosition();
    Color moblinColor = (bodyColor.r == 255 && bodyColor.g == 0) ? RED : ORANGE;
    
    DrawRectangle((int)pos.x - 14, (int)pos.y - 14, 28, 28, moblinColor);
    DrawRectangleLines((int)pos.x - 14, (int)pos.y - 14, 28, 28, BLACK);
    DrawTriangle({ pos.x - 12, pos.y - 14 }, { pos.x - 12, pos.y - 24 }, { pos.x - 4, pos.y - 14 }, WHITE);
    DrawTriangle({ pos.x + 4, pos.y - 14 }, { pos.x + 12, pos.y - 24 }, { pos.x + 12, pos.y - 14 }, WHITE);
    DrawLine(pos.x - 10, pos.y - 6, pos.x - 2, pos.y - 2, BLACK);
    DrawLine(pos.x + 10, pos.y - 6, pos.x + 2, pos.y - 2, BLACK);
    DrawCircle(pos.x - 6, pos.y - 1, 2.0f, RED);
    DrawCircle(pos.x + 6, pos.y - 1, 2.0f, RED);
}
