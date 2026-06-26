#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <raylib.h>

/*
 * Structure contenant les informations de génération d'une entité ou d'un objet interactif.
 * Mappée directement depuis les couches d'objets (Object Layers) de Tiled Map Editor.
 */
struct EntitySpawnInfo
{
    std::string type;       /* Catégorie générique de l'entité (ex: "PlayerSpawn", "Enemy", "Chest", "Trigger") */
    std::string subType;    /* Sous-type ou nom de l'entité (ex: "sword", "boomerang", "Octorok", "Moblin") */
    Vector2 position = { 0.0f, 0.0f }; /* Coordonnées de spawn en pixels dans l'espace monde */

    /* Propriétés personnalisées optionnelles pour le gameplay Zelda */
    int targetMapId = 0;              /* ID de la carte de destination pour un téléporteur */
    Vector2 targetSpawn = { 0.0f, 0.0f }; /* Coordonnées de spawn sur la carte de destination */
    std::string itemId = "";          /* ID de l'objet contenu dans un coffre ou disponible au sol */

    /* Extension Data-Driven : Attributs libres définis dans Tiled */
    std::unordered_map<std::string, std::string> properties;

    /* Méthode utilitaire : retourne une propriété ou une chaîne vide par défaut */
    [[nodiscard]] std::string GetProperty(const std::string& key, const std::string& defaultVal = "") const
    {
        auto it = properties.find(key);
        if (it != properties.end())
            return it->second;
        return defaultVal;
    }
};

/*
 * Structure représentant un niveau de jeu complet chargé dynamiquement.
 * Regroupe les données de tuiles, le masque de collision physique et les spawn d'objets.
 */
struct MapLevel
{
    int width = 0;          /* Largeur de la carte, en nombre de tuiles */
    int height = 0;         /* Hauteur de la carte, en nombre de tuiles */
    std::vector<int> tiles; /* Données de la couche de sol visuelle (IDs de textures linéaires) */
    std::vector<bool> collisionMap; /* Masque de collisions physiques (true = obstacle solide) */
    std::vector<EntitySpawnInfo> spawns; /* Liste de tous les points de spawn d'objets/ennemis */
};
