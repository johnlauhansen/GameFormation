#pragma once
#include <string>
#include <vector>
#include <unordered_map>

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

class Database
{
public:
    static void Initialize();

    [[nodiscard]] static EnemyTemplate GetEnemyTemplate(const std::string& enemyId);
    [[nodiscard]] static std::vector<std::string> GetDialogues(const std::string& dialogueId);

private:
    static std::unordered_map<std::string, EnemyTemplate> s_enemies;
    static std::unordered_map<std::string, std::vector<std::string>> s_dialogues;
    static bool s_initialized;
};
