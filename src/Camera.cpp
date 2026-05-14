#include "Camera.h"

CustomCamera::CustomCamera()
{
    camera.target = { 0,0 };
    camera.offset = { 0,0 };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
}

void CustomCamera::Update(float dt)
{
    float speed = moveSpeed * camera.zoom;

    // Move
    if (IsKeyDown(KEY_A))
        camera.target.x -= speed;
    else if (IsKeyDown(KEY_D))
        camera.target.x += speed;

    if (IsKeyDown(KEY_W))
        camera.target.y -= speed;
    else if (IsKeyDown(KEY_S))
        camera.target.y += speed;

    // Zoom
    if (IsKeyDown(KEY_Q))
        camera.zoom -= zoomSpeed;
    else if (IsKeyDown(KEY_E))
        camera.zoom += zoomSpeed;
   
    // Update
    BeginMode2D(camera);
}