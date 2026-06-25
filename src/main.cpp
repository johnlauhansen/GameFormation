#include "core/game.h"
#include <raylib.h>

int main(void)
{
    /* Initialisation de la fenêtre globale de l'application */
    const int screenWidth = 800;
    const int screenHeight = 600;
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
