#include "player.h"
#include <cmath>

Player::Player(Vector2 startPosition)
    : m_position(startPosition)
    , m_speed(200.0f)
    , m_width(32.0f)
    , m_height(32.0f)
    , m_state(PlayerState::Idle)
    , m_direction(Direction::Down)
    , m_attackTimer(0.0f)
    , m_attackDuration(0.25f)
    , m_health(100.0f)
    , m_maxHealth(100.0f)
    , m_magic(100.0f)
    , m_maxMagic(100.0f)
    , m_rupees(0)
{
}

void Player::Update(float deltaTime)
{
    /* Régénération passive de la magie */
    m_magic += 5.0f * deltaTime;
    if (m_magic > m_maxMagic)
    {
        m_magic = m_maxMagic;
    }

    /* Touches de débogage pour tester le HUD */
    if (IsKeyPressed(KEY_K))
    {
        SetHealth(m_health - 10.0f); /* Enlever un demi-cœur */
    }
    if (IsKeyPressed(KEY_H))
    {
        SetHealth(m_health + 10.0f); /* Régénérer un demi-cœur */
    }
    if (IsKeyPressed(KEY_P))
    {
        AddRupees(15); /* Gagner 15 rubis */
    }

    if (m_state == PlayerState::Attacking)
    {
        m_attackTimer -= deltaTime;
        if (m_attackTimer <= 0.0f)
        {
            m_state = PlayerState::Idle;
        }
    }
    else
    {
        Vector2 movement = { 0.0f, 0.0f };

        /* 1. Clavier */
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
        {
            movement.x += 1.0f;
            m_direction = Direction::Right;
        }
        if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
        {
            movement.x -= 1.0f;
            m_direction = Direction::Left;
        }
        if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))
        {
            movement.y += 1.0f;
            m_direction = Direction::Down;
        }
        if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))
        {
            movement.y -= 1.0f;
            m_direction = Direction::Up;
        }

        /* 2. Manette (Gamepad 0) */
        if (IsGamepadAvailable(0))
        {
            /* Joystick gauche analogique */
            const float axisX = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);
            const float axisY = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y);
            const float deadzone = 0.20f;

            if (std::abs(axisX) > deadzone)
            {
                movement.x = axisX;
                m_direction = (axisX > 0.0f) ? Direction::Right : Direction::Left;
            }
            if (std::abs(axisY) > deadzone)
            {
                movement.y = axisY;
                m_direction = (axisY > 0.0f) ? Direction::Down : Direction::Up;
            }

            /* D-Pad (Croix directionnelle) */
            if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_UP))
            {
                movement.y = -1.0f;
                m_direction = Direction::Up;
            }
            else if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN))
            {
                movement.y = 1.0f;
                m_direction = Direction::Down;
            }

            if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_LEFT))
            {
                movement.x = -1.0f;
                m_direction = Direction::Left;
            }
            else if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_RIGHT))
            {
                movement.x = 1.0f;
                m_direction = Direction::Right;
            }
        }

        /* 3. Application des mouvements */
        if (movement.x != 0.0f || movement.y != 0.0f)
        {
            const float length = std::sqrt((movement.x * movement.x) + (movement.y * movement.y));
            m_state = PlayerState::Walking;

            const float multiplier = (length > 1.0f) ? (1.0f / length) : 1.0f;
            m_position.x += movement.x * multiplier * m_speed * deltaTime;
            m_position.y += movement.y * multiplier * m_speed * deltaTime;
        }
        else
        {
            m_state = PlayerState::Idle;
        }

        /* 4. Attaque à l'épée (Uniquement si l'épée est collectée et équipée !) */
        if (m_inventory.HasItem("sword"))
        {
            bool isAttackPressed = IsKeyPressed(KEY_SPACE);
            if (IsGamepadAvailable(0))
            {
                if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_LEFT) || 
                    IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN))
                {
                    isAttackPressed = true;
                }
            }

            if (isAttackPressed)
            {
                m_state = PlayerState::Attacking;
                m_attackTimer = m_attackDuration;
            }
        }
    }
}

void Player::Draw() const
{
    Color playerColor = GREEN;
    if (m_state == PlayerState::Attacking)
    {
        playerColor = ORANGE;
    }
    DrawRectangleRec(GetCollisionRect(), playerColor);

    float indicatorSize = 6.0f;
    Rectangle rect = GetCollisionRect();
    Vector2 indicatorPos = { rect.x + (rect.width / 2.0f), rect.y + (rect.height / 2.0f) };

    if (m_direction == Direction::Up)
    {
        indicatorPos.y -= rect.height / 2.0f;
    }
    else if (m_direction == Direction::Down)
    {
        indicatorPos.y += rect.height / 2.0f;
    }
    else if (m_direction == Direction::Left)
    {
        indicatorPos.x -= rect.width / 2.0f;
    }
    else if (m_direction == Direction::Right)
    {
        indicatorPos.x += rect.width / 2.0f;
    }
    DrawCircleV(indicatorPos, indicatorSize, BLACK);

    /* Dessiner l'épée si en cours d'attaque */
    if (m_state == PlayerState::Attacking)
    {
        Color attackColor = RED;
        const Item* sword = m_inventory.GetItem("sword");
        if (sword != nullptr)
        {
            if (sword->element == ElementType::Fire)
            {
                attackColor = ORANGE;
            }
            else if (sword->element == ElementType::Ice)
            {
                attackColor = SKYBLUE;
            }
            else if (sword->element == ElementType::Lightning)
            {
                attackColor = GOLD;
            }
        }
        DrawRectangleRec(GetAttackRect(), attackColor);
    }
}

Rectangle Player::GetCollisionRect() const
{
    return { m_position.x - (m_width / 2.0f), m_position.y - (m_height / 2.0f), m_width, m_height };
}

Rectangle Player::GetAttackRect() const
{
    Rectangle body = GetCollisionRect();
    float range = 24.0f;
    float thickness = 12.0f;

    /* Lecture dynamique de la portée de l'épée améliorée */
    const Item* sword = m_inventory.GetItem("sword");
    if (sword != nullptr && sword->collected)
    {
        range = sword->range;
    }

    Rectangle attackRect = { 0.0f, 0.0f, 0.0f, 0.0f };

    if (m_direction == Direction::Up)
    {
        attackRect = { body.x + (body.width / 2.0f) - (thickness / 2.0f), body.y - range, thickness, range };
    }
    else if (m_direction == Direction::Down)
    {
        attackRect = { body.x + (body.width / 2.0f) - (thickness / 2.0f), body.y + body.height, thickness, range };
    }
    else if (m_direction == Direction::Left)
    {
        attackRect = { body.x - range, body.y + (body.height / 2.0f) - (thickness / 2.0f), range, thickness };
    }
    else if (m_direction == Direction::Right)
    {
        attackRect = { body.x + body.width, body.y + (body.height / 2.0f) - (thickness / 2.0f), range, thickness };
    }

    return attackRect;
}
