#include <raylib.h>
#include <vector>
#include "Constants.h"
#include <iostream>
#include <map>

class ComponentsManager;
class EntityManager;

//-----------------------------------------------
// World data
//-----------------------------------------------

enum ETerrainType
{
    Grass,
    Swamp,
    Rock,
    Water,
    Trees,
    ETerrainTypeCount
};

enum EDiscovetyState
{
    Undiscovered,
    Planned,
    Discovered
};

struct Vector2Int
{
    int x, y;
};

struct TreesTile
{
    int x, y, amount;
    int reservations = 0;
    Vector2Int* treePositions;

    TreesTile(int tileX, int tileY, int count, int range)
    {
        x = tileX;
        y = tileY;
        amount = count;

        // Setup tree positions
        treePositions = new Vector2Int[count];

        for (size_t i = 0; i < count; i++)
        {
            treePositions[i].x = GetRandomValue(-range, range);
            treePositions[i].y = GetRandomValue(-range, range);
        }
    }

    bool FellTree()
    {
        if (amount <= 0)
            return false;

        amount--;
        return true;
    }

    bool operator==(const TreesTile& other) const {
        return this == &other;
    }
};

//-----------------------------------------------
// World class definition
//-----------------------------------------------
class World
{
private:
    Texture2D treeTileTextures[5];
    EDiscovetyState** discovered; // Blocks discovered by NCPs, Undiscovered will be covered in fog
    std::vector<TreesTile> treeTiles;

public:
    int worldSize, width, height;

    // Map data
    int undiscoveredCount;
    ETerrainType** mapTerrain;

    // Colors
    const Color cGrass = { 70, 100, 52, 255 };
    const Color cSwamp = { 37, 53, 18, 255 };
    const Color cRock = { 82, 87, 89, 255 };
    const Color cWater = { 56, 134, 219, 255 };

    EntityManager* entityManager;

    World(const char* path);
    ~World();

    void Init();
    bool LoadMap(const char* path);
    void Update(float dTime);
    void Draw();
    //bool IsDiscovered(int x, int y);

    void Discover(int x, int y, EDiscovetyState state);
    EDiscovetyState inline TileDiscoveryState(int x, int y) { return discovered[x][y]; }

    TreesTile* ClosestTreeTile(Vector2Int currentTile);
    void RemoveTreeTile(TreesTile* tile);

    // Coordination
    static inline Vector2Int PositionToTile(Vector2 position)
    {
        int x = position.x / GlobalVars::TILE_SIZE;
        int y = position.y / GlobalVars::TILE_SIZE;
        return { x,y };
    }

    static inline Vector2 TileToCenterPosition(Vector2Int tile)
    {
        float x = tile.x * GlobalVars::TILE_SIZE + GlobalVars::TILE_HALF_SIZE;
        float y = tile.y * GlobalVars::TILE_SIZE + GlobalVars::TILE_HALF_SIZE;

        return { x,y };
    }
};