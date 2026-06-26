#pragma once
#include <raylib.h>
#include <vector>
#include <string>
#include <memory>

enum class NpcType
{
    Villager,
    QuestGiver,
    Merchant
};

enum class QuestState
{
    NotStarted,
    InProgress,
    ReadyToComplete,
    Completed
};

struct Quest
{
    std::string id;
    std::string description;
    QuestState state = QuestState::NotStarted;
    int requiredKillCount = 0;
    int currentKillCount = 0;
    int rewardRupees = 0;
    int rewardPoints = 0;
};

struct MerchantItem
{
    std::string itemId;
    std::string name;
    int price = 0;
    std::string description;
};

enum class MovementType
{
    Static,
    PatrolZone,
    DefinedPath
};

class Npc
{
public:
    Npc(const std::string& name, NpcType type, Vector2 startPos);

    void Update(float deltaTime, const class TileMap& tileMap);
    void Draw() const;

    /* Interaction avec le joueur */
    void Interact(class Player& player);
    
    /* Vérifier si le joueur est proche */
    [[nodiscard]] bool IsPlayerNear(Vector2 playerPos) const;

    [[nodiscard]] Rectangle GetCollisionRect() const;
    
    [[nodiscard]] Vector2 GetPosition() const
    {
        return m_position;
    }

    [[nodiscard]] const std::string& GetName() const
    {
        return m_name;
    }

    [[nodiscard]] NpcType GetType() const
    {
        return m_type;
    }

    /* Interface des dialogues */
    [[nodiscard]] bool IsInDialogue() const
    {
        return m_isInteracting;
    }

    void CloseDialogue()
    {
        m_isInteracting = false;
        m_dialogueIndex = 0;
        m_shopActive = false;
    }

    [[nodiscard]] std::string GetCurrentDialogueText() const;
    void AdvanceDialogue(class Player& player);

    /* Configuration des mouvements */
    void SetStatic();
    void SetPatrolZone(float radius, float speed);
    void SetDefinedPath(const std::vector<Vector2>& waypoints, float speed);

    /* Configuration des quêtes */
    void ConfigureQuest(const std::string& questId, const std::string& description, int requiredKills, int rewardRupees, int rewardPoints);
    
    [[nodiscard]] Quest* GetQuest()
    {
        return m_quest ? m_quest.get() : nullptr;
    }

    [[nodiscard]] const Quest* GetQuest() const
    {
        return m_quest ? m_quest.get() : nullptr;
    }

    /* Configuration du marchand */
    void AddMerchantItem(const std::string& itemId, const std::string& name, int price, const std::string& description);
    
    [[nodiscard]] const std::vector<MerchantItem>& GetMerchantItems() const
    {
        return m_merchantItems;
    }

    [[nodiscard]] bool IsMerchant() const
    {
        return m_type == NpcType::Merchant;
    }

    [[nodiscard]] bool IsShopActive() const
    {
        return m_shopActive;
    }

    [[nodiscard]] int GetSelectedShopIndex() const
    {
        return m_selectedShopIndex;
    }

    void SetSelectedShopIndex(int index)
    {
        m_selectedShopIndex = index;
    }

    void SetDefaultDialogues(const std::vector<std::string>& dialogues)
    {
        m_defaultDialogues = dialogues;
    }

private:
    std::string m_name;
    NpcType m_type;
    Vector2 m_position;
    Vector2 m_startPosition;
    float m_width;
    float m_height;

    /* Comportement de mouvement */
    MovementType m_movementType;
    float m_speed;
    
    /* Zone de patrouille */
    float m_patrolRadius;
    Vector2 m_patrolTarget;
    float m_patrolWaitTimer;

    /* Chemin prédéfini */
    std::vector<Vector2> m_waypoints;
    size_t m_currentWaypointIndex;
    bool m_pathForward;

    /* Interactivité */
    bool m_isInteracting;
    size_t m_dialogueIndex;
    std::vector<std::string> m_defaultDialogues;

    /* Quête */
    std::unique_ptr<Quest> m_quest;

    /* Marchand */
    std::vector<MerchantItem> m_merchantItems;
    int m_selectedShopIndex;
    bool m_shopActive;

    void UpdateMovement(float deltaTime, const class TileMap& tileMap);
};
