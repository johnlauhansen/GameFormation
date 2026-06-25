#pragma once
#include "map_data.h"
#include <optional>
#include <string>

/*
 * Classe utilitaire statique dédiée au chargement et au parsing des fichiers de cartes.
 * Permet de décoder les fichiers JSON exportés de Tiled Map Editor vers nos structures C++.
 */
class MapLoader
{
public:
    /*
     * Charge une carte Tiled au format JSON et l'analyse pour construire une structure MapLevel.
     * Cette fonction extrait les couches de tuiles de sol, calcule les collisions physiques et lit les objets.
     * @param[in] filePath Chemin d'accès relatif vers le fichier de carte JSON.
     * @return Une structure MapLevel si le parsing réussit, std::nullopt sinon.
     */
    [[nodiscard]] static std::optional<MapLevel> LoadFromJson(const std::string& filePath);
};
