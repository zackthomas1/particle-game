/*
Raylib example file.
This is an example main file for a simple raylib project.
Use this as a starting point or replace it with your code.

by Jeffery Myers is marked with CC0 1.0. To view a copy of this license, visit https://creativecommons.org/publicdomain/zero/1.0/

*/

#include "raylib.h"
#include "resource_dir.h"	// utility header for SearchAndSetResourceDir


// ------------------------
// Program main entry point
// ------------------------
int main ()
{
    // Initialization
    // ------------------------   
    // Tell the window to use vsync and work on high DPI displays
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

    // Create the window and OpenGL context
    const int screenWidth = 800; 
    const int screenHeight = 450;
    InitWindow(screenWidth, screenHeight, "Hello Raylib");

    int currentFPS = 60;
    SetTargetFPS(currentFPS); 

    // Sotre the position for both circles
    Vector2 deltaCircle = { 0, (float)screenHeight / 3.0f };
    Vector2 frameCircle = { 0, (float)screenHeight * (2.0f / 3.0f) };

    const float speed = 10.0f, circleRadius = 32.0f;

    // Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
    SearchAndSetResourceDir("resources");

    // Load a texture from the resources directory
    Texture wabbit = LoadTexture("wabbit_alpha.png");
    
    // Main game loop
    while (!WindowShouldClose())        // run the loop until the user presses ESCAPE or presses the Close button on the window
    {
        // Update
        // -----------------------
        // Adjust FPS target based on mouse wheel
        float mouseWheel = GetMouseWheelMove();
        if (mouseWheel != 0) {
            currentFPS += (int)mouseWheel;
            if (currentFPS < 0) {
                currentFPS = 0;
            }
            SetTargetFPS(currentFPS);
        }

        // GetFrameTime() returns the time it took to draw last frame, in seconds(delta time)
        // Use delta time to make cicle look like it's moving at a consistent speed reargless of actual FPS

        // Multiple by arbitrary value to make speed
        // visually closer to other cirlce (at 60 fps), for comparison 
        const float deltaMultiplier = 6.0f;
        deltaCircle.x += GetFrameTime() * deltaMultiplier * speed;

        // This circle can move faster or slower depending on FPS
        frameCircle.x += 0.1f * speed;

        if (deltaCircle.x > screenWidth) {
            deltaCircle.x = 0;
        }

        if (frameCircle.x > screenWidth) {
            frameCircle.x = 0;
        }

        // Reset both circles positions
        if (IsKeyPressed(KEY_R)) {
            deltaCircle.x = 0; 
            frameCircle.x = 0;
        }

        // -----------------------

        // Drawing
        // ------------------------
        BeginDrawing();

        // Setup the back buffer for drawing (clear color and depth buffers)
        ClearBackground(RAYWHITE);

        // Draw both circles to screen
        DrawCircleV(deltaCircle, circleRadius, RED);
        DrawCircleV(frameCircle, circleRadius, BLUE);

        // draw help text
        const char* fpsText = 0; 
        if (currentFPS <= 0) {
            fpsText = TextFormat("FPS: unlimited (%i)", GetFPS());
        }
        else {
            TextFormat("FPS: %i (target: %i)", GetFPS(), currentFPS);
        }

        DrawText(fpsText, 10, 10, 20, DARKGRAY);
        DrawText(TextFormat("Frame time: %02.02f ms", GetFrameTime()), 10, 30, 20, DARKGRAY);
        DrawText("Use the scroll wheel to change fps limit, r to reset", 10, 50, 20, DARKGRAY);

        // draw text above circles 
        DrawText("Func: x += GetFrameTime()*speed", 10, 90, 20, RED);
        DrawText("Func: x += speed", 10, 240, 20, BLUE);

        // end the frame and get ready for the next one  (display frame, poll input, etc...)
        EndDrawing();

    }
    // De-Initialization
    // ------------------------
    // cleanup
    // unload our texture so it can be cleaned up
    UnloadTexture(wabbit);

    // destroy the window and cleanup the OpenGL context
    CloseWindow();
    return 0;
}
