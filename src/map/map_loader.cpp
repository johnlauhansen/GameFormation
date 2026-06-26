#include "map_loader.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

using json = nlohmann::json;

/*
 * Décodage et parsing des fichiers JSON exportés de Tiled Map Editor.
 * Charge les tuiles, les masques de collisions et les métadonnées d'objets.
 */
std::optional<MapLevel> MapLoader::LoadFromJson(const std::string& filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        return std::nullopt;
    }

    try
    {
        json j;
        file >> j;

        MapLevel level;
        level.width = j.at("width").get<int>();
        level.height = j.at("height").get<int>();

        /* 1. On parcourt les calques (layers) du fichier Tiled JSON */
        for (const auto& layer : j.at("layers"))
        {
            std::string type = layer.at("type").get<std::string>();
            std::string name = layer.at("name").get<std::string>();

            /* Lecture des calques de tuiles (grilles) */
            if (type == "tilelayer")
            {
                std::vector<int> data = layer.at("data").get<std::vector<int>>();

                /* Calque visuel principal représentant le décor au sol */
                if (name == "Background" || name == "Ground")
                {
                    level.tiles = data;
                }

                /* Calque physique représentant les obstacles infranchissables */
                if (name == "Collisions" || name == "Solid")
                {
                    level.collisionMap.resize(data.size());
                    for (size_t i = 0; i < data.size(); ++i)
                    {
                        level.collisionMap[i] = (data[i] != 0);
                    }
                }
            }
            /* Lecture des calques d'objets (points d'intérêt, spawn, coffres, PNJ) */
            else if (type == "objectgroup")
            {
                for (const auto& obj : layer.at("objects"))
                {
                    EntitySpawnInfo spawn;
                    spawn.type = obj.value("type", "");
                    if (spawn.type.empty())
                    {
                        spawn.type = obj.value("class", ""); /* Gère la rétrocompatibilité Tiled 1.9+ ("class") */
                    }
                    spawn.subType = obj.value("name", "");
                    spawn.position.x = obj.value("x", 0.0f);
                    spawn.position.y = obj.value("y", 0.0f);

                    /* Lecture des propriétés personnalisées (Custom Properties) définies sous Tiled */
                    if (obj.contains("properties"))
                    {
                        for (const auto& prop : obj.at("properties"))
                        {
                            std::string propName = prop.at("name").get<std::string>();
                            
                            /* Extraction générique de la propriété en string vers le dictionnaire */
                            std::string propValueStr;
                            if (prop.at("value").is_string())
                            {
                                propValueStr = prop.at("value").get<std::string>();
                            }
                            else if (prop.at("value").is_number())
                            {
                                double val = prop.at("value").get<double>();
                                if (val == std::floor(val))
                                {
                                    propValueStr = std::to_string((int)val);
                                }
                                else
                                {
                                    propValueStr = std::to_string(val);
                                }
                            }
                            else if (prop.at("value").is_boolean())
                            {
                                propValueStr = prop.at("value").get<bool>() ? "1" : "0";
                            }
                            
                            spawn.properties[propName] = propValueStr;

                            /* Mappage direct pour rétrocompatibilité avec l'ancien système de spawn dur */
                            if (propName == "itemId")
                            {
                                spawn.itemId = propValueStr;
                            }
                            else if (propName == "targetMapId")
                            {
                                spawn.targetMapId = prop.at("value").get<int>();
                            }
                            else if (propName == "targetSpawnX")
                            {
                                spawn.targetSpawn.x = prop.at("value").get<float>();
                            }
                            else if (propName == "targetSpawnY")
                            {
                                spawn.targetSpawn.y = prop.at("value").get<float>();
                            }
                        }
                    }

                    level.spawns.push_back(spawn);
                }
            }
        }

        /* 2. Rétrocompatibilité : Si aucun calque de collision n'a été spécifié sous Tiled,
         * on calcule automatiquement le masque physique à partir des IDs de tuiles par défaut. */
        if (level.collisionMap.empty() && !level.tiles.empty())
        {
            level.collisionMap.resize(level.tiles.size(), false);
            for (size_t i = 0; i < level.tiles.size(); ++i)
            {
                /* Les tuiles 1 (mur pierre) et 2 (buisson d'arbres) sont solides */
                level.collisionMap[i] = (level.tiles[i] == 1 || level.tiles[i] == 2);
            }
        }

        return level;
    }
    catch (const std::exception& e)
    {
        (void)e; /* Élimine les avertissements de variables non utilisées */
        return std::nullopt;
    }
}
