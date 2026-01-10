#include "pch.h"
#include "particle.h"
#include "resource_dir.h"	// utility header for SearchAndSetResourceDir

const int screenWidth = 800, screenHeight = 450;
const KeyboardKey attractorKey = KEY_A, replusorKey = KEY_D, emitterKey = KEY_W, absorbKey = KEY_S;

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
    TraceLog(LOG_INFO, "RLGL: Version %d", rlGetVersion());

    // Create the window and OpenGL context
    InitWindow((int)screenWidth, (int)screenHeight, "Hello Raylib");

    int currentFPS = 60;
    SetTargetFPS(currentFPS);

    // Define 2D camera
    Camera2D camera = { 0 };
    camera.zoom = 1.0f;

    // Initialize particle system
    ParticleSystem *particleSystem = ConstructParticleSystem(0, screenWidth, 0, screenHeight);
    ParticleEmitter *emitter = &particleSystem->emitter;
    Force *gravity = AddForce(particleSystem, FORCE_GRAVITY);
    Force *attractor = AddForce(particleSystem, FORCE_ATTRACT);
    Force *repulsor = AddForce(particleSystem, FORCE_REPULSE);

     // Initialize particle rendering pipeline
    SearchAndSetResourceDir("resources");
    Shader particleShader = LoadShader("shaders/particle.vs", "shaders/particle.fs");

    InitParticleRender(&particleShader, (float)screenWidth, (float)screenHeight);
    // Main game loop
    while (!WindowShouldClose())        // run the loop until the user presses ESCAPE or presses the Close button on the window
    {
        // Update
        // -----------------------
        float deltaTime = GetFrameTime() * TIMESCALE;
        bool isEmitActive = false, isAbsorbActive = false, isForceActive = false;

        UpdateParticles(particleSystem, deltaTime);

        emitter->position = GetMousePosition();
        attractor->position = GetMousePosition();
        repulsor->position = GetMousePosition();
        attractor->mass = 0.f;
        repulsor->mass = 0.f;
        if (IsKeyDown(attractorKey))
        {
            isForceActive = true;
            attractor->mass = 9e5;
        } else if (IsKeyDown(replusorKey)) {
            isForceActive = true;
            repulsor->mass = 5e5;
        } else if(IsKeyDown(emitterKey)) {
            isEmitActive = true;
            EmitParticles(particleSystem, &defaultParticleProps, 4);
        } else if (IsKeyDown(absorbKey)) {
            isAbsorbActive = true;
            KillParticles(particleSystem, emitter->position, emitter->radius);
        }

        // Drawing
        // ------------------------
        BeginDrawing();
        {
            ClearBackground(BLACK);

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

                BeginShaderMode(particleShader);
                {
                    DrawParticlesInstanced(particleSystem);
                }
                EndShaderMode();
                // DrawParticlesPoints(particleSystem);

                // draw emitor at cursor position
                DrawCircleV(particleSystem->emitter.position,
                    particleSystem->emitter.radius,
                    isEmitActive ? GREEN : isAbsorbActive ? RED : isForceActive ? YELLOW : BLUE);
                // DrawForces(particleSystem);
            }
            EndMode2D();
            
            // Draw UI elements
            uint32_t posX = 5, posY = 5, width = 120, height = 40, fontSize = 10;
            DrawRectangle(posX, posY, width, height, Fade(SKYBLUE, 0.5f));
            DrawRectangleLines(posX, posY, width, height, BLUE);
            DrawText(TextFormat("FPS: %i ", GetFPS()), posX + 5, posY+5, fontSize, WHITE);
            DrawText(TextFormat("Frame time: %02.04f ms", GetFrameTime()), posX+ 5, posY+15, fontSize, WHITE);
            DrawText(TextFormat("Particle count: %i", particleSystem->particles_->activeCount), posX + 5, posY+25, fontSize, WHITE);
        
            posX = screenWidth - 100, posY = 5, width = 30, height = 30, fontSize = 20;
            DrawRectangle(posX, posY, width, height, Fade(SKYBLUE, 0.5f));
            DrawRectangleLines(posX, posY, width, height, isEmitActive ? GREEN : WHITE);
            DrawText(TextFormat("W"), posX+10, posY+5, fontSize, isEmitActive ? GREEN : WHITE);
            
            posX = screenWidth - 100, posY = 40, width = 30, height = 30, fontSize = 20;
            DrawRectangle(posX, posY, width, height, Fade(SKYBLUE, 0.5f));
            DrawRectangleLines(posX, posY, width, height, IsKeyDown(absorbKey) ? RED : WHITE);
            DrawText(TextFormat("S"), posX+10, posY+5, fontSize, IsKeyDown(absorbKey) ? RED : WHITE);
            
            posX = screenWidth - 135, posY = 40, width = 30, height = 30, fontSize = 20;
            DrawRectangle(posX, posY, width, height, Fade(SKYBLUE, 0.5f));
            DrawRectangleLines(posX, posY, width, height, IsKeyDown(attractorKey) ? YELLOW : WHITE);
            DrawText(TextFormat("A"), posX+10, posY+5, fontSize, IsKeyDown(attractorKey) ? YELLOW : WHITE);
            
            posX = screenWidth - 65, posY = 40, width = 30, height = 30, fontSize = 20;
            DrawRectangle(posX, posY, width, height, Fade(SKYBLUE, 0.5f));
            DrawRectangleLines(posX, posY, width, height, IsKeyDown(replusorKey) ? YELLOW : WHITE);
            DrawText(TextFormat("D"), posX+10, posY+5, fontSize, IsKeyDown(replusorKey) ? YELLOW : WHITE);
        }
        // end the frame and get ready for the next one  (display frame, poll input, etc...)
        EndDrawing();
    }
    // De-Initialization
    // ------------------------
    ShutdownParticleRender();
    DestructParticleSystem(particleSystem);
    CloseWindow();
    return 0;
}