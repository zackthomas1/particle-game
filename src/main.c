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
    Color ballColor = DARKBLUE;
    
    // Main game loop
    while (!WindowShouldClose())        // run the loop until the user presses ESCAPE or presses the Close button on the window
    {
        // Update
        // -----------------------
        if (IsKeyPressed(KEY_H)) {
            if (IsCursorHidden()) {
                ShowCursor(); 
            }
            else {
                HideCursor();
            }
        }

        ballPosition = GetMousePosition();

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) { ballColor = MAROON; }
        else if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE)) { ballColor = LIME; }
        else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) { ballColor = DARKBLUE; }
        else if (IsMouseButtonPressed(MOUSE_BUTTON_SIDE)) { ballColor = PURPLE; }
        else if (IsMouseButtonPressed(MOUSE_BUTTON_EXTRA)) { ballColor = YELLOW; }
        else if (IsMouseButtonPressed(MOUSE_BUTTON_FORWARD)) { ballColor = ORANGE; }
        else if (IsMouseButtonPressed(MOUSE_BUTTON_BACK)) { ballColor = BEIGE; }

        // -----------------------

        // Drawing
        // ------------------------
        BeginDrawing();

        // Setup the back buffer for drawing (clear color and depth buffers)
        ClearBackground(RAYWHITE);

        DrawCircleV(ballPosition, 50, ballColor);

        DrawText("move ball with mouse and click mouse button to change color", 10, 10, 20, DARKGRAY);
        DrawText("Press 'H' to toggle cursor visibility", 10, 30, 20, DARKGRAY);

        if (IsCursorHidden()) { 
            DrawText("CURSOR HIDDEN", 20, 60, 20, RED); 
        } else {
            DrawText("CURSOR VISIBLE", 20, 60, 20, LIME);
        }

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
