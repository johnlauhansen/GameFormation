#include "spatial_grid.h"
#include "destructible.h"

SpatialGrid::SpatialGrid(int cellSize)
    : m_cellSize(cellSize)
{
}

void SpatialGrid::Clear()
{
    m_cells.clear();
}

std::pair<int, int> SpatialGrid::GetCell(float x, float y) const
{
    return { (int)(x / m_cellSize), (int)(y / m_cellSize) };
}

void SpatialGrid::Insert(Destructible* dest)
{
    if (dest == nullptr || !dest->IsAlive()) return;

    Rectangle rect = dest->GetCollisionRect();
    
    // Trouver les limites min/max des cellules que cet objet touche
    auto minCell = GetCell(rect.x, rect.y);
    auto maxCell = GetCell(rect.x + rect.width, rect.y + rect.height);

    for (int cx = minCell.first; cx <= maxCell.first; ++cx)
    {
        for (int cy = minCell.second; cy <= maxCell.second; ++cy)
        {
            m_cells[{cx, cy}].push_back(dest);
        }
    }
}

std::vector<Destructible*> SpatialGrid::GetNearby(const Rectangle& rect) const
{
    std::vector<Destructible*> results;
    
    // Afin d'éviter de retourner des doublons si un objet chevauche deux cellules qu'on inspecte,
    // on pourrait utiliser un std::unordered_set, mais sur de petits volumes, un vecteur
    // avec vérification linéaire suffit et reste cache-friendly.
    
    auto minCell = GetCell(rect.x, rect.y);
    auto maxCell = GetCell(rect.x + rect.width, rect.y + rect.height);

    for (int cx = minCell.first; cx <= maxCell.first; ++cx)
    {
        for (int cy = minCell.second; cy <= maxCell.second; ++cy)
        {
            auto it = m_cells.find({cx, cy});
            if (it != m_cells.end())
            {
                for (Destructible* dest : it->second)
                {
                    // Ajout unique (vérification simple de doublons)
                    bool exists = false;
                    for (Destructible* existing : results)
                    {
                        if (existing == dest) { exists = true; break; }
                    }
                    if (!exists)
                    {
                        results.push_back(dest);
                    }
                }
            }
        }
    }
    return results;
}
