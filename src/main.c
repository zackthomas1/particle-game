/*
Raylib example file.
This is an example main file for a simple raylib project.
Use this as a starting point or replace it with your code.

by Jeffery Myers is marked with CC0 1.0. To view a copy of this license, visit https://creativecommons.org/publicdomain/zero/1.0/

*/

#include "raylib.h"
#include "resource_dir.h"	// utility header for SearchAndSetResourceDir

#define MAX_TOUCH_POINTS 10

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

    Vector2 touchPositions[MAX_TOUCH_POINTS] = { 0 };
    
    // Main game loop
    while (!WindowShouldClose())        // run the loop until the user presses ESCAPE or presses the Close button on the window
    {
        // Update
        // -----------------------
        // Get the touch point count (how many fingers are touching screen)
        int tCount = GetTouchPointCount();

        // Clamp touch points available
        if (tCount > MAX_TOUCH_POINTS) {
            tCount = MAX_TOUCH_POINTS;
        }

        for (int i = 0; i < tCount; i++) {
            touchPositions[i] = GetTouchPosition(i);
        }

        // -----------------------

        // Drawing
        // ------------------------
        BeginDrawing();

        // Setup the back buffer for drawing (clear color and depth buffers)
        ClearBackground(RAYWHITE);

        for (int i = 0; i < tCount; i++) {
            if ((touchPositions[i].x > 0) && (touchPositions[i].y > 0)) {
                // Draw circle and touch index number 
                DrawCircleV(touchPositions[i], 34, ORANGE); 
                DrawText(TextFormat("%d", i), (int)touchPositions[i].x - 10, (int)touchPositions[i].y - 70, 40, BLACK);
            }
        }

        DrawText("touch the screen at multiple locations to get multiple balls", 10, 10, 20, DARKGRAY);

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
