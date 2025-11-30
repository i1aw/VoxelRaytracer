#include "raylib.h"
#include "raytracer.h"
#include <iostream>
#include <thread>
#include <cassert>
#include "constants.h"
#include "SVO.h"

using namespace std;

void draw(Texture2D& texture, RayTracer& renderer, SparceVoxelOctree& world, int threadCount, int threadIndex);

int main() {

    int THREADS = 16;
    cout << "\nThreads: " << THREADS << "\n\n";
    
    Texture2D texture;
    RayTracer renderer = RayTracer(SCREEN_WIDTH / upscaling, SCREEN_HEIGHT / upscaling);
    SparceVoxelOctree SVO = SparceVoxelOctree(WORLD_SIZE);
    unsigned char* textureBytes = nullptr;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Voxel Renderer");

    SetTargetFPS(100000);

    texture = LoadTextureFromImage(GenImageColor(SCREEN_WIDTH / upscaling, SCREEN_HEIGHT / upscaling, WHITE));

    SVO.set({ 0,0,0 }, { 200,0,255 });
    SVO.set({ 2,0,0 }, { 200,0,255 });
    SVO.set({ 1,1,0 }, { 200,0,255 });
    SVO.set({ 1,2,0 }, { 200,0,255 });

    DisableCursor();
    
    thread* threads = new thread[THREADS];

    while (!WindowShouldClose())
    {
        Vector2 mouse = GetMousePosition();
        SetMousePosition(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);

        float mouseX = (mouse.x / (SCREEN_WIDTH / 2.0)) - 1;
        float mouseY = (mouse.y / (SCREEN_HEIGHT / 2.0)) - 1;

        renderer.yaw += mouseX * SENSITIVITY;
        renderer.pitch += -mouseY * SENSITIVITY;

        renderer.updateVectors();

        float deltaTime = GetFrameTime();

        if (IsKeyDown(KEY_D)) {
            renderer.moveRight(MOVEMENT_SPEED * deltaTime);
        }
        if (IsKeyDown(KEY_A)) {
            renderer.moveRight(-MOVEMENT_SPEED * deltaTime);
        }
        if (IsKeyDown(KEY_W)) {
            renderer.moveForward(MOVEMENT_SPEED * deltaTime);
        }
        if (IsKeyDown(KEY_S)) {
            renderer.moveForward(-MOVEMENT_SPEED * deltaTime);
        }
        if (IsKeyDown(KEY_SPACE)) {
            renderer.moveUp(-MOVEMENT_SPEED * deltaTime);
        }
        if (IsKeyDown(KEY_LEFT_SHIFT)) {
            renderer.moveUp(MOVEMENT_SPEED * deltaTime);

        }

        BeginDrawing();

        for (int i = 0; i < THREADS; i++) {
            threads[i] = thread(&RayTracer::render, std::ref(renderer), std::ref(SVO), THREADS, i);
                
        }

        for (int i = 0; i < THREADS; i++) {
            threads[i].join();
        }

        SetTextureFilter(texture, TEXTURE_FILTER_ANISOTROPIC_16X);
        UpdateTexture(texture, renderer.textureData);
        Rectangle src = { 0, 0, texture.width, texture.height };
        Rectangle dst = { 0, 0, GetScreenWidth(), GetScreenHeight() };
        DrawTexturePro(texture, src, dst, { 0, 0 }, 0.0f, WHITE);
        DrawText(TextFormat("%i FPS", GetFPS()), 10, 15, 30, WHITE);
        DrawText(TextFormat(
            "X: %f Y: %f Z: %f",
            renderer.position.x,
            renderer.position.y,
            renderer.position.z),
            10, 45, 30, WHITE);


        EndDrawing();
    }

    CloseWindow();

    delete[] threads;

    return 0;
}

void draw(Texture2D& texture, RayTracer& renderer, SparceVoxelOctree& world, int threadCount, int threadIndex) {
    UpdateTexture(texture, renderer.render(world, threadCount, threadIndex));
}