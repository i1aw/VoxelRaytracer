#include "raylib.h"
#include "raytracer.h"
#include "voxel.h"
#include <iostream>
#include <thread>
#include <cassert>

using namespace std;

void draw(Texture2D& texture, RayTracer& renderer, Octree& world, int threadCount, int threadIndex);

int main() {

    int THREADS = std::thread::hardware_concurrency();
    cout << "\nThreads: " << THREADS << "\n\n";
    const float SENSITIVITY = 1;
    const float upscaling = 2;

    const int SCREEN_WIDTH = 1920;
    const int SCREEN_HEIGHT = 1080;

    const float MOVEMENT_SPEED = 2.5; // units / second

    const unsigned int WORLD_SIZE = 2;
    
    Texture2D texture;
    RayTracer renderer = RayTracer(SCREEN_WIDTH / upscaling, SCREEN_HEIGHT / upscaling);
    Octree OctWorld = Octree(WORLD_SIZE);
    unsigned char* textureBytes = nullptr;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Voxel Renderer");

    SetTargetFPS(100000);

    texture = LoadTextureFromImage(GenImageColor(SCREEN_WIDTH / upscaling, SCREEN_HEIGHT / upscaling, WHITE));

    OctWorld.set({ 0,0,0 }, 1);
    OctWorld.set({ 2,0,0 }, 1);
    OctWorld.set({ 1,1,0 }, 1);
    OctWorld.set({ 1,2,0 }, 1);

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
            //threads[i] = thread(draw, std::ref(texture), std::ref(renderer), std::ref(world), THREADS, i);
            threads[i] = thread(&RayTracer::render, std::ref(renderer), std::ref(OctWorld), THREADS, i);
                
        }

        for (int i = 0; i < THREADS; i++) {
            threads[i].join();
        }

        SetTextureFilter(texture, TEXTURE_FILTER_ANISOTROPIC_16X);
        UpdateTexture(texture, renderer.textureData);
        //DrawTexture(texture, 0, 0, WHITE);
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

void draw(Texture2D& texture, RayTracer& renderer, Octree& world, int threadCount, int threadIndex) {
    UpdateTexture(texture, renderer.render(world, threadCount, threadIndex));
}