#include "pch.h"
#include "particle.h"
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
    SetTraceLogLevel(LOG_INFO);
    TraceLog(LOG_INFO, "RLGL: Version %d", rlGetVersion());

    // Create the window and OpenGL context
    const int screenWidth = 800; 
    const int screenHeight = 450;
    InitWindow(screenWidth, screenHeight, "Hello Raylib");

    int currentFPS = 60;
    SetTargetFPS(currentFPS);

    // Define 2D camera
    Camera2D camera = { 0 };
    camera.zoom = 1.0f;

    // Initialize particle system
    ParticleSystem *particleSystem = ConstructParticleSystem(0, screenWidth, 0, screenHeight);
    ParticleEmitter *emitter = &particleSystem->emitter;
    AddForce(particleSystem, 
        (Force){FORCE_GRAVITY, 0.0f, (Vector2){screenWidth * 0.25f, screenHeight * 0.5f}, 50.0f });
    AddForce(particleSystem, 
        (Force){FORCE_VISCOUS, AIR_VISCOSITY, (Vector2){screenWidth * 0.25f, screenHeight * 0.5f}, 50.0f });
    // AddForce(particleSystem, 
    //     (Force){FORCE_REPULSE, 0.0f, (Vector2){screenWidth * 0.25f, screenHeight * 0.5f}, 5.0e5 });
    // AddForce(particleSystem, 
    //     (Force){FORCE_REPULSE, 0.0f, (Vector2){screenWidth * 0.75f, screenHeight * 0.5f}, 5.0e5 });

    // // Set up vertex data
    // float quadVertices [] = {
    //     // positions
    //     0.71f,  0.71f,
    //     -0.71f,  0.71f,
    //     -0.71f, -0.71f,
        
    //     0.71f, -0.71f,
    //     0.71f, -0.71f,
    //     0.71f,  0.71f,
    // };
    // uint32_t quadVAO, quadVBO;
    // quadVAO = rlLoadVertexArray();
    // quadVBO = rlLoadVertexBuffer(&quadVertices, sizeof(quadVertices) / sizeof(float), false);
    // rlEnableVertexAttribute(0);
    // rlSetVertexAttribute(0, 2, RL_FLOAT, false, 2 * sizeof(float), 0);

    // uint32_t instancePositionVBO;
    // instancePositionVBO = rlLoadVertexBuffer(&particleSystem->particles_->pPositions, sizeof(particleSystem->particles_->pPositions) / sizeof(Vector2), false);

    // // Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
    // SearchAndSetResourceDir("resources");

    // Shader particleShader = LoadShader("shaders/particle.vs", "shaders/particle.fs");

    // Main game loop
    while (!WindowShouldClose())        // run the loop until the user presses ESCAPE or presses the Close button on the window
    {
        // Update
        // -----------------------
        float deltaTime = GetFrameTime();
        
        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)) 
        {
            Vector2 pos = Vector2Add(emitter->position, 
                            Vector2Scale((Vector2){ GetRandomValueF(), GetRandomValueF() }, emitter->radius));
            EmitParticle(particleSystem, pos, &defaultParticleProps);
        }
        
        emitter->position = GetMousePosition();
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

                // draw emitor at cursor position
                DrawCircleV(particleSystem->emitter.position, particleSystem->emitter.radius, BLUE);

                // BeginShaderMode(particleShader);
                // {
                //     DrawParticles(particleSystem);
                // }
                // EndShaderMode();

                DrawParticles(particleSystem);
                DrawForces(particleSystem);
            }
            EndMode2D();
            
            // Draw UI elements
            DrawRectangle(5, 10, 320, 93, Fade(SKYBLUE, 0.5f));
            DrawRectangleLines(5, 10, 320, 93, BLUE);
            DrawText(TextFormat("FPS: %i ", GetFPS()), 10, 10, 10, DARKGRAY);
            DrawText(TextFormat("Frame time: %02.02f ms", GetFrameTime()), 10, 20, 10, DARKGRAY);
            DrawText(TextFormat("Particle count: %i", particleSystem->particles_->activeCount), 10, 30, 10, DARKGRAY);
            DrawText(TextFormat("Emitter Coords: (%02.02f, %02.02f)", emitter->position.x, emitter->position.y), 10, 40, 10, DARKGRAY);
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