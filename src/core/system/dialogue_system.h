#pragma once
#include <raylib.h>

class Npc;
class Player;
class HUD;

class DialogueSystem
{
public:
    DialogueSystem();

    void StartInteraction(Npc& npc, Player& player);
    void Update(float deltaTime, Player& player, HUD& hud);
    void Draw(const Player& player) const;

    [[nodiscard]] bool IsActive() const
    {
        return m_activeNpc != nullptr;
    }

    void Reset();

private:
    Npc* m_activeNpc;
};
