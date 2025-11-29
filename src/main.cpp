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

void test01_basic_insert() {
    Octree t(2); // side = 4

    // leaf position
    Vector3f p{ 1, 1, 1 };

    // set leaf to non-zero
    t.set(p, 7);
    assert(t.get(p) == 7);

    std::cout << "test01_basic_insert passed\n";
}

void test02_parent_increment() {
    Octree t(3);

    Vector3f p{ 1, 1, 1 };

    t.set(p, 5);

    // ascend manually
    Node* leaf = t.getNode(p);
    Node* parent = leaf->parent;

    assert(parent->value == 1);
    assert(parent->parent->value == 1);

    std::cout << "test02_parent_increment passed\n";
}

void test03_parent_decrement() {
    Octree t(3);

    Vector3f p{ 1, 1, 1 };

    t.set(p, 5); // +1
    t.set(p, 0); // -1

    Node* leaf = t.getNode(p);

    assert(leaf->value == 0);
    assert(leaf->parent->value == 0);
    assert(leaf->parent->parent->value == 0);

    std::cout << "test03_parent_decrement passed\n";
}

void test04_no_update_on_nonzero_to_nonzero() {
    Octree t(3);

    Vector3f p{ 1, 1, 1 };
    t.set(p, 4);

    Node* leaf = t.getNode(p);
    Node* parent = leaf->parent;

    int before = parent->value; // should be 1

    t.set(p, 99); // non-zero → non-zero

    int after = parent->value;

    assert(before == after);
    assert(after == 1);

    std::cout << "test04_no_update_on_nonzero_to_nonzero passed\n";
}

void test05_multiple_children() {
    Octree t(3);

    Vector3f a{ 0, 0, 0 };
    Vector3f b{ 1, 1, 1 };

    t.set(a, 3);
    t.set(b, 2);

    Node* A = t.getNode(a);
    Node* P = A->parent;

    // parent should have count 2
    assert(P->value == 2);

    std::cout << "test05_multiple_children passed\n";
}

void test06_decrement_updates_only_when_needed() {
    Octree t(3);

    Vector3f a{ 0,0,0 };
    Vector3f b{ 1,1,1 };

    t.set(a, 3);
    t.set(b, 3);

    Node* leafA = t.getNode(a);
    Node* parent = leafA->parent;
    assert(parent->value == 2);

    // remove child A
    t.set(a, 0);

    // count should drop from 2 → 1
    assert(parent->value == 1);

    std::cout << "test06_decrement_updates_only_when_needed passed\n";
}

int runTests() {
    test01_basic_insert();
    test02_parent_increment();
    test03_parent_decrement();
    test04_no_update_on_nonzero_to_nonzero();
    test05_multiple_children();
    test06_decrement_updates_only_when_needed();

    std::cout << "ALL TESTS PASSED\n";
    return 0;
}