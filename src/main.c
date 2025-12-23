#include "pch.h"
#include "particle.h"

// ------------------------
// Program main entry point
// ------------------------
int main ()
{
    // Initialization
    // ------------------------   
    // Tell the window to use vsync and work on high DPI displays
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
    SetTraceLogLevel(LOG_INFO);

    // Create the window and OpenGL context
    const int screenWidth = 800; 
    const int screenHeight = 450;
    InitWindow(screenWidth, screenHeight, "Hello Raylib");

    int currentFPS = 60;
    SetTargetFPS(currentFPS);

    // Define the 2D camera
    Camera2D camera = { 0 };
    camera.zoom = 1.0f;

    ParticleSystem* particleSystem = ConstructParticleSystem();
    AddAffector(particleSystem, 
        (Affector){AFFECTOR_DIRECTION, (Vector2){0.0f, 9.8f}, (Vector2){0.0f, 0.0f}, 0.0f, 0.0f });
    AddAffector(particleSystem, 
        (Affector){AFFECTOR_POINT, (Vector2){0.0f, 0.0f}, (Vector2){screenWidth/2, screenHeight/2}, 50.0f, 10.0f });

    // Main game loop
    while (!WindowShouldClose())        // run the loop until the user presses ESCAPE or presses the Close button on the window
    {
        // Update
        // -----------------------
        float deltaTime = GetFrameTime();
        
        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)) { EmitParticle(particleSystem, &defaultParticleProps); }
        
        particleSystem->emitter.position = GetMousePosition();
        UpdateParticles(particleSystem, deltaTime);

        // Drawing
        // ------------------------
        BeginDrawing();
        {
            ClearBackground(RAYWHITE);

            // Draw Scene
            BeginMode2D(camera);
            {
                // Draw background grid
                rlPushMatrix();
                {
                    rlTranslatef(0, 25*50, 0); 
                    rlRotatef(90,1,0,0);
                    DrawGrid(100, 50);
                }
                rlPopMatrix();

                //
                DrawCircleV(particleSystem->emitter.position, particleSystem->emitter.radius, BLUE);

                DrawParticles(particleSystem);
                DrawAffectors(particleSystem);
            }
            EndMode2D();
            
            // Draw UI elements
            DrawRectangle(5, 10, 320, 93, Fade(SKYBLUE, 0.5f));
            DrawRectangleLines(5, 10, 320, 93, BLUE);
            DrawText(TextFormat("FPS: %i ", GetFPS()), 10, 10, 10, DARKGRAY);
            DrawText(TextFormat("Frame time: %02.02f ms", GetFrameTime()), 10, 20, 10, DARKGRAY);
            DrawText(TextFormat("Particle count: %i", particleSystem->activeCount), 10, 30, 10, DARKGRAY);
        }
        // end the frame and get ready for the next one  (display frame, poll input, etc...)
        EndDrawing();
    }
    // De-Initialization
    // ------------------------
    DestructParticleSystem(particleSystem);
    // destroy the window and cleanup the OpenGL context
    CloseWindow();
    return 0;
}