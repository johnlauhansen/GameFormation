#include "core/game.h"
#include <raylib.h>

int main(void)
{
    /* Configuration des drapeaux de fenêtre (redimensionnable) */
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    /* Initialisation de la fenêtre globale de l'application adaptée pour un écran 15 pouces */
    const int screenWidth = 1600;
    const int screenHeight = 1200;
    InitWindow(screenWidth, screenHeight, "gameFormation - Le Jeu Zelda 2D");
    SetTargetFPS(60);

    {
        /* Instanciation de notre contrôleur de jeu et lancement de la boucle principale */
        Game game;
        game.Run();
    }

    CloseWindow();
    return 0;
}
