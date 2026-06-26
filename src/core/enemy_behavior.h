#pragma once
#include <raylib.h>
#include <vector>

class Enemy;
class TileMap;

class EnemyBehavior
{
public:
    virtual ~EnemyBehavior() = default;

    /* Interface polymorphique pour l'IA et le dessin procédural */
    virtual void Update(Enemy& context, float deltaTime, const TileMap& tileMap, const Vector2& playerPos) = 0;
    virtual void Draw(const Enemy& context, Color bodyColor) const = 0;
};

class SlimeBehavior : public EnemyBehavior
{
public:
    void Update(Enemy& context, float deltaTime, const TileMap& tileMap, const Vector2& playerPos) override;
    void Draw(const Enemy& context, Color bodyColor) const override;
};

class OctorokBehavior : public EnemyBehavior
{
public:
    void Update(Enemy& context, float deltaTime, const TileMap& tileMap, const Vector2& playerPos) override;
    void Draw(const Enemy& context, Color bodyColor) const override;
};

class MoblinBehavior : public EnemyBehavior
{
public:
    void Update(Enemy& context, float deltaTime, const TileMap& tileMap, const Vector2& playerPos) override;
    void Draw(const Enemy& context, Color bodyColor) const override;
};
