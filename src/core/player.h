#pragma once
#include <raylib.h>
#include "inventory.h"

enum class PlayerState
{
    Idle,
    Walking,
    Attacking
};

enum class Direction
{
    Up,
    Down,
    Left,
    Right
};

class Player
{
public:
    Player(Vector2 startPosition);

    void Update(float deltaTime);
    void Draw() const;

    [[nodiscard]] Rectangle GetCollisionRect() const;
    [[nodiscard]] Rectangle GetAttackRect() const;

    [[nodiscard]] Vector2 GetPosition() const
    {
        return m_position;
    }

    void SetPosition(Vector2 pos)
    {
        m_position = pos;
    }

    [[nodiscard]] PlayerState GetState() const
    {
        return m_state;
    }

    [[nodiscard]] Direction GetDirection() const
    {
        return m_direction;
    }

    [[nodiscard]] Inventory& GetInventory()
    {
        return m_inventory;
    }

    [[nodiscard]] const Inventory& GetInventory() const
    {
        return m_inventory;
    }

private:
    Vector2 m_position;
    float m_speed;
    float m_width;
    float m_height;

    PlayerState m_state;
    Direction m_direction;

    float m_attackTimer;
    const float m_attackDuration;

    Inventory m_inventory;
};
