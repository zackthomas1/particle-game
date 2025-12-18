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

    // Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
    SearchAndSetResourceDir("resources");

    // Create the window and OpenGL context
    const int screenWidth = 800; 
    const int screenHeight = 450;
    InitWindow(screenWidth, screenHeight, "Hello Raylib");

    int currentFPS = 60;
    SetTargetFPS(currentFPS); 

    Vector2 ballPosition = { (float)screenWidth / 2, (float)screenHeight / 2 };
    const float offset = 2.0f;

    // Main game loop
    while (!WindowShouldClose())        // run the loop until the user presses ESCAPE or presses the Close button on the window
    {
        // Update
        // -----------------------
        if (IsKeyDown(KEY_RIGHT)) { ballPosition.x += offset; }
        if (IsKeyDown(KEY_LEFT)) { ballPosition.x -= offset; }
        if (IsKeyDown(KEY_UP)) { ballPosition.y -= offset; }
        if (IsKeyDown(KEY_DOWN)) { ballPosition.y += offset; }

        // -----------------------

        // Drawing
        // ------------------------
        BeginDrawing();

        // Setup the back buffer for drawing (clear color and depth buffers)
        ClearBackground(RAYWHITE);

        DrawText("move ball with arrow keys", 10, 10, 20, DARKGRAY); 
        DrawCircleV(ballPosition, 50, MAROON);

        // end the frame and get ready for the next one  (display frame, poll input, etc...)
        EndDrawing();
        // ------------------------
    }
    // De-Initialization
    // ------------------------
    // destroy the window and cleanup the OpenGL context
    CloseWindow();
    return 0;
}
