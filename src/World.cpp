#include "World.h"
#include <raymath.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include "Constants.h"
#include "Database.h"

//-----------------------------------------------
// World class definition
//-----------------------------------------------

World::World(const char* path)
{
    // World setup
    LoadMap(path);

    // Discovered parts
    undiscoveredCount = worldSize;
}

World::~World()
{
    // Clear map
    for (size_t x = 0; x < width; x++)
    {
        delete[] mapTerrain[x];
        delete[] discovered[x];
    }

    delete[] mapTerrain;
    delete[] discovered;

    // Clear trees
    for (size_t i = 0; i < treeTiles.size(); i++)
    {
        delete[] treeTiles[i].treePositions;
    }

    treeTiles.clear();
}

void World::Init()
{
    // Texture setup
    for (int i = 0; i < 5; i++)
    {
        std::string imgPath = "resources/trees" + std::to_string(i + 1) + ".png";
        Image img = LoadImage(imgPath.c_str());

        treeTileTextures[i] = LoadTexture(imgPath.c_str());
        UnloadImage(img);
    }
}

void World::Update(float dTime)
{
    /*
    // Discover world
    Vector2 pos;

    for (int i = 0; i < entityManager->workers.size(); i++)
    {
        pos = entityManager->workers[i]->position;
        pos.x = pos.x / GlobalVars::TILE_SIZE;
        pos.y = pos.y / GlobalVars::TILE_SIZE;

        i = (int)pos.y * width + (int)pos.x;

        // Discover
        if (!discovered[i])
            undiscoveredCount--;

        discovered[i] = true;
    }
    */
}

/*
Load map data and create world setup
*/
bool World::LoadMap(const char* path)
{
    // Load file
	std::ifstream file;
	file.open(path);

	// Define array
	int lines = std::count(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(), '\n');
	height = lines + 1;

    const char** map = new const char*[height];

	// Restart line reading
	int currentLine = 0;
	file.clear();
	file.seekg(0);

	// Fail to read?
    if (!file.is_open())
        return false;

    // Read file
    std::string line;

    int treeTilesCount = 0;
    GameDB::Database* db = GameDB::Database::Instance();
    char treeChar = db->terrains[ETerrainType::Trees].charIdentifier;

    // Read all the lines
    while (std::getline(file, line))
    {
        char* cstr = new char[line.size()];
        for (size_t i = 0; i < line.size(); ++i)
        {
            cstr[i] = line[i];

            if (line[i] == treeChar)
                treeTilesCount++;
        }

        map[currentLine] = cstr;

        currentLine++;
        if (this->width == 0)
            this->width = line.size();
    }

    // Define resources
    worldSize = width * height;
    mapTerrain = new ETerrainType*[width];
    discovered = new EDiscovetyState*[width];
    treeTiles.reserve(treeTilesCount);

    for (size_t x = 0; x < height; x++)
    {
        mapTerrain[x] = new ETerrainType[height];
        discovered[x] = new EDiscovetyState[height];

        for (size_t y = 0; y < height; y++)
        {
            discovered[x][y] = EDiscovetyState::Undiscovered;
            //discovered[x][y] = EDiscovetyState::Discovered;
            // NOTE: map[y][x]
            // Temporary map data are stored as Y first, X second, due to file reading getting the height first.
            // This is fixed for actual map data, which can be naturaly accessed as [x][y]

            // Terrain types
            if (map[y][x] == db->terrains[ETerrainType::Grass].charIdentifier)
                mapTerrain[x][y] = ETerrainType::Grass;

            if (map[y][x] == db->terrains[ETerrainType::Swamp].charIdentifier)
                mapTerrain[x][y] = ETerrainType::Swamp;

            if (map[y][x] == db->terrains[ETerrainType::Water].charIdentifier)
                mapTerrain[x][y] = ETerrainType::Water;

            if (map[y][x] == db->terrains[ETerrainType::Rock].charIdentifier)
                mapTerrain[x][y] = ETerrainType::Rock;

            // Trees
            if (map[y][x] == db->terrains[ETerrainType::Trees].charIdentifier)
            {
                mapTerrain[x][y] = ETerrainType::Trees;
                treeTiles.push_back(TreesTile(x, y, 5, GlobalVars::TILE_HALF_SIZE));
            }
        }
    }

    // Discover starting area
    int startTileX = GlobalVars::START_TILE_X;
    int startTileY = GlobalVars::START_TILE_Y;
    int endTileX = GlobalVars::START_TILE_X + 3;
    int	endTileY = GlobalVars::START_TILE_Y + 3;

    for (int x = startTileX; x < endTileX; x++)
    {
        for (int y = startTileY; y < endTileY; y++)
        {
            //discovered[x][y] = EDiscovetyState::Discovered;
        }
    }

    // Clearup
    for (int y = 0; y < height; y++)
    {
        delete[] map[y];
    }

    delete[] map;

    // Stop reading
	file.close();
    return true;
}

void World::Draw()
{
    // Background grass
    int size = 100 * GlobalVars::TILE_SIZE;
    DrawRectangle(0, 0, size, size, cGrass);

    for (size_t x = 0; x < height; x++)
    {
        for (size_t y = 0; y < height; y++)
        {
            // Show fog 
            if (discovered[x][y] != EDiscovetyState::Discovered)
            {
                DrawRectangle(x * GlobalVars::TILE_SIZE, y * GlobalVars::TILE_SIZE, GlobalVars::TILE_SIZE, GlobalVars::TILE_SIZE, GRAY);
                continue;
            }

            // Show terrain
            Color col = cGrass;

            switch (mapTerrain[x][y])
            {
            //case ETerrainType::Grass: col = cGrass; continue;
            //case ETerrainType::Trees: col = cGrass; continue;
            case ETerrainType::Swamp: DrawRectangle(x * GlobalVars::TILE_SIZE, y * GlobalVars::TILE_SIZE, GlobalVars::TILE_SIZE, GlobalVars::TILE_SIZE, cSwamp); break;
            case ETerrainType::Water: DrawRectangle(x * GlobalVars::TILE_SIZE, y * GlobalVars::TILE_SIZE, GlobalVars::TILE_SIZE, GlobalVars::TILE_SIZE, cWater); break;
            case ETerrainType::Rock: DrawRectangle(x * GlobalVars::TILE_SIZE, y * GlobalVars::TILE_SIZE, GlobalVars::TILE_SIZE, GlobalVars::TILE_SIZE, cRock); break;
            }
        }
    }

    // Show trees
    for (size_t i = 0; i < treeTiles.size(); i++)
    {
        float x = treeTiles[i].x * GlobalVars::TILE_SIZE;
        float y = treeTiles[i].y * GlobalVars::TILE_SIZE;

        // Show fog 
        if (discovered[treeTiles[i].x][treeTiles[i].y] != EDiscovetyState::Discovered)
        {
            //DrawRectangle(x, y, GlobalVars::TILE_SIZE, GlobalVars::TILE_SIZE, GRAY);
            continue;
        }

        /*
        if (treeTiles[i].amount - treeTiles[i].reservations <= 0)
            DrawCircle(x, y, 6, PINK);
        */

        DrawTexture(treeTileTextures[treeTiles[i].amount - 1], x, y, BROWN);
    }
}

void DrawTrees()
{

}

void World::Discover(int x, int y, EDiscovetyState state)
{
    discovered[x][y] = state;
}

/// <summary>
/// Find closest tree tile based on selected tile coordinate
/// </summary>
/// <param name="currentTile">Current tile coordinate in int vector</param>
/// <returns></returns>
TreesTile* World::ClosestTreeTile(Vector2Int currentTile)
{
    TreesTile* closestTile = nullptr;
    float closest = -1;

    for (TreesTile& tile : treeTiles)
    {
        // Ignore undiscovered 
        if (TileDiscoveryState(tile.x, tile.y) != EDiscovetyState::Discovered)
            continue;

        // Ignore tile that will be empty
        if (tile.amount - tile.reservations <= 0)
            continue;

        Vector2Int diff = { tile.x - currentTile.x, tile.y - currentTile.y };

        float dist = diff.x * diff.x + diff.y * diff.y;

        if (closest == -1 || closest > dist)
        {
            closestTile = &tile;
            closest = dist;
        }
    }

    return closestTile;
}

/// <summary>
/// Erase selected tile from trees tile list
/// </summary>
/// <param name="tile"></param>
void World::RemoveTreeTile(TreesTile* tile)
{
    treeTiles.erase(std::find(treeTiles.begin(), treeTiles.end(), *tile));
}