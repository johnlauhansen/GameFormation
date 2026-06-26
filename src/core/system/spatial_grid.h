#pragma once
#include <vector>
#include <unordered_map>
#include <raylib.h>

class Destructible;

/*
 * Hash simple pour transformer une coordonnée de cellule (X, Y) en entier 64-bit.
 * Utilisé en clé pour notre std::unordered_map.
 */
struct CellHash
{
    std::size_t operator()(const std::pair<int, int>& cell) const
    {
        return std::hash<int>()(cell.first) ^ (std::hash<int>()(cell.second) << 1);
    }
};

/*
 * Spatial Hash Grid très léger pour l'optimisation des collisions physiques (Broadphase).
 * Découpe la carte en "cellules" (par défaut de la taille de 4 tuiles, ex: 160x160px).
 * Permet d'abaisser la recherche de collisions physiques de O(N) à O(1).
 */
class SpatialGrid
{
public:
    /*
     * Constructeur.
     * @param[in] cellSize Taille de chaque cellule sur la grille en pixels (160px par défaut, soit 4 tuiles).
     */
    SpatialGrid(int cellSize = 160);

    /*
     * Vide entièrement la grille spatiale.
     */
    void Clear();

    /*
     * Insère un pointeur vers un objet destructible dans toutes les cellules qu'il chevauche.
     * @param[in] dest Pointeur vers l'entité vivante à indexer.
     */
    void Insert(Destructible* dest);
    
    /*
     * Recherche et retourne tous les objets destructibles présents dans les cellules touchées par ce rectangle.
     * @param[in] rect Zone de requête (ex: Rectangle d'attaque ou boîte du joueur).
     * @return Liste de pointeurs uniques d'entités proches potentiellement en collision.
     */
    std::vector<Destructible*> GetNearby(const Rectangle& rect) const;

private:
    int m_cellSize;
    std::unordered_map<std::pair<int, int>, std::vector<Destructible*>, CellHash> m_cells;

    /*
     * Calcule l'indice d'une cellule spatiale (colonne, ligne) à partir d'une coordonnée du monde en pixels.
     * @param[in] x Position X mondiale.
     * @param[in] y Position Y mondiale.
     * @return Une paire (colonne, ligne).
     */
    std::pair<int, int> GetCell(float x, float y) const;
};
