#include "npc.h"
#include "player.h"
#include "map/tile_map.h"
#include "event_system.h"
#include <cmath>
#include <raymath.h>

Npc::Npc(const std::string& name, NpcType type, Vector2 startPos)
    : m_name(name)
    , m_type(type)
    , m_position(startPos)
    , m_startPosition(startPos)
    , m_width(32.0f)
    , m_height(32.0f)
    , m_movementType(MovementType::Static)
    , m_speed(80.0f)
    , m_patrolRadius(0.0f)
    , m_patrolTarget(startPos)
    , m_patrolWaitTimer(0.0f)
    , m_currentWaypointIndex(0)
    , m_pathForward(true)
    , m_isInteracting(false)
    , m_dialogueIndex(0)
    , m_selectedShopIndex(0)
    , m_shopActive(false)
{
    /* Initialisation des dialogues par défaut */
    if (m_type == NpcType::Villager)
    {
        m_defaultDialogues = {
            "Bonjour voyageur !",
            "Bienvenue dans notre humble village.",
            "Le temps est magnifique aujourd'hui, n'est-ce pas ?"
        };
    }
    else if (m_type == NpcType::QuestGiver)
    {
        m_defaultDialogues = {
            "Halte-là ! J'ai besoin de votre aide !",
            "Des caisses sauvages encombrent mon jardin à l'est...",
            "Si vous détruisez 5 caisses en bois, je vous récompenserai !"
        };
    }
    else if (m_type == NpcType::Merchant)
    {
        m_defaultDialogues = {
            "Bienvenue dans ma boutique !",
            "J'ai des marchandises de premier choix pour vous.",
            "Jetez un oeil à mon inventaire !"
        };
    }
}

void Npc::Update(float deltaTime, const TileMap& tileMap)
{
    if (m_isInteracting)
    {
        /* Fige le mouvement lors de l'interaction */
        return;
    }

    UpdateMovement(deltaTime, tileMap);
}

void Npc::UpdateMovement(float deltaTime, const TileMap& tileMap)
{
    if (m_movementType == MovementType::Static)
    {
        return;
    }

    if (m_movementType == MovementType::PatrolZone)
    {
        if (m_patrolWaitTimer > 0.0f)
        {
            m_patrolWaitTimer -= deltaTime;
            return;
        }

        /* Calculer la distance au point cible */
        float dist = Vector2Distance(m_position, m_patrolTarget);
        if (dist < 5.0f)
        {
            /* Cible atteinte, on attend */
            m_patrolWaitTimer = (float)GetRandomValue(10, 30) / 10.0f; /* 1.0s à 3.0s */
            
            /* Choisir une nouvelle cible aléatoire dans la zone */
            float angle = (float)GetRandomValue(0, 360) * DEG2RAD;
            float r = (float)GetRandomValue(0, (int)m_patrolRadius);
            m_patrolTarget.x = m_startPosition.x + std::cos(angle) * r;
            m_patrolTarget.y = m_startPosition.y + std::sin(angle) * r;
        }
        else
        {
            /* Déplacement vers la cible */
            Vector2 dir = Vector2Normalize(Vector2Subtract(m_patrolTarget, m_position));
            Vector2 oldPos = m_position;
            m_position = Vector2Add(m_position, Vector2Scale(dir, m_speed * deltaTime));

            if (tileMap.CheckCollision(GetCollisionRect()))
            {
                m_position = oldPos;
                /* On change immédiatement de cible s'il y a un obstacle */
                m_patrolWaitTimer = 0.5f;
                m_patrolTarget = m_startPosition;
            }
        }
    }
    else if (m_movementType == MovementType::DefinedPath)
    {
        if (m_waypoints.empty())
        {
            return;
        }

        Vector2 target = m_waypoints[m_currentWaypointIndex];
        float dist = Vector2Distance(m_position, target);

        if (dist < 5.0f)
        {
            /* Point atteint, passer au suivant */
            if (m_waypoints.size() > 1)
            {
                if (m_pathForward)
                {
                    m_currentWaypointIndex++;
                    if (m_currentWaypointIndex >= m_waypoints.size())
                    {
                        m_currentWaypointIndex = m_waypoints.size() - 2;
                        m_pathForward = false;
                    }
                }
                else
                {
                    if (m_currentWaypointIndex == 0)
                    {
                        m_currentWaypointIndex = 1;
                        m_pathForward = true;
                    }
                    else
                    {
                        m_currentWaypointIndex--;
                    }
                }
            }
        }
        else
        {
            Vector2 dir = Vector2Normalize(Vector2Subtract(target, m_position));
            Vector2 oldPos = m_position;
            m_position = Vector2Add(m_position, Vector2Scale(dir, m_speed * deltaTime));

            if (tileMap.CheckCollision(GetCollisionRect()))
            {
                m_position = oldPos;
            }
        }
    }
}

void Npc::Draw() const
{
    /* Dessin procédural du PNJ */
    Vector2 headPos = { m_position.x, m_position.y - 12.0f };
    Vector2 bodyPos = m_position;

    Color robeColor = BLUE;
    if (m_type == NpcType::QuestGiver)
    {
        robeColor = ORANGE;
    }
    else if (m_type == NpcType::Merchant)
    {
        robeColor = PURPLE;
    }

    /* Dessin du corps (Robe en triangle/trapèze ou rectangle arrondi) */
    DrawRectangle(m_position.x - 12.0f, m_position.y - 4.0f, 24.0f, 20.0f, robeColor);
    DrawRectangleLines(m_position.x - 12.0f, m_position.y - 4.0f, 24.0f, 20.0f, BLACK);

    /* Dessin de la tête */
    DrawCircleV(headPos, 8.0f, BEIGE);
    DrawCircleLinesV(headPos, 8.0f, BLACK);

    /* Dessin des yeux */
    DrawCircle(headPos.x - 3, headPos.y - 1, 1.5f, BLACK);
    DrawCircle(headPos.x + 3, headPos.y - 1, 1.5f, BLACK);

    /* Sac à dos pour le marchand */
    if (m_type == NpcType::Merchant)
    {
        DrawRectangle(m_position.x - 18.0f, m_position.y - 2.0f, 6.0f, 14.0f, DARKBROWN);
        DrawRectangleLines(m_position.x - 18.0f, m_position.y - 2.0f, 6.0f, 14.0f, BLACK);
    }

    /* Icône flottante au-dessus de la tête */
    float pulseY = std::sin(GetTime() * 5.0f) * 3.0f;
    Vector2 iconPos = { m_position.x, m_position.y - 30.0f + pulseY };

    if (m_type == NpcType::QuestGiver && m_quest)
    {
        if (m_quest->state == QuestState::NotStarted)
        {
            DrawText("!", (int)iconPos.x - 3, (int)iconPos.y - 5, 16, YELLOW);
        }
        else if (m_quest->state == QuestState::InProgress)
        {
            DrawText("?", (int)iconPos.x - 3, (int)iconPos.y - 5, 16, GRAY);
        }
        else if (m_quest->state == QuestState::ReadyToComplete)
        {
            DrawText("!", (int)iconPos.x - 3, (int)iconPos.y - 5, 18, GREEN);
        }
    }
    else if (m_type == NpcType::Merchant)
    {
        DrawCircleV(iconPos, 5.0f, GOLD);
        DrawCircleLinesV(iconPos, 5.0f, ORANGE);
        DrawText("$", (int)iconPos.x - 3, (int)iconPos.y - 5, 10, BLACK);
    }

    /* Nom du PNJ au-dessus de sa tête */
    int nameWidth = MeasureText(m_name.c_str(), 10);
    DrawText(m_name.c_str(), (int)m_position.x - nameWidth / 2, (int)m_position.y - 24, 10, WHITE);
}

bool Npc::IsPlayerNear(Vector2 playerPos) const
{
    return Vector2Distance(m_position, playerPos) < 60.0f;
}

Rectangle Npc::GetCollisionRect() const
{
    return { m_position.x - m_width / 2.0f, m_position.y - m_height / 2.0f, m_width, m_height };
}

void Npc::Interact(Player& player)
{
    if (!m_isInteracting)
    {
        m_isInteracting = true;
        m_dialogueIndex = 0;
        m_shopActive = false;

        /* Mise à jour immédiate si prêt à compléter une quête */
        if (m_type == NpcType::QuestGiver && m_quest && m_quest->state == QuestState::InProgress)
        {
            if (m_quest->currentKillCount >= m_quest->requiredKillCount)
            {
                m_quest->state = QuestState::ReadyToComplete;
            }
        }
    }
    else
    {
        AdvanceDialogue(player);
    }
}

std::string Npc::GetCurrentDialogueText() const
{
    if (m_type == NpcType::QuestGiver && m_quest)
    {
        if (m_quest->state == QuestState::NotStarted)
        {
            if (m_dialogueIndex < m_defaultDialogues.size())
            {
                return m_defaultDialogues[m_dialogueIndex];
            }
        }
        else if (m_quest->state == QuestState::InProgress)
        {
            return "Alors, vous avancez ? Detruisez " + std::to_string(m_quest->requiredKillCount) + 
                   " caisses ! (" + std::to_string(m_quest->currentKillCount) + "/" + 
                   std::to_string(m_quest->requiredKillCount) + ")";
        }
        else if (m_quest->state == QuestState::ReadyToComplete)
        {
            return "Incroyable ! Vous avez detruit toutes les caisses ! Merci infiniment. Voici votre recompense !";
        }
        else if (m_quest->state == QuestState::Completed)
        {
            return "Merci encore pour votre aide ! Grace a vous, mon jardin est resplendissant.";
        }
    }

    if (m_dialogueIndex < m_defaultDialogues.size())
    {
        return m_defaultDialogues[m_dialogueIndex];
    }

    return "";
}

void Npc::AdvanceDialogue(Player& player)
{
    if (m_shopActive)
    {
        /* Si le magasin est actif, les dialogues ne défilent plus normalement.
         * C'est géré par l'interface d'achat du magasin. */
        return;
    }

    m_dialogueIndex++;

    /* Vérifier si on a fini le dialogue */
    bool dialogueFinished = false;
    if (m_type == NpcType::QuestGiver && m_quest)
    {
        if (m_quest->state == QuestState::NotStarted)
        {
            if (m_dialogueIndex >= m_defaultDialogues.size())
            {
                m_quest->state = QuestState::InProgress;
                dialogueFinished = true;
            }
        }
        else if (m_quest->state == QuestState::InProgress)
        {
            dialogueFinished = true;
        }
        else if (m_quest->state == QuestState::ReadyToComplete)
        {
            /* Donner les récompenses */
            player.AddRupees(m_quest->rewardRupees);
            player.GetInventory().m_upgradePoints += m_quest->rewardPoints;
            m_quest->state = QuestState::Completed;
            dialogueFinished = true;
        }
        else if (m_quest->state == QuestState::Completed)
        {
            dialogueFinished = true;
        }
    }
    else
    {
        if (m_dialogueIndex >= m_defaultDialogues.size())
        {
            dialogueFinished = true;
        }
    }

    if (dialogueFinished)
    {
        if (m_type == NpcType::Merchant && !m_merchantItems.empty())
        {
            /* Si c'est un marchand, on active la boutique après le dialogue d'accueil ! */
            m_shopActive = true;
        }
        else
        {
            CloseDialogue();
        }
    }
}

void Npc::SetStatic()
{
    m_movementType = MovementType::Static;
}

void Npc::SetPatrolZone(float radius, float speed)
{
    m_movementType = MovementType::PatrolZone;
    m_patrolRadius = radius;
    m_speed = speed;
    m_patrolTarget = m_position;
}

void Npc::SetDefinedPath(const std::vector<Vector2>& waypoints, float speed)
{
    m_movementType = MovementType::DefinedPath;
    m_waypoints = waypoints;
    m_speed = speed;
    m_currentWaypointIndex = 0;
    m_pathForward = true;
}

void Npc::ConfigureQuest(const std::string& questId, const std::string& description, int requiredKills, int rewardRupees, int rewardPoints)
{
    m_quest = std::make_unique<Quest>();
    m_quest->id = questId;
    m_quest->description = description;
    m_quest->state = QuestState::NotStarted;
    m_quest->requiredKillCount = requiredKills;
    m_quest->currentKillCount = 0;
    m_quest->rewardRupees = rewardRupees;
    m_quest->rewardPoints = rewardPoints;

    /* Abonnement au bus d'événements pour découpler les quêtes du CombatSystem */
    if (questId == "crate_hunt")
    {
        EventSystem::SubscribeToCrateDestroyed([this](const CrateDestroyedEvent& e) {
            (void)e; /* On ignore la position de la caisse pour l'instant */
            if (this->m_quest && this->m_quest->state == QuestState::InProgress)
            {
                this->m_quest->currentKillCount++;
            }
        });
    }
}

void Npc::AddMerchantItem(const std::string& itemId, const std::string& name, int price, const std::string& description)
{
    MerchantItem item;
    item.itemId = itemId;
    item.name = name;
    item.price = price;
    item.description = description;
    m_merchantItems.push_back(item);
}
