/*
Raylib example file.
This is an example main file for a simple raylib project.
Use this as a starting point or replace it with your code.

by Jeffery Myers is marked with CC0 1.0. To view a copy of this license, visit https://creativecommons.org/publicdomain/zero/1.0/

*/

#include "raylib.h"
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

    Rectangle player = { 400, 280, 40, 40 };
    Rectangle buildings[MAX_BUILDINGS] = { 0 };
    Color buildColors[MAX_BUILDINGS] = { 0 };

    int spacing = 0;
    
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        buildings[i].width = (float)GetRandomValue(50, 200);
        buildings[i].height = (float)GetRandomValue(100, 800);
        buildings[i].y = screenHeight - 130.0f - buildings[i].height;
        buildings[i].x = -6000.00f + spacing;

        spacing += (int)buildings[i].width;

        buildColors[i] = (Color){
            (unsigned char)GetRandomValue(200, 240),
            (unsigned char)GetRandomValue(200, 240),
            (unsigned char)GetRandomValue(200, 250),
            255
        };
    }

    Camera2D camera = { 0 }; 
    camera.target = (Vector2){ player.x + 20.0f, player.y + 20.0f };
    camera.offset = (Vector2){ screenWidth / 2.0f, screenHeight / 2.0f };
    camera.rotation = 0.0f; 
    camera.zoom = 1.0f;

    // Main game loop
    while (!WindowShouldClose())        // run the loop until the user presses ESCAPE or presses the Close button on the window
    {
        // Update
        // -----------------------
        // player movement
        if (IsKeyDown(KEY_RIGHT)) { 
            player.x += 2; 
        }
        else if (IsKeyDown(KEY_LEFT)) {
            player.x -= 2;
        }

        // camera target follows player
        camera.target = (Vector2){ player.x + 20, player.y + 20 };

        // Camera rotation controls
        if (IsKeyDown(KEY_A)) {
            camera.rotation--;
        }
        else if (IsKeyDown(KEY_S)) {
            camera.rotation++;
        }

        // limit camera rotation to 80 degrees [-40,40]
        if (camera.rotation > 40) {
            camera.rotation = 40;
        }
        else if (camera.rotation < -40) {
            camera.rotation = -40;
        }

        // Camera zoom controls
        // Use log scaling to provide consistent zoom speed
        camera.zoom = camera.zoom + ((float)GetMouseWheelMove() * 0.1f);

        if (camera.zoom > 3.0f) {
            camera.zoom = 3.0f;
        }
        else if (camera.zoom < 0.1f) {
            camera.zoom = 0.1f;
        }

        // camera reset (zoom/rotation)
        if (IsKeyPressed(KEY_R)) {
            camera.zoom = 1.0f;
            camera.rotation = 0.0f;
        }

        // -----------------------

        // Drawing
        // ------------------------
        BeginDrawing();
        {
            // Setup the back buffer for drawing (clear color and depth buffers)
            ClearBackground(RAYWHITE);

            BeginMode2D(camera);
            {
                DrawRectangle(-6000, 320, 13000, 8000, DARKGRAY);

                for (int i = 0; i < MAX_BUILDINGS; i++) {
                    DrawRectangleRec(buildings[i], buildColors[i]);
                }

                DrawRectangleRec(player, RED);

                DrawLine((int)camera.target.x, -screenHeight * 10, (int)camera.target.x, screenHeight * 10, GREEN);
                DrawLine(-screenWidth * 10, (int)camera.target.y, screenWidth * 10, (int)camera.target.y, GREEN);
            }
            EndMode2D();

            DrawText("SCREEN AREA", 640, 10, 20, RED);


            DrawRectangle(0, 0, screenWidth, 5, RED);
            DrawRectangle(0, 5, 5, screenHeight - 10, RED);
            DrawRectangle(screenWidth - 5, 5, 5, screenHeight - 10, RED);
            DrawRectangle(0, screenHeight - 5, screenWidth, 5, RED);

            DrawRectangle(10, 10, 250, 113, Fade(SKYBLUE, 0.5f));
            DrawRectangleLines(10, 10, 250, 113, BLUE);

            DrawText("Free 2D camera controls:", 20, 20, 10, BLACK);
            DrawText("- Right/Left to move player", 40, 40, 10, DARKGRAY);
            DrawText("- Mouse Wheel to Zoom in-out", 40, 60, 10, DARKGRAY);
            DrawText("- A / S to Rotate", 40, 80, 10, DARKGRAY);
            DrawText("- R to reset Zoom and Rotation", 40, 100, 10, DARKGRAY);
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
