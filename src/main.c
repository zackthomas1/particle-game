/*
Raylib example file.
This is an example main file for a simple raylib project.
Use this as a starting point or replace it with your code.

by Jeffery Myers is marked with CC0 1.0. To view a copy of this license, visit https://creativecommons.org/publicdomain/zero/1.0/

*/

#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"

#include "resource_dir.h"	// utility header for SearchAndSetResourceDir

#define MAX_BUILDINGS 100

// ------------------------
// Program main entry point
// ------------------------
int main ()
{
    // Initialization
    // ------------------------   
    // Tell the window to use vsync and work on high DPI displays
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

    // Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
    SearchAndSetResourceDir("resources");

    // Create the window and OpenGL context
    const int screenWidth = 800; 
    const int screenHeight = 450;
    InitWindow(screenWidth, screenHeight, "Hello Raylib");

    int currentFPS = 60;
    SetTargetFPS(currentFPS); 

    Camera2D camera = { 0 }; 
    camera.zoom = 1.0f;

    int zoomMode = 0;   // 0 - Mouse Wheel, 1 - Mouse Move

    // Main game loop
    while (!WindowShouldClose())        // run the loop until the user presses ESCAPE or presses the Close button on the window
    {
        // Update
        // -----------------------
        if (IsKeyPressed(KEY_ONE)) {
            zoomMode = 0; 
        }
        else if (IsKeyPressed(KEY_TWO)) {
            zoomMode = 1;
        }

        // Translate based on mouse left click
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 delta = GetMouseDelta(); 
            delta = Vector2Scale(delta, -1.0f / camera.zoom); 
            camera.target = Vector2Add(camera.target, delta);
        }

        if (zoomMode == 0) {
            // Zoom based on mouse wheel
            float wheel = GetMouseWheelMove(); 
            if (wheel != 0) {

                // Get world point that is under mouse
                Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);

                // Set offset to where mouse is
                camera.offset = GetMousePosition(); 

                // Set targeet to match, so that camera maps world space point
                // under cursor to screen space point under cursor at any zoom
                camera.target = mouseWorldPos; 

                // zoom increment
                // Use log scalling
                float scale = 0.2f * wheel; 
                camera.zoom = Clamp(expf(logf(camera.zoom) + scale), 0.125f, 64.0f);
            }
        }
        else {
            // Zoom based on mouse right click
            if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
            {
                // Get the world point that is under the mouse
                Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);

                // Set the offset to where the mouse is
                camera.offset = GetMousePosition();

                // Set the target to match, so that the camera maps the world space point
                // under the cursor to the screen space point under the cursor at any zoom
                camera.target = mouseWorldPos;
            }

            if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
            {
                // Zoom increment
                // Uses log scaling to provide consistent zoom speed
                float deltaX = GetMouseDelta().x;
                float scale = 0.005f * deltaX;
                camera.zoom = Clamp(expf(logf(camera.zoom) + scale), 0.125f, 64.0f);
            }
        }

        // Drawing
        // ------------------------
        BeginDrawing();
        {
            // Setup the back buffer for drawing (clear color and depth buffers)
            ClearBackground(RAYWHITE);

            BeginMode2D(camera);
            {
                // draw the 3D grid, rotated 90 degrees and centered around (0,0)
                rlPushMatrix();
                {
                    rlTranslatef(0, 25*50, 0);
                    rlRotatef(90, 1, 0, 0);
                    DrawGrid(100, 50);
                }
                rlPopMatrix();

                // Draw referencee circle
                DrawCircle(GetScreenWidth() / 2, GetScreenHeight() / 2, 50, MAROON);
            }
            EndMode2D();

            // Draw mouse reference
            // Vector2 mousePos = GetWorldToScreen2D(GetMousePosition(), camera)
            DrawCircleV(GetMousePosition(), 4, DARKGRAY);
            DrawTextEx(GetFontDefault(), TextFormat("[%i, %i]", GetMouseX(), GetMouseY()),
                Vector2Add(GetMousePosition(), (Vector2) { -44, -24 }), 20, 2, BLACK);

            DrawText("[1][2] Select mouse zoom mode (Wheel or Move)", 20, 20, 20, DARKGRAY);
            if (zoomMode == 0) {
                DrawText("Mouse left button drag to move, mouse wheel to zoom", 20, 50, 20, DARKGRAY);
            } else {
                DrawText("Mouse left button drag to move, mouse press and move to zoom", 20, 50, 20, DARKGRAY);
            }
        }
        // end the frame and get ready for the next one  (display frame, poll input, etc...)
        EndDrawing();
    }
    // De-Initialization
    // ------------------------
    // destroy the window and cleanup the OpenGL context
    CloseWindow();
    return 0;
}
