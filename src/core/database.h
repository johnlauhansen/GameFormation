#pragma once
#include <string>
#include <vector>
#include <unordered_map>

/*
 * Structure représentant les caractéristiques par défaut d'un type d'ennemi.
 * Chargée depuis le fichier JSON global pour l'équilibrage orienté données.
 */
struct EnemyTemplate
{
    float health = 10.0f;
    float damage = 5.0f;
    float speed = 50.0f;
    float chaseRadius = 0.0f;
    float width = 32.0f;
    float height = 32.0f;
    float shootInterval = 0.0f;
    std::string behavior = "Slime";
};

/*
 * Classe de gestion de la Base de Données globale du jeu.
 * Centralise et met en cache toutes les propriétés lues depuis les fichiers JSON d'assets.
 */
class Database
{
public:
    /*
     * Charge toutes les tables JSON de gabarits (Ennemis, Dialogues) en mémoire si ce n'est pas déjà fait.
     */
    static void Initialize();

    /*
     * Récupère le gabarit de statistiques d'un ennemi par son identifiant unique.
     * @param[in] enemyId L'identifiant de l'ennemi (ex: "Slime", "Moblin").
     * @return Une structure EnemyTemplate peuplée ou des valeurs par défaut.
     */
    [[nodiscard]] static EnemyTemplate GetEnemyTemplate(const std::string& enemyId);

    /*
     * Récupère la liste ordonnée des répliques de dialogue d'un PNJ par son identifiant de template.
     * @param[in] dialogueId L'identifiant unique des répliques (ex: "QuestGiver_Crates").
     * @return Un vecteur de chaînes contenant les lignes de dialogue.
     */
    [[nodiscard]] static std::vector<std::string> GetDialogues(const std::string& dialogueId);

private:
    static std::unordered_map<std::string, EnemyTemplate> s_enemies;
    static std::unordered_map<std::string, std::vector<std::string>> s_dialogues;
    static bool s_initialized;
};
