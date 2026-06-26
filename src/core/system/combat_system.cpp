#include "combat_system.h"
#include "player.h"
#include "enemy.h"
#include "destructible.h"
#include "npc.h"
#include "hud.h"
#include "entity_manager.h"
#include "event_system.h"
#include "game_world.h"
#include "map/tile_map.h"
#include <cmath>

namespace CombatSystem
{
    void ResolvePlayerSwordAttacks(
        Player& player,
        EntityManager& entityManager
    )
    {
        if (player.GetState() != PlayerState::Attacking)
        {
            return;
        }

        const Rectangle attackRect = player.GetAttackRect();
        const Item* sword = player.GetInventory().GetItem("sword");
        if (sword == nullptr || !sword->collected)
        {
            return;
        }

        /* 1. Attaques sur les objets destructibles via Spatial Grid (Broadphase O(1)) */
        auto nearbyDestructibles = entityManager.GetSpatialGrid().GetNearby(attackRect);
        for (auto dest : nearbyDestructibles)
        {
            if (dest->IsAlive() && CheckCollisionRecs(attackRect, dest->GetCollisionRect()))
            {
                bool wasAlive = dest->IsAlive();
                if (dest->TakeDamage(sword->damage, sword->damageType, sword->element))
                {
                    if (wasAlive && !dest->IsAlive())
                    {
                        /* Spawner un rubis vert */
                        GroundPickup rupee;
                        rupee.itemId = "rupee";
                        rupee.name = "RUBIS VERT";
                        rupee.position = dest->GetPosition();
                        rupee.active = true;
                        entityManager.AddPickup(std::move(rupee));

                        /* Diffusion d'un événement global pour le système de quêtes */
                        if (dest->GetType() == DestructibleType::Crate)
                        {
                            EventSystem::PublishCrateDestroyed({ dest->GetPosition() });
                        }
                    }
                }
            }
        }

        /* 2. Attaques sur les ennemis */
        for (auto& enemy : entityManager.GetEnemies())
        {
            if (enemy.IsAlive() && CheckCollisionRecs(attackRect, enemy.GetCollisionRect()))
            {
                if (enemy.TakeDamage(sword->damage))
                {
                    if (!enemy.IsAlive())
                    {
                        GroundPickup rupee;
                        rupee.itemId = "rupee";
                        rupee.name = "RUBIS VERT";
                        rupee.position = enemy.GetPosition();
                        rupee.active = true;
                        entityManager.AddPickup(std::move(rupee));
                    }
                }
            }
        }
    }

    void ResolvePlayerBoomerangAttacks(
        Player& player,
        BoomerangProjectile& boomerang,
        EntityManager& entityManager,
        const TileMap& tileMap,
        float deltaTime
    )
    {
        if (!boomerang.active)
        {
            return;
        }

        boomerang.rotation += 720.0f * deltaTime;

        if (!boomerang.returning)
        {
            boomerang.position.x += boomerang.velocity.x * deltaTime;
            boomerang.position.y += boomerang.velocity.y * deltaTime;

            const float dx = boomerang.position.x - boomerang.originPos.x;
            const float dy = boomerang.position.y - boomerang.originPos.y;
            const float distance = std::sqrt((dx * dx) + (dy * dy));

            const Rectangle boomRect = { boomerang.position.x - 8.0f, boomerang.position.y - 8.0f, 16.0f, 16.0f };
            const Item* boomStats = player.GetInventory().GetItem("boomerang");

            if (boomStats != nullptr)
            {
                /* 1. Collision du boomerang contre les objets destructibles (Broadphase O(1)) */
                auto nearbyDestructibles = entityManager.GetSpatialGrid().GetNearby(boomRect);
                for (auto dest : nearbyDestructibles)
                {
                    if (dest->IsAlive() && CheckCollisionRecs(boomRect, dest->GetCollisionRect()))
                    {
                        bool wasAlive = dest->IsAlive();
                        if (dest->TakeDamage(boomStats->damage, boomStats->damageType, boomStats->element))
                        {
                            if (wasAlive && !dest->IsAlive())
                            {
                                GroundPickup rupee;
                                rupee.itemId = "rupee";
                                rupee.name = "RUBIS VERT";
                                rupee.position = dest->GetPosition();
                                rupee.active = true;
                                entityManager.AddPickup(std::move(rupee));

                                if (dest->GetType() == DestructibleType::Crate)
                                {
                                    EventSystem::PublishCrateDestroyed({ dest->GetPosition() });
                                }
                            }
                            boomerang.returning = true;
                            break;
                        }
                    }
                }

                /* 2. Collision du boomerang contre les ennemis */
                if (!boomerang.returning)
                {
                    for (auto& enemy : entityManager.GetEnemies())
                    {
                        if (enemy.IsAlive() && CheckCollisionRecs(boomRect, enemy.GetCollisionRect()))
                        {
                            if (enemy.TakeDamage(boomStats->damage))
                            {
                                if (!enemy.IsAlive())
                                {
                                    GroundPickup rupee;
                                    rupee.itemId = "rupee";
                                    rupee.name = "RUBIS VERT";
                                    rupee.position = enemy.GetPosition();
                                    rupee.active = true;
                                    entityManager.AddPickup(std::move(rupee));
                                }
                                boomerang.returning = true;
                                break;
                            }
                        }
                    }
                }
            }

            /* Collision contre les murs */
            if (distance >= boomerang.maxRange || tileMap.CheckCollision(boomRect))
            {
                boomerang.returning = true;
            }
        }
        else
        {
            /* Retour vers le joueur */
            const Vector2 playerPos = player.GetPosition();
            const Vector2 dirToPlayer = { playerPos.x - boomerang.position.x, playerPos.y - boomerang.position.y };
            const float length = std::sqrt((dirToPlayer.x * dirToPlayer.x) + (dirToPlayer.y * dirToPlayer.y));

            if (length <= 20.0f)
            {
                boomerang.active = false;
            }
            else
            {
                boomerang.position.x += (dirToPlayer.x / length) * boomerang.speed * deltaTime;
                boomerang.position.y += (dirToPlayer.y / length) * boomerang.speed * deltaTime;
            }
        }
    }

    void ResolveEnemyDamageToPlayer(
        Player& player,
        EntityManager& entityManager,
        float& playerHitCooldown,
        HUD& hud,
        float deltaTime
    )
    {
        if (playerHitCooldown > 0.0f)
        {
            playerHitCooldown -= deltaTime;
        }

        if (playerHitCooldown <= 0.0f)
        {
            bool playerDamaged = false;
            float damageAmount = 0.0f;

            for (const auto& enemy : entityManager.GetEnemies())
            {
                if (enemy.IsAlive())
                {
                    /* Dégâts de contact */
                    if (CheckCollisionRecs(player.GetCollisionRect(), enemy.GetCollisionRect()))
                    {
                        playerDamaged = true;
                        damageAmount = enemy.GetDamage();
                        break;
                    }

                    /* Dégâts de projectile */
                    for (const auto& proj : enemy.GetProjectiles())
                    {
                        if (proj.active)
                        {
                            Rectangle projRect = { proj.position.x - proj.radius, proj.position.y - proj.radius, proj.radius * 2.0f, proj.radius * 2.0f };
                            if (CheckCollisionRecs(player.GetCollisionRect(), projRect))
                            {
                                playerDamaged = true;
                                damageAmount = enemy.GetDamage();
                                break;
                            }
                        }
                    }
                    
                    if (playerDamaged)
                    {
                        break;
                    }
                }
            }

            if (playerDamaged)
            {
                player.SetHealth(player.GetHealth() - damageAmount);
                playerHitCooldown = 1.0f; /* 1 seconde d'invulnérabilité */
                hud.TriggerNotification("AIE !", 1.0f);
            }
        }
    }
}
