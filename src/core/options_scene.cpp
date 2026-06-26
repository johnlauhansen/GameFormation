#include "options_scene.h"
#include "scene_manager.h"

void OptionsScene::Update(float deltaTime, SceneManager& manager)
{
    (void)deltaTime;

    /* 1. Ajustement du volume (Gabarit fictif interactif) */
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_Q))
    {
        m_soundVolumePercent -= 10;
        if (m_soundVolumePercent < 0) m_soundVolumePercent = 0;
    }
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D))
    {
        m_soundVolumePercent += 10;
        if (m_soundVolumePercent > 100) m_soundVolumePercent = 100;
    }

    /* 2. Afficher/Cacher les contrôles */
    if (IsKeyPressed(KEY_C))
    {
        m_controlsShown = !m_controlsShown;
    }

    /* 3. Retour au Menu Principal (Echap ou Enter) */
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER))
    {
        manager.ChangeScene("Title");
    }
}

void OptionsScene::Draw() const
{
    ClearBackground({ 20, 20, 30, 255 });

    DrawText("PARAMETRES & CONTROLES", 800 / 2 - MeasureText("PARAMETRES & CONTROLES", 24) / 2, 80, 24, LIGHTGRAY);

    /* Volume */
    DrawText("Volume Principal :", 200, 200, 18, RAYWHITE);
    DrawRectangle(400, 200, 200, 20, DARKGRAY);
    DrawRectangle(400, 200, m_soundVolumePercent * 2, 20, GREEN);
    DrawRectangleLines(400, 200, 200, 20, WHITE);
    DrawText(TextFormat("%d %%", m_soundVolumePercent), 620, 200, 18, RAYWHITE);
    DrawText("(Utilisez Gauche/Droite pour ajuster)", 200, 230, 12, GRAY);

    /* Touche d'aide */
    DrawText("Appuyez sur 'C' pour afficher/cacher le tutoriel des touches", 200, 300, 14, ORANGE);

    if (m_controlsShown)
    {
        DrawRectangle(180, 330, 440, 160, Fade(BLACK, 0.8f));
        DrawRectangleLines(180, 330, 440, 160, ORANGE);

        DrawText("Z, Q, S, D ou Fleches : Deplacer le heros", 200, 350, 14, WHITE);
        DrawText("ESPACE ou ENTREE : Attaquer a l'Epee", 200, 380, 14, WHITE);
        DrawText("B : Lancer le Boomerang", 200, 410, 14, WHITE);
        DrawText("E : Parler a un PNJ / Lire un Panneau", 200, 440, 14, WHITE);
        DrawText("I : Ouvrir la Forge (Inventaire)", 200, 470, 14, WHITE);
    }

    /* Indication de retour */
    DrawText("Appuyez sur ECHAP pour revenir", 800 / 2 - MeasureText("Appuyez sur ECHAP pour revenir", 16) / 2, 530, 16, GRAY);
}
