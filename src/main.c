#include "pch.h"
#include "particle.h"
#include "resource_dir.h"	// utility header for SearchAndSetResourceDir
#include "external/glad.h"  // Required for glMemoryBarrier

const int screenWidth = 800;
const int screenHeight = 450;

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
    // AddForce(particleSystem, 
    //     (Force){FORCE_GRAVITY, 0.0f, (Vector2){screenWidth * 0.25f, screenHeight * 0.5f}, 50.0f });
    // AddForce(particleSystem, 
    //     (Force){FORCE_VISCOUS, AIR_VISCOSITY, (Vector2){screenWidth * 0.25f, screenHeight * 0.5f}, 50.0f });
    AddForce(particleSystem, 
        (Force){FORCE_REPULSE, 0.0f, (Vector2){screenWidth * 0.25f, screenHeight * 0.75f}, 5.0e4 });
    AddForce(particleSystem, 
        (Force){FORCE_REPULSE, 0.0f, (Vector2){screenWidth * 0.25f, screenHeight * 0.25f}, 5.0e4 });
    AddForce(particleSystem, 
        (Force){FORCE_REPULSE, 0.0f, (Vector2){screenWidth * 0.75f, screenHeight * 0.5f}, 5.0e4 });

     // Initialize particle rendering pipeline
    SearchAndSetResourceDir("resources");
    Shader particleShader = LoadShader("shaders/particle.vs", "shaders/particle.fs");

    InitParticleRender(&particleShader, (float)screenWidth, (float)screenHeight);

    // Physics Computer Shader initialization
    char *physicsComputeCode = LoadFileText("shaders/physics-compute.glsl");
    uint32_t physicsComputeData = rlCompileShader(physicsComputeCode, RL_COMPUTE_SHADER);
    uint32_t physicsComputeShader = rlLoadComputeShaderProgram(physicsComputeData);
    UnloadFileText(physicsComputeCode);

    uint32_t prevPositionsSSBO  = rlLoadShaderBuffer(sizeof(particleSystem->particles_->pPrevPositions), particleSystem->particles_->pPrevPositions, RL_DYNAMIC_COPY);
    uint32_t positionsSSBO  = rlLoadShaderBuffer(sizeof(particleSystem->particles_->pPositions), particleSystem->particles_->pPositions, RL_DYNAMIC_COPY);
    uint32_t velocitiesSSBO = rlLoadShaderBuffer(sizeof(particleSystem->particles_->pVelocities), particleSystem->particles_->pVelocities, RL_DYNAMIC_COPY);
    uint32_t massesSSBO     = rlLoadShaderBuffer(sizeof(particleSystem->particles_->pMasses), particleSystem->particles_->pMasses, RL_DYNAMIC_COPY);
    
    rlBindShaderBuffer(prevPositionsSSBO, 0);
    rlBindShaderBuffer(positionsSSBO, 1);
    rlBindShaderBuffer(velocitiesSSBO, 2);
    rlBindShaderBuffer(massesSSBO, 3);

    uint32_t local_size_x = 16, local_size_y = local_size_x, local_size_z = 1;
    uint32_t work_groups_x = (uint32_t)sqrt(MAX_PARTICLE_COUNT / (local_size_x*local_size_y)), work_groups_y = work_groups_x, work_groups_z = 1;

    // Main game loop
    while (!WindowShouldClose())        // run the loop until the user presses ESCAPE or presses the Close button on the window
    {
        // Update
        // -----------------------
        float deltaTime = GetFrameTime();
        
        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            EmitParticles(particleSystem, &defaultParticleProps, 4);
        }
        
        emitter->position = GetMousePosition();
        // UpdateParticles(particleSystem, deltaTime);
        rlEnableShader(physicsComputeShader);
        rlSetUniform(rlGetLocationUniform(physicsComputeShader, "uDeltaTime"), &deltaTime, SHADER_UNIFORM_FLOAT, 1);
        rlComputeShaderDispatch(work_groups_x, work_groups_y, work_groups_z);
        rlDisableShader();
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

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
                DrawForces(particleSystem);

                BeginShaderMode(particleShader);
                {
                    // rlUpdateShaderBuffer(positionsSSBO, &particleSystem->particles_->pPositions, sizeof(particleSystem->particles_->pPositions), 0);
                    DrawParticlesInstanced(particleSystem);
                }
                EndShaderMode();
                // DrawParticlesPoints(particleSystem);
            }
            EndMode2D();
            
            // Draw UI elements
            DrawRectangle(5, 10, 200, 50, Fade(SKYBLUE, 0.5f));
            DrawRectangleLines(5, 10, 200, 50, BLUE);
            DrawText(TextFormat("FPS: %i ", GetFPS()), 10, 10, 10, DARKGRAY);
            DrawText(TextFormat("Frame time: %02.04f ms", GetFrameTime()), 10, 20, 10, DARKGRAY);
            DrawText(TextFormat("Particle count: %i", particleSystem->particles_->activeCount), 10, 30, 10, DARKGRAY);
            DrawText(TextFormat("Emitter Coords: (%02.02f, %02.02f)", emitter->position.x, emitter->position.y), 10, 40, 10, DARKGRAY);
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