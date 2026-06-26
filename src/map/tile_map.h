#pragma once
#include <raylib.h>
#include <vector>
#include "map_data.h"

/*
 * Classe gérant la carte en grille (TileMap) du jeu.
 * Stocke le layout visuel, gère le rendu optimisé des tuiles et résout les collisions physiques AABB.
 */
class TileMap
{
public:
    static constexpr int kTileSize = 40; /* Taille carrée physique d'une tuile, en pixels */

    /*
     * Constructeur par défaut.
     * Génère une carte 20x15 par défaut (herbe avec murs aux bords) si aucune carte externe n'est chargée.
     */
    TileMap();

    /*
     * Charge dynamiquement un nouveau niveau dans le moteur de rendu et de collision.
     * @param[in] level La structure MapLevel contenant les nouvelles grilles.
     */
    void LoadLevel(const MapLevel& level);

    /*
     * Dessine la carte à l'écran.
     * Cette méthode boucle sur les dimensions de la grille et dessine chaque tuile visible.
     */
    void Draw() const;

    /*
     * Vérifie si un rectangle (ex: la boîte de collision du joueur) intersecte un obstacle solide.
     * @param[in] rect Le rectangle à tester.
     * @return true si une collision est détectée, false sinon.
     */
    [[nodiscard]] bool CheckCollision(Rectangle rect) const;

    /*
     * Résout une collision de manière fluide en découpant le mouvement sur les axes X et Y séparément.
     * Permet au joueur de "glisser" le long des murs au lieu de rester bloqué sur les angles.
     * @param[in] currentPos La position cible (nouvelle position calculée par les mouvements).
     * @param[in] oldPos La dernière position connue saine et sûre (avant mouvement).
     * @param[in] width Largeur de la boîte de collision de l'entité.
     * @param[in] height Hauteur de la boîte de collision de l'entité.
     * @return La position finale résolue, saine et ajustée sans collision.
     */
    [[nodiscard]] Vector2 ResolveCollision(Vector2 currentPos, Vector2 oldPos, float width, float height) const;

    [[nodiscard]] int GetWidth() const
    {
        return m_width;
    }

    [[nodiscard]] int GetHeight() const
    {
        return m_height;
    }

    [[nodiscard]] const std::vector<bool>& GetCollisions() const
    {
        return m_collisions;
    }

private:
    /*
     * Convertit des coordonnées (X, Y) d'une grille 2D en index de tableau linéaire 1D.
     * @param[in] x Colonne de la tuile.
     * @param[in] y Ligne de la tuile.
     * @return L'index linéaire 1D correspondant.
     */
    [[nodiscard]] int ToIndex(int x, int y) const
    {
        return (y * m_width) + x;
    }

    int m_width;                     /* Largeur courante de la grille active (en nombre de tuiles) */
    int m_height;                    /* Hauteur courante de la grille active (en nombre de tuiles) */
    std::vector<int> m_tiles;        /* Tableau linéaire des IDs visuels des tuiles */
    std::vector<bool> m_collisions;  /* Tableau linéaire de collisions (true = solide) */
};
