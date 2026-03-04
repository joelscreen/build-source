#include "r3d/r3d.h"
#include <iostream>
#include <list>
#include <print>
#include <raymath.h>
#include <vector>

struct BLOCKS {
  Vector3 pos;
};

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

  // ----- MESHES -----

  // Generating meshes
  R3D_Mesh plane = R3D_GenMeshPlane(1000, 1000, 1, 1);
  R3D_Mesh cube = R3D_GenMeshCube(cubeSize, cubeSize, cubeSize);
  R3D_Mesh shopkeeper = R3D_GenMeshCube(1, 2, 1);
  R3D_Mesh block = R3D_GenMeshCube(cubeSize, cubeSize, cubeSize);

  // Default Material (only for debugging)
  R3D_Material material = R3D_GetDefaultMaterial();

  // Textures
  R3D_Material cubeMaterial = R3D_GetDefaultMaterial();
  cubeMaterial.albedo.texture = LoadTexture("../assets/cubeTexture.png");

  R3D_Material blockMaterial = R3D_GetDefaultMaterial();
  blockMaterial.albedo.texture = LoadTexture("../assets/brickTexture.png");

  R3D_Material planeMaterial = R3D_GetDefaultMaterial();
  planeMaterial.albedo.texture = LoadTexture("../assets/planeTexture.png");

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

  while (!WindowShouldClose()) {
    SetExitKey(KEY_NULL);

    if (!menu) {
      UpdateCamera(&camera, CAMERA_FIRST_PERSON);
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

    Vector2 screenCenter = {400, 300};
    Ray blockPos = GetMouseRay(screenCenter, camera);
    RayCollision blockPosPlane = GetRayCollisionQuad(
        blockPos, (Vector3){-1000, 0, 1000}, (Vector3){1000, 0, 1000},
        (Vector3){1000, 0, -1000}, (Vector3){-1000, 0, -1000});

    float maxDistance = fminf(blockPosPlane.distance, maxBlockDistance);
    for (auto &blockPosition : blockData) {
      BoundingBox bb = (BoundingBox){
          .min = (Vector3){blockPosition.pos.x - blockSize / 2.0f,
                           blockPosition.pos.y - blockSize / 2.0f,
                           blockPosition.pos.z - blockSize / 2.0f},
          .max = Vector3Add(
              blockPosition.pos,
              (Vector3){blockSize / 2.0f, blockSize / 2.0f, blockSize / 2.0f})};
      RayCollision blockPosBlock = GetRayCollisionBox(blockPos, bb);

      if (blockPosBlock.hit && blockPosBlock.distance <= maxDistance) {
        maxDistance = blockPosBlock.distance;
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
          Vector3 blockNormal = blockPosBlock.normal;
          Vector3 newPos = Vector3Add(
              blockPosition.pos,
              blockNormal);
          blockData.push_back((BLOCKS){newPos.x, newPos.y, newPos.z});
        }
      }
    }

    if (blockPosPlane.hit && blockPosPlane.distance <= maxDistance) {
      Vector3 point = blockPosPlane.point;
      int x = std::round(point.x / blockSize) * blockSize;
      int y = std::round(point.y / blockSize) * blockSize;
      int z = std::round(point.z / blockSize) * blockSize;

      if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
        BLOCKS pos = (BLOCKS){(float)x, (float)y + 0.5f, (float)z};
        blockData.push_back(pos);
      }
    }

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
    R3D_DrawMesh(shopkeeper, material, (Vector3){5, 1, 5}, 1.0f);
    for (auto &pos_index : blockData) {
      R3D_DrawMesh(block, blockMaterial,
                   pos_index.pos,
                   (float)blockSize);
    }
    R3D_End();

    BeginMode3D(camera);
    if (blockPosPlane.hit && blockPosPlane.distance <= maxBlockDistance) {
      Vector3 point = blockPosPlane.point;
      int x = std::round(point.x / blockSize) * blockSize;
      int y = std::round(point.y / blockSize) * blockSize;
      int z = std::round(point.z / blockSize) * blockSize;

      Vector3 pos = (Vector3){(float)x, (float)y - 0.49f, (float)z};
      DrawCubeWires(pos, 1.0f, 1.0f, 1.0f, BLACK);
    }
    for (auto &blockPosition : blockData) {
      BoundingBox bb = (BoundingBox){
          .min = (Vector3){blockPosition.pos.x - blockSize / 2.0f,
                           blockPosition.pos.y - blockSize / 2.0f,
                           blockPosition.pos.z - blockSize / 2.0f},
          .max = Vector3Add(
              blockPosition.pos,
              (Vector3){blockSize / 2.0f, blockSize / 2.0f, blockSize / 2.0f})};
      RayCollision blockPosBlock = GetRayCollisionBox(blockPos, bb);

      if (blockPosBlock.hit && blockPosBlock.distance == maxDistance) {
        Vector3 blockNormal = blockPosBlock.normal;
        Vector3 oldPos = blockPosition.pos;
        Vector3 newPos = Vector3Add(oldPos, blockNormal);
        DrawCubeWires(oldPos, 1.0f, 1.0f, 1.0f, BLACK);
      }
    }
    EndMode3D();

    // Crosshair
    DrawLine(400, 290, 400, 310, GRAY);
    DrawLine(390, 300, 410, 300, GRAY);

    // Coins text
    DrawText(TextFormat("Coins: $%d", coins), 10, 10, 30, BLACK);

    // Shop menu logic
    if (openShopMenu) {
      DrawRectangle(30, 30, 740, 540, GRAY); // Main menu rectangle
      int items_y = 60;
      for (int o = 0; o < 3; o++) {
        for (int i = 0; i <= 4; i++) {
          DrawRectangle((i * 145) + 35, items_y, 140, 135, DARKGRAY);
          DrawCircle((i * 145) + 110, items_y + 35, 30, YELLOW);
        }
        items_y += 140;
        if (coins < 100) {
          DrawText("Double clicker", 37, 140, 20, GRAY);
          DrawText("$100", 85, 160, 20, GRAY);
        } else {
          DrawText("Double clicker", 37, 140, 20, BLACK);
          DrawText("$100", 85, 160, 20, BLACK);
        }
        if (coins < 200 || framesTarget == 1) {
          DrawText("Auto clicker", 190, 140, 20, GRAY);
          DrawText("$200", 225, 160, 20, GRAY);
        } else {
          DrawText("Auto clicker", 190, 140, 20, BLACK);
          DrawText("$200", 225, 160, 20, BLACK);
        }
      }
      if (IsKeyDown(KEY_ESCAPE)) {
        openShopMenu = false;
        menu = false;
        DisableCursor();
      }
      if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && GetMouseX() > 35 &&
          GetMouseX() < 175 && GetMouseY() > 60 && GetMouseY() < 195 &&
          coins >= 100) {
        coins -= 100;
        doubleClicker = true;
        clickerVal *= 2;
      }
      if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && GetMouseX() > 180 &&
          GetMouseX() < 320 && GetMouseY() > 60 && GetMouseY() < 195 &&
          coins >= 200 && framesTarget != 1) {
        autoClicker = true;
        coins -= 200;
        autoClickerVal += 1;
        framesTarget = std::max((int)(framesTarget / 2), 1);
      }
    } else if (IsKeyPressed(KEY_ESCAPE) && !openShopMenu && !pauseMenu) {
      pauseMenu = true;
      menu = true;
      EnableCursor();
    } else if (IsKeyPressed(KEY_ESCAPE) && !openShopMenu && pauseMenu) {
      pauseMenu = false;
      menu = false;
      DisableCursor();
    }

    // Pause menu logic
    if (pauseMenu) {
      // Back to game
      DrawRectangle(200, 180, 400, 70, GRAY);
      DrawText("Back to game", 300, 200, 30, BLACK);
      if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (GetMouseX() > 200 && GetMouseX() < 600) {
          if (GetMouseY() > 180 && GetMouseY() < 250) {
            pauseMenu = false;
            menu = false;
            DisableCursor();
          }
        }
      }

      // Quit game
      DrawRectangle(200, 380, 400, 70, GRAY);
      DrawText("Quit game", 300, 400, 30, BLACK);
      if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (GetMouseX() > 200 && GetMouseX() < 600) {
          if (GetMouseY() > 380 && GetMouseY() < 550) {
            break;
          }
        }
      }
    }

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