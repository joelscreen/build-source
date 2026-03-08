#include "r3d/r3d.h"
#include <raymath.h>
#include <vector>
#include "menu.h"

struct BLOCKS {
  Vector3 pos;
};

#include "blocks.h"

int main() {
  // ----- SETTING UP THE SCREEN -----
  InitWindow(800, 600, "Build alpha-0.1.0");
  SetTargetFPS(60);

  R3D_Init(800, 600);

  DisableCursor();

  // ----- VARIABLES -----

  float cubeSize = 1;
  int coins = 0;
  bool openShopMenu = false;
  bool pauseMenu = false;
  bool menu = false;
  bool doubleClicker = false;
  int clickerVal = 1;
  bool autoClicker = false;
  int autoClickerVal = 0;
  int frames = 0;
  int framesTarget = 60;
  int blockSize = 1;
  std::vector<BLOCKS> blockData = {{100000.0f, 0.0f, 100000.0f}};
  bool isBockPlaced = false;
  float maxBlockDistance = 4.0f;
  float playerHeight = 0.0f;
  float jumpVelocity = 0.0f;
  float gravity = 0.01f;
  bool onGround = true;
  bool shouldClose = true;

  // ----- MESHES -----

  // Generating meshes
  R3D_Mesh plane = R3D_GenMeshPlane(1000, 1000, 1, 1);
  R3D_Mesh cube = R3D_GenMeshCube(cubeSize, cubeSize, cubeSize);
  R3D_Mesh shopkeeper = R3D_GenMeshCube(1, 2, 1);
  R3D_Mesh block = R3D_GenMeshCube(cubeSize, cubeSize, cubeSize);

  // Default Material (only for debugging)
  R3D_Material shopMaterial = R3D_GetDefaultMaterial();

  // Textures
  R3D_Material cubeMaterial = R3D_GetDefaultMaterial();
  cubeMaterial.albedo.texture = LoadTexture("../assets/cubeTexture.png");

  R3D_Material blockMaterial = R3D_GetDefaultMaterial();
  blockMaterial.albedo.texture = LoadTexture("../assets/brickTexture.png");

  R3D_Material planeMaterial = R3D_GetDefaultMaterial();
  planeMaterial.albedo.texture = LoadTexture("../assets/planeTexture.png");

  R3D_ENVIRONMENT_SET(ambient.color, (Color){10, 10, 10, 0});
  // ----- LIGHTS -----

  // Setting up the light
  R3D_Light light = R3D_CreateLight(R3D_LIGHT_DIR);
  R3D_SetLightDirection(light, (Vector3){1, -1, -1});
  R3D_EnableShadow(light);
  R3D_SetLightActive(light, true);

  // ----- CAMERA -----

  // Camera
  Camera3D camera = {0};
  camera.position = {0, 2, 2};
  camera.target = {0, 0, 0};
  camera.up = {0, 1, 0};
  camera.fovy = 70;
  camera.projection = CAMERA_PERSPECTIVE;

  // ----- GAMELOOP -----

  while (!WindowShouldClose() && shouldClose) {
    SetExitKey(KEY_NULL);

    if (!menu) {
      UpdateCamera(&camera, CAMERA_FIRST_PERSON);
    }

    if (IsKeyPressed(KEY_F11)) {
      ToggleFullscreen();
    }

    // Updating bounding boxes
    BoundingBox cubeBox = cube.aabb;
    cubeBox.min = Vector3Add(cubeBox.min, (Vector3){0, cubeSize / 2, 0});
    cubeBox.max = Vector3Add(cubeBox.max, (Vector3){0, cubeSize / 2, 0});

    BoundingBox shopBox = shopkeeper.aabb;
    shopBox.min = Vector3Add(shopBox.min, (Vector3){5, 1, 5});
    shopBox.max = Vector3Add(shopBox.max, (Vector3){5, 1, 5});

    // Click detection
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      Vector2 screenCenter = {400, 300};
      Ray ray = GetMouseRay(screenCenter, camera);

      RayCollision cubeClick = GetRayCollisionBox(ray, cubeBox);
      RayCollision shopClick = GetRayCollisionBox(ray, shopBox);
      if (cubeClick.hit && !menu && !pauseMenu) {
        coins += clickerVal;
      }

      if (shopClick.hit && !menu) {
        openShopMenu = true;
        menu = true;
        EnableCursor();
      }
    }

    // Blocks Placement
    Vector2 screenCenter = {400, 300};
    Ray blockPos = GetMouseRay(screenCenter, camera);
    RayCollision blockPosPlane = GetRayCollisionQuad(
        blockPos, (Vector3){-1000, 0, 1000}, (Vector3){1000, 0, 1000},
        (Vector3){1000, 0, -1000}, (Vector3){-1000, 0, -1000});
    BlocksPlacement(camera, maxBlockDistance, blockData, blockSize);
    // Jumping logic
    if (onGround && IsKeyDown(KEY_SPACE)) {
      jumpVelocity = 0.15f;
      onGround = false;
    }

    if (!onGround) {
      playerHeight += jumpVelocity;
      jumpVelocity -= gravity;

      if (playerHeight <= 0.0f) {
        playerHeight = 0.0f;
        onGround = true;
        jumpVelocity = 0.0f;
      }

      camera.position.y = 2.0f + playerHeight;
      camera.target.y += jumpVelocity;
    }

    // Drawing
    BeginDrawing();
    // R3D Drawing
    R3D_Begin(camera);
    R3D_DrawMesh(plane, planeMaterial, (Vector3){0, 0, 0}, 1.0f);
    R3D_DrawMesh(cube, cubeMaterial, (Vector3){0, cubeSize / 2, 0}, 1.0f);
    R3D_DrawMesh(shopkeeper, shopMaterial, (Vector3){5, 1, 5}, 1.0f);
    for (auto &pos_index : blockData) {
      R3D_DrawMesh(block, blockMaterial,
                   pos_index.pos,
                   (float)blockSize);
    }
    R3D_End();

    BeginMode3D(camera);
    DrawBlocks(camera, maxBlockDistance, blockData, blockSize, blockPosPlane);
    EndMode3D();

    // Crosshair
    DrawLine(400, 290, 400, 310, GRAY);
    DrawLine(390, 300, 410, 300, GRAY);

    // Coins text
    DrawText(TextFormat("Coins: $%d", coins), 10, 10, 30, BLACK);

    // Shop menu logic
    DrawShopMenu(openShopMenu, coins, framesTarget, menu, doubleClicker, clickerVal, autoClicker, autoClickerVal, pauseMenu);

    // Pause menu logic
    DrawPauseMenu(pauseMenu, menu, shouldClose);

    EndDrawing();

    frames++;

    if (autoClickerVal != 0) {
      if (frames >= GetFPS() / autoClickerVal) {
        frames = 0;
      }
    } else {
      if (frames >= GetFPS()) {
        frames = 0;
      }
    }

    // Auto Clicker logic
    if (autoClickerVal != 0) {
      if (autoClicker && frames == framesTarget - 1) {
        coins++;
      }
    } else {
      if (autoClicker && frames == GetFPS() - 1) {
        coins++;
      }
    }
  }

  // ----- UNLOADING AND CLOSING -----

  // Unloading Meshes
  R3D_UnloadMesh(plane);
  R3D_UnloadMesh(cube);

  // Closing the program
  R3D_Close();
  CloseWindow();
  return 0;
}
