#pragma once
#include <vector>
#include <unordered_map>
#include <raylib.h>

class Destructible;

/*
 * Hash simple pour transformer une coordonnée de cellule (X, Y) en entier 64-bit
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
 */
class SpatialGrid
{
public:
    SpatialGrid(int cellSize = 160);

    void Clear();
    void Insert(Destructible* dest);
    
    /* Retourne la liste des destructibles proches de ce rectangle */
    std::vector<Destructible*> GetNearby(const Rectangle& rect) const;

private:
    int m_cellSize;
    std::unordered_map<std::pair<int, int>, std::vector<Destructible*>, CellHash> m_cells;

    /* Calcule la cellule spatiale à partir d'une coordonnée du monde */
    std::pair<int, int> GetCell(float x, float y) const;
};
