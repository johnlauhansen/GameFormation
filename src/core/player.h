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

    [[nodiscard]] float GetHealth() const
    {
        return m_health;
    }

    void SetHealth(float hp)
    {
        m_health = hp;
        if (m_health < 0.0f) m_health = 0.0f;
        if (m_health > m_maxHealth) m_health = m_maxHealth;
    }

    [[nodiscard]] float GetMaxHealth() const
    {
        return m_maxHealth;
    }

    void SetMaxHealth(float maxHp)
    {
        m_maxHealth = maxHp;
        if (m_health > m_maxHealth) m_health = m_maxHealth;
    }

    [[nodiscard]] float GetMagic() const
    {
        return m_magic;
    }

    void SetMagic(float mp)
    {
        m_magic = mp;
        if (m_magic < 0.0f) m_magic = 0.0f;
        if (m_magic > m_maxMagic) m_magic = m_maxMagic;
    }

    [[nodiscard]] float GetMaxMagic() const
    {
        return m_maxMagic;
    }

    void SetMaxMagic(float maxMp)
    {
        m_maxMagic = maxMp;
        if (m_magic > m_maxMagic) m_magic = m_maxMagic;
    }

    [[nodiscard]] int GetRupees() const
    {
        return m_rupees;
    }

    void AddRupees(int amount)
    {
        m_rupees += amount;
        if (m_rupees < 0) m_rupees = 0;
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

    float m_health;
    float m_maxHealth;
    float m_magic;
    float m_maxMagic;
    int m_rupees;

    Inventory m_inventory;
};
