#include "dialogue_system.h"
#include "npc.h"
#include "player.h"
#include "hud.h"

DialogueSystem::DialogueSystem()
    : m_activeNpc(nullptr)
{
}

void DialogueSystem::StartInteraction(Npc& npc, Player& player)
{
    m_activeNpc = &npc;
    m_activeNpc->Interact(player);
}

void DialogueSystem::Reset()
{
    if (m_activeNpc != nullptr)
    {
        m_activeNpc->CloseDialogue();
    }
    m_activeNpc = nullptr;
}

void DialogueSystem::Update(float deltaTime, Player& player, HUD& hud)
{
    (void)deltaTime;

    if (m_activeNpc == nullptr)
    {
        return;
    }

    /* Si l'interaction a été fermée à l'intérieur du PNJ */
    if (!m_activeNpc->IsInDialogue())
    {
        m_activeNpc = nullptr;
        return;
    }

    /* Gérer la boutique */
    if (m_activeNpc->IsShopActive())
    {
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
        {
            int count = (int)m_activeNpc->GetMerchantItems().size();
            if (count > 0)
            {
                int idx = m_activeNpc->GetSelectedShopIndex();
                m_activeNpc->SetSelectedShopIndex((idx - 1 + count) % count);
            }
        }
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
        {
            int count = (int)m_activeNpc->GetMerchantItems().size();
            if (count > 0)
            {
                int idx = m_activeNpc->GetSelectedShopIndex();
                m_activeNpc->SetSelectedShopIndex((idx + 1) % count);
            }
        }
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_E))
        {
            const auto& items = m_activeNpc->GetMerchantItems();
            int idx = m_activeNpc->GetSelectedShopIndex();
            if (idx >= 0 && idx < (int)items.size())
            {
                const auto& item = items[idx];
                if (player.GetRupees() >= item.price)
                {
                    player.AddRupees(-item.price);
                    if (item.itemId == "heal_potion")
                    {
                        player.SetHealth(player.GetHealth() + 30.0f);
                        hud.TriggerNotification("Achete : " + item.name + " (Vie Restauree) !", 2.0f);
                    }
                    else if (item.itemId == "forge_point")
                    {
                        player.GetInventory().m_upgradePoints += 1;
                        hud.TriggerNotification("Achete : " + item.name + " (+1 Pt de Forge) !", 2.0f);
                    }
                    else if (item.itemId == "boomerang")
                    {
                        player.GetInventory().AddItem(item.itemId);
                        hud.TriggerNotification("Achete : " + item.name + " (Obtenu) !", 2.0f);
                    }
                }
                else
                {
                    hud.TriggerNotification("Pas assez de Rubis !", 1.5f);
                }
            }
        }
        if (IsKeyPressed(KEY_ESCAPE))
        {
            Reset();
        }
    }
    else
    {
        /* Dialogue classique : faire défiler */
        if (IsKeyPressed(KEY_E) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))
        {
            m_activeNpc->Interact(player);
            
            /* Si l'interaction s'est arrêtée après défilement */
            if (!m_activeNpc->IsInDialogue())
            {
                m_activeNpc = nullptr;
            }
        }
    }
}

void DialogueSystem::Draw(const Player& player) const
{
    if (m_activeNpc == nullptr)
    {
        return;
    }

    /* 1. Boîte de dialogue standard */
    int boxX = 50;
    int boxY = 440;
    int boxW = 700;
    int boxH = 120;
    
    DrawRectangle(boxX, boxY, boxW, boxH, Fade(BLACK, 0.9f));
    DrawRectangleLines(boxX, boxY, boxW, boxH, BLUE);
    
    /* Nom du PNJ */
    DrawText(m_activeNpc->GetName().c_str(), boxX + 20, boxY + 15, 18, GOLD);
    
    /* Texte de dialogue */
    std::string currentText = m_activeNpc->GetCurrentDialogueText();
    DrawText(currentText.c_str(), boxX + 20, boxY + 45, 16, WHITE);

    if (m_activeNpc->IsShopActive())
    {
        DrawText("[ENTREE/ESPACE] Acheter  [ECHAP] Quitter", boxX + 400, boxY + 15, 12, GRAY);
        
        /* Dessiner la boutique */
        const auto& items = m_activeNpc->GetMerchantItems();
        int shopSel = m_activeNpc->GetSelectedShopIndex();
        
        int shopX = 50;
        int shopY = 160;
        int shopW = 700;
        int shopH = 260;
        
        DrawRectangle(shopX, shopY, shopW, shopH, Fade(BLACK, 0.95f));
        DrawRectangleLines(shopX, shopY, shopW, shopH, GOLD);
        DrawText("BOUTIQUE DU MARCHAND", shopX + 20, shopY + 15, 20, GOLD);
        DrawText(TextFormat("Vos Rubis : %d", player.GetRupees()), shopX + 500, shopY + 15, 16, GREEN);
        
        for (size_t i = 0; i < items.size(); ++i)
        {
            const auto& item = items[i];
            int itemY = shopY + 60 + (int)i * 50;
            bool isSelected = ((int)i == shopSel);
            
            Color itemColor = isSelected ? YELLOW : WHITE;
            if (isSelected)
            {
                DrawRectangle(shopX + 15, itemY - 5, shopW - 30, 40, Fade(GRAY, 0.2f));
                DrawRectangleLines(shopX + 15, itemY - 5, shopW - 30, 40, YELLOW);
                DrawTriangle({ (float)shopX + 25, (float)itemY + 5 }, { (float)shopX + 25, (float)itemY + 20 }, { (float)shopX + 37, (float)itemY + 12.5f }, YELLOW);
            }
            
            DrawText(item.name.c_str(), shopX + 50, itemY + 5, 16, itemColor);
            DrawText(item.description.c_str(), shopX + 220, itemY + 7, 12, GRAY);
            
            std::string priceText = std::to_string(item.price) + " Rubis";
            Color priceColor = (player.GetRupees() >= item.price) ? GREEN : RED;
            DrawText(priceText.c_str(), shopX + 580, itemY + 5, 16, priceColor);
        }
    }
    else
    {
        DrawText("Appuyez sur [E] ou [ENTREE] pour continuer...", boxX + 380, boxY + 95, 12, GRAY);
    }
}
