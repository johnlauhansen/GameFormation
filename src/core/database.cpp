#include "database.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

using json = nlohmann::json;

std::unordered_map<std::string, EnemyTemplate> Database::s_enemies;
std::unordered_map<std::string, std::vector<std::string>> Database::s_dialogues;
bool Database::s_initialized = false;

void Database::Initialize()
{
    if (s_initialized) return;

    /* Chargement des ennemis */
    try
    {
        std::ifstream file("assets/databases/enemies.json");
        if (file.is_open())
        {
            json j;
            file >> j;
            for (auto& el : j.items())
            {
                EnemyTemplate tmpl;
                tmpl.health = el.value().value("health", 10.0f);
                tmpl.damage = el.value().value("damage", 5.0f);
                tmpl.speed = el.value().value("speed", 50.0f);
                tmpl.chaseRadius = el.value().value("chase_radius", 0.0f);
                tmpl.width = el.value().value("width", 32.0f);
                tmpl.height = el.value().value("height", 32.0f);
                tmpl.shootInterval = el.value().value("shoot_interval", 0.0f);
                tmpl.behavior = el.value().value("behavior", "Slime");
                s_enemies[el.key()] = tmpl;
            }
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Erreur chargement enemies.json: " << e.what() << std::endl;
    }

    /* Chargement des dialogues */
    try
    {
        std::ifstream file("assets/databases/dialogues.json");
        if (file.is_open())
        {
            json j;
            file >> j;
            for (auto& el : j.items())
            {
                std::vector<std::string> lines;
                for (auto& line : el.value())
                {
                    lines.push_back(line.get<std::string>());
                }
                s_dialogues[el.key()] = lines;
            }
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Erreur chargement dialogues.json: " << e.what() << std::endl;
    }

    s_initialized = true;
}

EnemyTemplate Database::GetEnemyTemplate(const std::string& enemyId)
{
    Initialize();
    auto it = s_enemies.find(enemyId);
    if (it != s_enemies.end())
        return it->second;
    return EnemyTemplate(); // Fallback par defaut
}

std::vector<std::string> Database::GetDialogues(const std::string& dialogueId)
{
    Initialize();
    auto it = s_dialogues.find(dialogueId);
    if (it != s_dialogues.end())
        return it->second;
    return {};
}
