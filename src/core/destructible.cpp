#include "destructible.h"

Destructible::Destructible(DestructibleType type, Vector2 position)
    : m_type(type)
    , m_position(position)
    , m_width(32.0f)
    , m_height(32.0f)
    , m_maxHealth(30.0f)
    , m_currentHealth(30.0f)
    , m_hitCooldownTimer(0.0f)
    , m_damageFlashTimer(0.0f)
{
    /* Applique les vulnérabilités par défaut selon le type de destructible */
    if (m_type == DestructibleType::Crate)
    {
        m_vulnerableDamageTypes.push_back(DamageType::Blunt);
    }
    else if (m_type == DestructibleType::Plant)
    {
        m_vulnerableDamageTypes.push_back(DamageType::Slashing);
    }
}

void Destructible::Update(float deltaTime)
{
    if (m_hitCooldownTimer > 0.0f)
    {
        m_hitCooldownTimer -= deltaTime;
    }

    if (m_damageFlashTimer > 0.0f)
    {
        m_damageFlashTimer -= deltaTime;
    }
}

void Destructible::Draw() const
{
    if (!IsAlive())
    {
        return;
    }

    switch (m_type)
    {
        case DestructibleType::Crate:
        {
            DrawCrate();
            break;
        }
        case DestructibleType::Plant:
        {
            DrawPlant();
            break;
        }
        case DestructibleType::Custom:
        {
            DrawCustom();
            break;
        }
    }
}

bool Destructible::TakeDamage(float amount, DamageType dmgType, ElementType elemType)
{
    if (!IsAlive())
    {
        return false;
    }

    if (m_hitCooldownTimer > 0.0f)
    {
        return false;
    }

    bool isVulnerable = false;

    /* 1. Vérification de la vulnérabilité physique */
    if (!m_vulnerableDamageTypes.empty())
    {
        for (const auto& t : m_vulnerableDamageTypes)
        {
            if (t == dmgType)
            {
                isVulnerable = true;
                break;
            }
        }
    }

    /* 2. Vérification de la vulnérabilité magique/élémentaire */
    if (!m_vulnerableElements.empty())
    {
        for (const auto& e : m_vulnerableElements)
        {
            if (e == elemType)
            {
                isVulnerable = true;
                break;
            }
        }
    }

    /* 3. Si aucune liste n'est spécifiée, l'objet est vulnérable par défaut à tout */
    if (m_vulnerableDamageTypes.empty() && m_vulnerableElements.empty())
    {
        isVulnerable = true;
    }

    if (isVulnerable)
    {
        m_currentHealth -= amount;
        if (m_currentHealth < 0.0f)
        {
            m_currentHealth = 0.0f;
        }

        m_hitCooldownTimer = 0.25f; /* Empêche les dégâts multi-frames instantanés */
        m_damageFlashTimer = 0.15f; /* Enclenche le clignotement d'impact */
        return true;
    }

    return false;
}

Rectangle Destructible::GetCollisionRect() const
{
    return { m_position.x - (m_width / 2.0f), m_position.y - (m_height / 2.0f), m_width, m_height };
}

void Destructible::AddVulnerableDamageType(DamageType type)
{
    if (m_type == DestructibleType::Custom)
    {
        m_vulnerableDamageTypes.push_back(type);
    }
}

void Destructible::AddVulnerableElement(ElementType element)
{
    if (m_type == DestructibleType::Custom)
    {
        m_vulnerableElements.push_back(element);
    }
}

void Destructible::SetMaxHealth(float maxHealth)
{
    m_maxHealth = maxHealth;
    m_currentHealth = maxHealth;
}

void Destructible::DrawCrate() const
{
    const Rectangle rect = GetCollisionRect();

    Color baseColor = Color{ 139, 90, 43, 255 };  /* Couleur de base : Brun bois chaud */
    Color borderColor = Color{ 89, 50, 20, 255 }; /* Brun foncé pour les renforts */

    if (m_damageFlashTimer > 0.0f)
    {
        baseColor = RED;
        borderColor = WHITE;
    }

    DrawRectangleRec(rect, baseColor);
    DrawRectangleLinesEx(rect, 2.5f, borderColor);

    const float hpPercent = GetHealthPercent();
    if (hpPercent >= 1.0f)
    {
        /* Caisse saine : Une croix de renforcement nette */
        DrawLineEx({ rect.x + 3.0f, rect.y + 3.0f }, { rect.x + rect.width - 3.0f, rect.y + rect.height - 3.0f }, 2.0f, borderColor);
        DrawLineEx({ rect.x + rect.width - 3.0f, rect.y + 3.0f }, { rect.x + 3.0f, rect.y + rect.height - 3.0f }, 2.0f, borderColor);
    }
    else if (hpPercent >= 0.5f)
    {
        /* Caisse abîmée : Diagonale conservée et première fissure sombre */
        DrawLineEx({ rect.x + 3.0f, rect.y + 3.0f }, { rect.x + rect.width - 3.0f, rect.y + rect.height - 3.0f }, 2.0f, borderColor);
        DrawLineEx({ rect.x + 6.0f, rect.y + 12.0f }, { rect.x + 16.0f, rect.y + 10.0f }, 1.5f, BLACK);
        DrawLineEx({ rect.x + 16.0f, rect.y + 10.0f }, { rect.x + 12.0f, rect.y + 20.0f }, 1.5f, BLACK);
    }
    else if (hpPercent > 0.0f)
    {
        /* Caisse fortement lézardée : Pas de diagonale, fissures multiples */
        DrawLineEx({ rect.x + 6.0f, rect.y + 12.0f }, { rect.x + 16.0f, rect.y + 10.0f }, 1.5f, BLACK);
        DrawLineEx({ rect.x + 16.0f, rect.y + 10.0f }, { rect.x + 22.0f, rect.y + 24.0f }, 1.5f, BLACK);
        
        DrawLineEx({ rect.x + rect.width - 6.0f, rect.y + 10.0f }, { rect.x + rect.width - 16.0f, rect.y + 18.0f }, 1.5f, BLACK);
        DrawLineEx({ rect.x + rect.width - 16.0f, rect.y + 18.0f }, { rect.x + rect.width - 10.0f, rect.y + 26.0f }, 1.5f, BLACK);
    }
}

void Destructible::DrawPlant() const
{
    const Rectangle rect = GetCollisionRect();

    Color plantColor = Color{ 46, 125, 50, 255 }; /* Vert forêt vif */
    Color stemColor = Color{ 27, 94, 32, 255 };    /* Vert sombre pour la tige */

    if (m_damageFlashTimer > 0.0f)
    {
        plantColor = RED;
        stemColor = WHITE;
    }

    const float hpPercent = GetHealthPercent();
    const Vector2 center = { rect.x + rect.width / 2.0f, rect.y + rect.height / 2.0f };

    /* Dessin de la tige de base */
    DrawLineEx({ center.x, rect.y + rect.height }, { center.x, center.y }, 3.5f, stemColor);

    if (hpPercent >= 1.0f)
    {
        /* Plante intacte : 3 belles feuilles rondes */
        DrawCircleV({ center.x - 8.0f, center.y }, 9.0f, plantColor);
        DrawCircleV({ center.x + 8.0f, center.y }, 9.0f, plantColor);
        DrawCircleV({ center.x, center.y - 8.0f }, 9.0f, plantColor);
        DrawCircleV(center, 4.0f, GOLD); /* Cœur doré de la plante */
    }
    else if (hpPercent >= 0.5f)
    {
        /* Plante endommagée : Plus que 2 feuilles de couleur vert olive terne */
        const Color oliveGreen = Color{ 85, 139, 47, 255 };
        DrawCircleV({ center.x - 6.0f, center.y + 2.0f }, 8.0f, oliveGreen);
        DrawCircleV({ center.x, center.y - 6.0f }, 8.0f, oliveGreen);
        DrawCircleV(center, 3.0f, Color{ 174, 182, 70, 255 });
    }
    else if (hpPercent > 0.0f)
    {
        /* Plante quasi coupée : 1 seule feuille pendante desséchée (couleur jaune/marron) */
        const Color driedColor = Color{ 130, 119, 23, 255 };
        DrawCircleV({ center.x + 4.0f, center.y + 4.0f }, 7.0f, driedColor);
        DrawCircleV(center, 2.0f, driedColor);
    }
}

void Destructible::DrawCustom() const
{
    const Rectangle rect = GetCollisionRect();

    Color baseColor = Color{ 156, 39, 176, 255 };  /* Violet magique */
    Color borderColor = Color{ 74, 20, 140, 255 }; /* Violet très foncé */

    if (m_damageFlashTimer > 0.0f)
    {
        baseColor = RED;
        borderColor = WHITE;
    }

    DrawRectangleRec(rect, baseColor);
    DrawRectangleLinesEx(rect, 2.5f, borderColor);

    /* Symbole mystique au centre (un losange d'or) */
    const Vector2 center = { rect.x + rect.width / 2.0f, rect.y + rect.height / 2.0f };
    const float hpPercent = GetHealthPercent();
    if (hpPercent >= 0.5f)
    {
        DrawCircleV(center, 6.0f, GOLD);
    }
    else if (hpPercent > 0.0f)
    {
        DrawCircleV(center, 3.0f, ORANGE);
    }
}
