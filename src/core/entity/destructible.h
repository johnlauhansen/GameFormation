#pragma once
#include <raylib.h>
#include <vector>
#include "inventory.h"

/*
 * Énumération décrivant la catégorie d'un objet destructible.
 */
enum class DestructibleType
{
    Crate,   /* Caisse - sensible aux dégâts contondants (Blunt) */
    Plant,   /* Plante - sensible aux dégâts tranchants (Slashing) */
    Custom   /* Objet personnalisé avec vulnérabilités configurables */
};

/*
 * Classe représentant un objet destructible sur la carte (ex: caisse en bois, plante coupable).
 * Change de représentation visuelle (sprite procédural) selon ses points de vie.
 * Peut être configuré pour être vulnérable à des types de dégâts ou des éléments spécifiques.
 */
class Destructible
{
public:
    /*
     * Constructeur.
     * Initialise l'objet à la position et au type donnés avec ses vulnérabilités par défaut.
     * @param[in] type Catégorie d'objet destructible.
     * @param[in] position Position centrale de l'objet dans le monde.
     */
    Destructible(DestructibleType type, Vector2 position);

    /*
     * Met à jour l'état temporel de l'objet (timers, clignotements).
     * @param[in] deltaTime Temps écoulé depuis la dernière frame.
     */
    void Update(float deltaTime);

    /*
     * Rendu graphique de l'objet. Dessine le sprite procédural correspondant
     * à son type et son niveau de dégâts actuels.
     */
    void Draw() const;

    /*
     * Tente d'infliger des dégâts à l'objet.
     * @param[in] amount Quantité brute de dégâts.
     * @param[in] dmgType Type physique de l'attaque (Tranchant, Contondant, Perçant).
     * @param[in] elemType Élément magique de l'attaque (Feu, Glace, Foudre, Aucun).
     * @return true si l'attaque a réussi à l'endommager, false sinon (immunité ou cooldown).
     */
    bool TakeDamage(float amount, DamageType dmgType, ElementType elemType);

    /*
     * Récupère la boîte de collision physique (AABB) de l'objet.
     * @return Le rectangle de collision.
     */
    [[nodiscard]] Rectangle GetCollisionRect() const;

    /*
     * Vérifie si l'objet est toujours en vie.
     * @return true s'il lui reste des PV, false s'il est détruit.
     */
    [[nodiscard]] bool IsAlive() const
    {
        return m_currentHealth > 0.0f;
    }

    /*
     * Récupère le ratio de santé actuel de l'objet (compris entre 0.0f et 1.0f).
     * @return Le pourcentage de PV restants.
     */
    [[nodiscard]] float GetHealthPercent() const
    {
        return m_currentHealth / m_maxHealth;
    }

    /*
     * Récupère la catégorie de l'objet destructible.
     */
    [[nodiscard]] DestructibleType GetType() const
    {
        return m_type;
    }

    /*
     * Récupère la position dans le monde de l'objet.
     */
    [[nodiscard]] Vector2 GetPosition() const
    {
        return m_position;
    }

    /*
     * Ajoute une vulnérabilité physique personnalisée (uniquement pour le type Custom).
     */
    void AddVulnerableDamageType(DamageType type);

    /*
     * Ajoute une vulnérabilité élémentaire magique personnalisée (uniquement pour le type Custom).
     */
    void AddVulnerableElement(ElementType element);

    /*
     * Modifie les points de vie max et actuels de l'objet.
     */
    void SetMaxHealth(float maxHealth);

private:
    /* Rendu d'une caisse en bois avec des détails de fissures progressives */
    void DrawCrate() const;

    /* Rendu d'une plante herbeuse avec des feuilles qui tombent progressivement */
    void DrawPlant() const;

    /* Rendu d'un monument magique violet personnalisé */
    void DrawCustom() const;

    DestructibleType m_type;            /* Catégorie de l'objet */
    Vector2 m_position;                 /* Position centrale de l'objet */
    float m_width;                      /* Largeur de la boîte de collision */
    float m_height;                     /* Hauteur de la boîte de collision */

    float m_maxHealth;                  /* PV maximum de l'objet */
    float m_currentHealth;              /* PV restants de l'objet */

    std::vector<DamageType> m_vulnerableDamageTypes; /* Types physiques acceptés (vide = tous) */
    std::vector<ElementType> m_vulnerableElements;    /* Éléments magiques acceptés (vide = tous) */

    float m_hitCooldownTimer;           /* Cooldown temporel pour éviter de prendre des dégâts chaque frame */
    float m_damageFlashTimer;           /* Timer pour faire clignoter l'objet en rouge lors d'un impact */
};
