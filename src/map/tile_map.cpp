#include "tile_map.h"

/*
 * Initialisation d'une grille par défaut (20x15) entourée de murs.
 * Permet au jeu de démarrer sans carte JSON si nécessaire.
 */
TileMap::TileMap()
    : m_width(20)
    , m_height(15)
{
    const int tempMap[15][20] = {
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
        { 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
        { 1, 0, 0, 0, 0, 0, 0, 1, 0, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 1 },
        { 1, 0, 0, 0, 0, 0, 0, 1, 0, 2, 0, 2, 0, 0, 0, 1, 1, 1, 0, 1 },
        { 1, 0, 0, 1, 1, 0, 0, 0, 0, 2, 2, 2, 0, 0, 0, 1, 0, 1, 0, 1 },
        { 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1 },
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
        { 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1 },
        { 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1 },
        { 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 1 },
        { 1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1 },
        { 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }
    };

    m_tiles.resize(m_width * m_height);
    m_collisions.resize(m_width * m_height);

    for (int y = 0; y < m_height; ++y)
    {
        for (int x = 0; x < m_width; ++x)
        {
            const int idx = ToIndex(x, y);
            m_tiles[idx] = tempMap[y][x];
            m_collisions[idx] = (tempMap[y][x] == 1 || tempMap[y][x] == 2);
        }
    }
}

/*
 * Chargement dynamique des données de carte.
 * Copie les tuiles et le masque de collisions de niveau à chaud.
 */
void TileMap::LoadLevel(const MapLevel& level)
{
    m_width = level.width;
    m_height = level.height;
    m_tiles = level.tiles;
    m_collisions = level.collisionMap;
}

/*
 * Boucle de rendu de la carte.
 * Dessine chaque tuile visible à sa coordonnée d'écran ou de monde.
 */
void TileMap::Draw() const
{
    for (int y = 0; y < m_height; ++y)
    {
        for (int x = 0; x < m_width; ++x)
        {
            const int idx = ToIndex(x, y);
            const int tileVal = m_tiles[idx];
            Rectangle tileRect = { (float)x * kTileSize, (float)y * kTileSize, (float)kTileSize, (float)kTileSize };

            if (tileVal == 1)
            {
                /* Obstacle solide : Mur en gris foncé */
                DrawRectangleRec(tileRect, DARKGRAY);
                DrawRectangleLines((int)tileRect.x, (int)tileRect.y, (int)tileRect.width, (int)tileRect.height, BLACK);
            }
            else if (tileVal == 2)
            {
                /* Obstacle destructible/solide : Buisson en vert foncé */
                DrawRectangleRec(tileRect, DARKGREEN);
                DrawRectangleLines((int)tileRect.x, (int)tileRect.y, (int)tileRect.width, (int)tileRect.height, LIME);
            }
            else
            {
                /* Sol normal : Prairie en vert clair texturé simple */
                DrawRectangleRec(tileRect, { 100, 180, 100, 255 });
            }
        }
    }
}

/*
 * Test de collision AABB optimisé.
 * Ne teste que les tuiles directement sous le rectangle d'entrée pour préserver les performances.
 */
bool TileMap::CheckCollision(Rectangle rect) const
{
    int startX = (int)(rect.x / kTileSize);
    int endX = (int)((rect.x + rect.width) / kTileSize);
    int startY = (int)(rect.y / kTileSize);
    int endY = (int)((rect.y + rect.height) / kTileSize);

    /* Sécurité anti-débordement de tableau */
    if (startX < 0)
    {
        startX = 0;
    }
    if (endX >= m_width)
    {
        endX = m_width - 1;
    }
    if (startY < 0)
    {
        startY = 0;
    }
    if (endY >= m_height)
    {
        endY = m_height - 1;
    }

    for (int y = startY; y <= endY; ++y)
    {
        for (int x = startX; x <= endX; ++x)
        {
            const int idx = ToIndex(x, y);
            if (m_collisions[idx])
            {
                Rectangle tileRect = { (float)x * kTileSize, (float)y * kTileSize, (float)kTileSize, (float)kTileSize };
                if (CheckCollisionRecs(rect, tileRect))
                {
                    return true; /* Collision confirmée ! */
                }
            }
        }
    }

    return false;
}

/*
 * Algorithme de glissement le long des obstacles.
 * Découpe les essais de déplacement en sous-scopes indépendants sur X puis sur Y.
 */
Vector2 TileMap::ResolveCollision(Vector2 currentPos, Vector2 oldPos, float width, float height) const
{
    Vector2 nextPos = { currentPos.x, oldPos.y };
    Rectangle checkRect = { nextPos.x - (width / 2.0f), nextPos.y - (height / 2.0f), width, height };

    /* 1. Premier essai sur l'axe X */
    if (CheckCollision(checkRect))
    {
        nextPos.x = oldPos.x; /* Bloque le mouvement horizontal s'il y a collision */
    }

    /* 2. Second essai sur l'axe Y */
    nextPos.y = currentPos.y;
    checkRect = { nextPos.x - (width / 2.0f), nextPos.y - (height / 2.0f), width, height };
    if (CheckCollision(checkRect))
    {
        nextPos.y = oldPos.y; /* Bloque le mouvement vertical s'il y a collision */
    }

    return nextPos;
}
