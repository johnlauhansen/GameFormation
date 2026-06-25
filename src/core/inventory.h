#pragma once
#include <string>
#include <vector>

/*
 * Énumération des éléments magiques applicables aux armes.
 * Permet d'infuser des effets élémentaires (visuels et statistiques) aux équipements.
 */
enum class ElementType
{
    None,      /* Attaque physique standard sans effet élémentaire */
    Fire,      /* Élément Feu - Hitbox d'attaque de couleur orange */
    Ice,       /* Élément Glace - Hitbox de couleur bleu givré */
    Lightning  /* Élément Foudre - Hitbox de couleur jaune électrique */
};

/*
 * Structure représentant une instance d'un objet (arme ou équipement).
 * Contient les statistiques améliorables à la forge ainsi que son état de collecte.
 */
struct Item
{
    std::string id;          /* Identifiant unique interne de l'objet (ex: "sword", "boomerang") */
    std::string name;        /* Nom d'affichage dans les menus de l'inventaire */
    bool collected = false;  /* Indique si l'objet a été ramassé par le joueur en jeu */

    /* Statistiques de combat dynamiques améliorables */
    float damage = 0.0f;     /* Dégâts infligés par l'arme */
    float speed = 0.0f;      /* Vitesse de vol (boomerang) ou cadence d'attaque (épée) */
    float range = 0.0f;      /* Portée de la hitbox d'attaque ou distance max de vol */
    ElementType element = ElementType::None; /* Affinité magique active */

    int level = 1;           /* Niveau d'amélioration général de l'arme */
};

/*
 * Classe gérant l'inventaire complet du joueur.
 * Stocke la liste des objets disponibles, l'équipement actif et les points d'amélioration de forge.
 */
class Inventory
{
public:
    /*
     * Constructeur par défaut.
     * Initialise l'inventaire en créant l'épée et le boomerang à l'état non collecté (par défaut).
     */
    Inventory()
    {
        Item sword;
        sword.id = "sword";
        sword.name = "Epee d'initiation";
        sword.collected = false;
        sword.damage = 10.0f;
        sword.speed = 1.0f;
        sword.range = 24.0f;
        sword.element = ElementType::None;
        sword.level = 1;

        Item boomerang;
        boomerang.id = "boomerang";
        boomerang.name = "Boomerang de voyage";
        boomerang.collected = false;
        boomerang.damage = 5.0f;
        boomerang.speed = 350.0f;
        boomerang.range = 150.0f;
        boomerang.element = ElementType::None;
        boomerang.level = 1;

        m_items.push_back(sword);
        m_items.push_back(boomerang);

        m_upgradePoints = 5; /* Donne 5 points d'amélioration initiaux utilisables à la forge */
    }

    /*
     * Ajoute un objet à l'inventaire (marqué comme collecté).
     * Équipe automatiquement l'objet dans l'emplacement correspondant s'il est vide.
     * @param[in] id L'identifiant unique de l'objet à collecter.
     */
    void AddItem(const std::string& id)
    {
        for (auto& item : m_items)
        {
            if (item.id == id)
            {
                item.collected = true;
                
                if (id == "sword" && m_equippedPrimaryId.empty())
                {
                    m_equippedPrimaryId = id;
                }
                else if (id == "boomerang" && m_equippedSecondaryId.empty())
                {
                    m_equippedSecondaryId = id;
                }
                break;
            }
        }
    }

    /*
     * Retire un objet de l'inventaire (marqué comme perdu) et le déséquipe.
     * @param[in] id L'identifiant unique de l'objet à retirer.
     */
    void RemoveItem(const std::string& id)
    {
        for (auto& item : m_items)
        {
            if (item.id == id)
            {
                item.collected = false;
                if (m_equippedPrimaryId == id)
                {
                    m_equippedPrimaryId = "";
                }
                if (m_equippedSecondaryId == id)
                {
                    m_equippedSecondaryId = "";
                }
                break;
            }
        }
    }

    /*
     * Vérifie si un objet spécifique a été collecté par le joueur.
     * @param[in] id L'identifiant unique de l'objet à vérifier.
     * @return true si l'objet est présent et collecté, false sinon.
     */
    [[nodiscard]] bool HasItem(const std::string& id) const
    {
        for (const auto& item : m_items)
        {
            if (item.id == id && item.collected)
            {
                return true;
            }
        }
        return false;
    }

    /*
     * Récupère un pointeur modifiable vers un objet de l'inventaire.
     * Utilisé principalement par les menus de forge pour appliquer des améliorations statistiques.
     * @param[in] id L'identifiant de l'objet à récupérer.
     * @return Un pointeur vers l'objet, ou nullptr s'il n'existe pas.
     */
    [[nodiscard]] Item* GetItem(const std::string& id)
    {
        for (auto& item : m_items)
        {
            if (item.id == id)
            {
                return &item;
            }
        }
        return nullptr;
    }

    /*
     * Récupère un pointeur en lecture seule vers un objet de l'inventaire (surcharge const).
     * Utilisé lors des rendus graphiques ou des vérifications d'état constantes.
     * @param[in] id L'identifiant de l'objet à récupérer.
     * @return Un pointeur constant vers l'objet, ou nullptr s'il n'existe pas.
     */
    [[nodiscard]] const Item* GetItem(const std::string& id) const
    {
        for (const auto& item : m_items)
        {
            if (item.id == id)
            {
                return &item;
            }
        }
        return nullptr;
    }

    std::string m_equippedPrimaryId = "";   /* Identifiant de l'arme principale active (Épée) */
    std::string m_equippedSecondaryId = ""; /* Identifiant de l'arme secondaire active (Boomerang) */
    std::vector<Item> m_items;              /* Liste complète des objets de quête possibles */
    int m_upgradePoints;                    /* Nombre de points d'amélioration de forge restants */
};
