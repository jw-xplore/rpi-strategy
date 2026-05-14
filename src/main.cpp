#include <raylib.h>
#include <string>
#include "Constants.h"
#include "World.h"
#include "Database.h"
#include "PathFinding.h"
#include "EntityManager.h"
#include "Worker.h"
#include <map>
#include <iostream>
#include "raymath.h"
#include "SystemsHolder.h"
#include "Commander.h"
#include "Camera.h"

extern float TIME_SCALE = 1;
bool pause = false;

CustomCamera cam;

/*
Debug
*/
void DrawPath(std::vector<Node>* path)
{
    int halfSize = GlobalVars::TILE_SIZE / 2;

    for (int i = 0; i < path->size(); i++)
    {
        DrawCircle((*path)[i].x * GlobalVars::TILE_SIZE, (*path)[i].y * GlobalVars::TILE_SIZE, GlobalVars::TILE_HALF_SIZE, BLUE);
    }
}

void AdjustTimeScale()
{
    if (IsKeyDown(KEY_RIGHT))
        TIME_SCALE += 0.1f;
    else  if (IsKeyDown(KEY_LEFT) && TIME_SCALE > 0.1)
        TIME_SCALE -= 0.1f;

    if (IsKeyPressed(KEY_SPACE))
        pause = !pause;

    // Show time
    std::string strPause = " (Running)";
    if (pause)
        strPause = " (Paused)";

    std::string strTime = "Time scale: " + std::to_string(TIME_SCALE) + strPause;
    char const* cTime = strTime.c_str();
    DrawText(cTime, 50 + cam.camera.target.x, 10 + cam.camera.target.y, 16 / cam.camera.zoom, YELLOW);

    // Show FPS
    std::string strFPS = "FPS: " + std::to_string(1 / GetFrameTime());
    char const* cFPS = strFPS.c_str();
    DrawText(cFPS, 50 + cam.camera.target.x, 30 + cam.camera.target.y, 16 / cam.camera.zoom, YELLOW);

    // Camera zoom
    std::string strZoom = "Zoom: " + std::to_string(cam.camera.zoom) + "x";
    char const* cZoom = strZoom.c_str();
    DrawText(cZoom, 50 + cam.camera.target.x, 50 + cam.camera.target.y, 16 / cam.camera.zoom, YELLOW);
}

// Game functionality
void RunGame()
{
    // Init
    //World world = World("resources/testMap.txt");
    World world = World("resources/WorldMap.txt");
    PathFinding pathfinding = PathFinding(world);
    EntityManager entityManager = EntityManager();
    entityManager.world = &world;
    SystemsHolder::GetInstance()->Init(&world, &entityManager, &pathfinding);
    Commander commander = Commander();
    cam = CustomCamera();

    //std::vector<Node>* path = pathfinding.AStar({ 64, 64 }, { 640, 640 });
    //return;

    std::map<Node*, NodeRecordAs> searchResult;
    std::priority_queue<NodeRecordAs, std::vector<NodeRecordAs>, NodeRecordAsCompare> open;
    //std::vector<Node>* path = pathfinding.AStarDivided({ 1300, 64 }, { 1000, 640 }, searchResult, open);
    //std::vector<Node>* path = {};
    int frameCount = 0;

    /*
    Camera2D camera = { 0 };
    camera.target = { 0,0};
    camera.offset = { 0,0 };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
    */

    // Gameloop
    while (!WindowShouldClose())
    {
        // Update entities
        float ts = TIME_SCALE;
        if (pause)
            ts = 0;

        float dt = GetFrameTime() * ts;
        if (dt > 1.0)
            dt = 1.0;

        world.Update(dt);
        commander.Update(dt);

        // Rendering
        BeginDrawing();
        //BeginMode2D(camera);
        cam.Update(dt);
        ClearBackground(BLACK);
        world.Draw();

        //pathfinding.DrawGraph();
        //if (path)
            //DrawPath(path);

        entityManager.Update(dt);

        AdjustTimeScale();
        //std::cout << "Path calculation FPS: " << 1 / GetFrameTime() << "\n";

        // Debug drawing
        commander.DebugDraw();

        EndDrawing();
        //return;
    }

    // Cleanup
    //delete path;
    delete GameDB::Database::Instance();
}

int main()
{
    // Window setup
    InitWindow(GlobalVars::SCREEN_WIDTH, GlobalVars::SCREEN_HEIGHT, "My first RAYLIB program!");
    SetTargetFPS(60);

    RunGame();

    // End
    CloseWindow();
}