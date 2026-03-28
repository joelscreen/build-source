#include "r3d/r3d.h"
#include <raymath.h>
#include <vector>
#include "menu/menu.h"
#include <iostream>

#include "blocks/blocks.h"

int main() {
  // ----- SETTING UP THE SCREEN -----
  InitWindow(800, 600, "Build alpha-0.0.1");
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
  std::vector<BLOCKS> blockData = {{100000.0f, -100000.0f, 100000.0f}};
  bool isBockPlaced = false;
  float maxBlockDistance = 4.0f;
  float playerY = 0.0f;
  float jumpVelocity = 0.0f;
  float gravity = 0.01f;
  bool onGround = true;
  bool shouldClose = true;
  int blocks = 0;
  bool canJump = true;
  Vector3 cubeLocation = { 0, cubeSize/2, 0 };
  Vector3 shopkeeperLocation = {5, 1, 5};

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

  // Skybox
  /*
  R3D_Cubemap sky = R3D_LoadCubemap("assets/sky.hdr", R3D_CUBEMAP_LAYOUT_AUTO_DETECT);

  R3D_EnvBackground background = {0};
  background.sky = sky;
  background.energy = 1.0f;

  R3D_EnvAmbient ambient = {0};
  ambient.map = R3D_GenAmbientMap(sky, R3D_AMBIENT_ILLUMINATION | R3D_AMBIENT_REFLECTION);
  ambient.energy = 1.0f;

  R3D_ENVIRONMENT_SET(background, background);
  R3D_ENVIRONMENT_SET(ambient, ambient);
  */

  R3D_ENVIRONMENT_SET(ambient.color, Color{10, 10, 10, 0});

  // ----- LIGHTS -----

  // Setting up the light
  R3D_Light light = R3D_CreateLight(R3D_LIGHT_DIR);
  R3D_SetLightDirection(light, Vector3{1, -1, -1});
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

    Vector3 oldCamPos = camera.position;

    if (!menu) {
      UpdateCamera(&camera, CAMERA_FIRST_PERSON);
    }
    // Jumping logic
    if (canJump) {
      if (onGround && IsKeyDown(KEY_SPACE)) {
        jumpVelocity = 0.2f;
        onGround = false;
      }

      if (!onGround) {
        playerY += jumpVelocity * GetFrameTime();
        jumpVelocity -= gravity;

        if (playerY <= 0.0f) {
          playerY = 0.0f;
          onGround = true;
          jumpVelocity = 0.0f;
        }
        camera.position.y = 2.0f + playerY * 50;
        camera.target.y += jumpVelocity * GetFrameTime() * 50;
      }
    }

    // Collision detection with blocks
    for (auto &block : blockData) {
      if (CheckCollisionBoxes(
        BoundingBox{Vector3{ camera.position.x - 0.5f,
                                 camera.position.y - 2.0f - 1.0f,
                                 camera.position.z - 0.5f },
                      Vector3{ camera.position.x + 0.5f,
                                 camera.position.y - 2.0f + 1.0f,
                                 camera.position.z + 0.5f }},
        BoundingBox{Vector3{ block.pos.x - 0.5f,
                                 block.pos.y - 0.5f,
                                 block.pos.z - 0.5f },
                      Vector3{ block.pos.x + 0.5f,
                                 block.pos.y + 0.5f,
                                 block.pos.z + 0.5f }})) {
        bool frontX = oldCamPos.x < camera.position.x && block.pos.x - 0.5f > camera.position.x && oldCamPos.y - 2.0f <= block.pos.y + 0.5f;
        bool backX = oldCamPos.x > camera.position.x && block.pos.x + 0.5f < camera.position.x && oldCamPos.y - 2.0f <= block.pos.y + 0.5f;

        bool frontZ = oldCamPos.z > camera.position.z && block.pos.z + 0.5f < camera.position.z && oldCamPos.y - 2.0f <= block.pos.y + 0.5f;
        bool backZ = oldCamPos.z < camera.position.z && block.pos.z - 0.5f > camera.position.z && oldCamPos.y - 2.0f <= block.pos.y + 0.5f;

        // X-axis collision
        if (frontX && !frontZ && !backZ) {
          camera.position.x -= 0.089997f;
          camera.target.y -= 0.089997f;
        }
        else if (backX && !frontZ && !backZ) {
          camera.position.x += 0.089997f;
          camera.target.y -= 0.089997f;
        }

        // Y-axis collision
        float playerBottom = camera.position.y - 2.0f;
        float blockTop = block.pos.y + 0.5f;

        bool insideX = camera.position.x >= block.pos.x - 0.5f &&
                        camera.position.x <= block.pos.x + 0.5f;

        bool insideZ = camera.position.z >= block.pos.z - 0.5f &&
                        camera.position.z <= block.pos.z + 0.5f;

        if (insideX && insideZ && playerBottom <= blockTop + 0.1f && playerBottom >= blockTop - 0.2f) {
          camera.position.y = blockTop + 2.0f;
          onGround = true;
          jumpVelocity = 0.0f;
        }
        if (camera.position.y - 2.0f < 0.0f) {
          camera.position.y = 2.0f;
        }

        // Z-axis collision
        if (frontZ && !frontX && !backX) {
          camera.position.z += 0.089997f;
          camera.target.y -= 0.089997f;
        }
        else if (backZ && !frontX && !backX) {
          camera.position.z -= 0.089997f;
          camera.target.y -= 0.089997f;
        }

        // Diagonal collision
        if (frontX && frontZ) {
          camera.position.x += 0.06364f;
          camera.position.z += 0.06364f;
        }
        if (backX && backZ) {
          camera.position.x -= 0.06364f;
          camera.position.z -= 0.06364f;
        }
        if (frontX && backZ) {
          camera.position.x += 0.06364f;
          camera.position.z -= 0.06364f;
        }
        if (backX && frontZ) {
          camera.position.x -= 0.06364f;
          camera.position.z += 0.06364f;
        }
      }
      float playerBottom = camera.position.y - 2.0f;
      float blockTop = block.pos.y + 0.5f;
      if (playerBottom != blockTop && playerBottom > 0) {
        onGround = false;
      }
    }
    if (camera.position.y - 2.0f < 0) camera.position.y = 2.0f;

    // Cube collision
    if (CheckCollisionBoxes(
      BoundingBox{Vector3{ camera.position.x - 0.5f,
                               camera.position.y - 2.0f - 1.0f,
                               camera.position.z - 0.5f },
                    Vector3{ camera.position.x + 0.5f,
                               camera.position.y - 2.0f + 1.0f,
                               camera.position.z + 0.5f }},
      BoundingBox{Vector3{ cubeLocation.z - 0.5f,
                               cubeLocation.y - 0.5f,
                               cubeLocation.z - 0.5f },
                    Vector3{ cubeLocation.x + 0.5f,
                               cubeLocation.y + 0.5f,
                               cubeLocation.z + 0.5f }})) {
      bool frontX = oldCamPos.x < camera.position.x && cubeLocation.x - 0.5f > camera.position.x && oldCamPos.y - 2.0f <= cubeLocation.y + 0.5f;
      bool backX = oldCamPos.x > camera.position.x && cubeLocation.x + 0.5f < camera.position.x && oldCamPos.y - 2.0f <= cubeLocation.y + 0.5f;

      bool frontZ = oldCamPos.z > camera.position.z && cubeLocation.z + 0.5f < camera.position.z && oldCamPos.y - 2.0f <= cubeLocation.y + 0.5f;
      bool backZ = oldCamPos.z < camera.position.z && cubeLocation.z - 0.5f > camera.position.z && oldCamPos.y - 2.0f <= cubeLocation.y + 0.5f;

      // X-axis collision
      if (frontX && !frontZ && !backZ) {
        camera.position.x -= 0.089997f;
        camera.target.y -= 0.089997f;
      }
      else if (backX && !frontZ && !backZ) {
        camera.position.x += 0.089997f;
        camera.target.y -= 0.089997f;
      }

      // Y-axis collision
      float playerBottom = camera.position.y - 2.0f;
      float cubeTop = cubeLocation.y + 0.5f;

      bool insideX = camera.position.x >= cubeLocation.x - 0.5f &&
                      camera.position.x <= cubeLocation.x + 0.5f;

      bool insideZ = camera.position.z >= cubeLocation.z - 0.5f &&
                      camera.position.z <= cubeLocation.z + 0.5f;

      if (insideX && insideZ && playerBottom <= cubeTop + 0.1f && playerBottom >= cubeTop - 0.2f) {
        camera.position.y = cubeTop + 2.0f;
        onGround = true;
        jumpVelocity = 0.0f;
      }
      if (camera.position.y - 2.0f < 0.0f) {
        camera.position.y = 2.0f;
      }

      // Z-axis collision
      if (frontZ && !frontX && !backX) {
        camera.position.z += 0.089997f;
        camera.target.y -= 0.089997f;
      }
      else if (backZ && !frontX && !backX) {
        camera.position.z -= 0.089997f;
        camera.target.y -= 0.089997f;
      }

      // Diagonal collision
      if (frontX && frontZ) {
        camera.position.x += 0.06364f;
        camera.position.z += 0.06364f;
      }
      if (backX && backZ) {
        camera.position.x -= 0.06364f;
        camera.position.z -= 0.06364f;
      }
      if (frontX && backZ) {
        camera.position.x += 0.06364f;
        camera.position.z -= 0.06364f;
      }
      if (backX && frontZ) {
        camera.position.x -= 0.06364f;
        camera.position.z += 0.06364f;
      }
    }
    float playerBottom = camera.position.y - 2.0f;
    float cubeTop = cubeLocation.y + 0.5f;
    
    // Shopkeeper collision
    if (CheckCollisionBoxes(
      BoundingBox{Vector3{ camera.position.x - 0.5f,
                               camera.position.y - 2.0f - 1.0f,
                               camera.position.z - 0.5f },
                    Vector3{ camera.position.x + 0.5f,
                               camera.position.y - 2.0f + 1.0f,
                               camera.position.z + 0.5f }},
      BoundingBox{Vector3{ shopkeeperLocation.x - 0.5f,
                               shopkeeperLocation.y + 1.0f - 1.0f,
                               shopkeeperLocation.z - 0.5f },
                    Vector3{ shopkeeperLocation.x + 0.5f,
                               shopkeeperLocation.y + 1.0f + 1.0f,
                               shopkeeperLocation.z + 0.5f }})) {
      bool frontX = oldCamPos.x < camera.position.x && shopkeeperLocation.x - 0.5f > camera.position.x && oldCamPos.y - 2.0f <= shopkeeperLocation.y + 1.0f;
      bool backX = oldCamPos.x > camera.position.x && shopkeeperLocation.x + 0.5f < camera.position.x && oldCamPos.y - 2.0f <= shopkeeperLocation.y + 1.0f;

      bool frontZ = oldCamPos.z > camera.position.z && shopkeeperLocation.z + 0.5f < camera.position.z && oldCamPos.y - 2.0f <= shopkeeperLocation.y + 1.0f;
      bool backZ = oldCamPos.z < camera.position.z && shopkeeperLocation.z - 0.5f > camera.position.z && oldCamPos.y - 2.0f <= shopkeeperLocation.y + 1.0f;

      // X-axis collision
      if (frontX && !frontZ && !backZ) {
        camera.position.x -= 0.089997f;
        camera.target.y -= 0.089997f;
      }
      else if (backX && !frontZ && !backZ) {
        camera.position.x += 0.089997f;
        camera.target.y -= 0.089997f;
      }

      // Y-axis collision
      float playerBottom = camera.position.y - 2.0f;
      float shopkeeperTop = shopkeeperLocation.y + 1.0f;

      bool insideX = camera.position.x >= shopkeeperLocation.x - 0.5f &&
                      camera.position.x <= shopkeeperLocation.x + 0.5f;

      bool insideZ = camera.position.z >= shopkeeperLocation.z - 0.5f &&
                      camera.position.z <= shopkeeperLocation.z + 0.5f;

      if (insideX && insideZ && playerBottom <= shopkeeperTop + 0.1f && playerBottom >= shopkeeperTop) {
        camera.position.y = shopkeeperTop + 2.0f;
        onGround = true;
        jumpVelocity = 0.0f;
      }
      if (camera.position.y - 2.0f < 0.0f) {
        camera.position.y = 2.0f;
      }

      // Z-axis collision
      if (frontZ && !frontX && !backX) {
        camera.position.z += 0.089997f;
        camera.target.y -= 0.089997f;
      }
      else if (backZ && !frontX && !backX) {
        camera.position.z -= 0.089997f;
        camera.target.y -= 0.089997f;
      }

      // Diagonal collision
      if (frontX && frontZ) {
        camera.position.x += 0.06364f;
        camera.position.z += 0.06364f;
      }
      if (backX && backZ) {
        camera.position.x -= 0.06364f;
        camera.position.z -= 0.06364f;
      }
      if (frontX && backZ) {
        camera.position.x += 0.06364f;
        camera.position.z -= 0.06364f;
      }
      if (backX && frontZ) {
        camera.position.x -= 0.06364f;
        camera.position.z += 0.06364f;
      }
    }
    // Gravity
    float shopkeeperTop = shopkeeperLocation.y + 1.0f;
    if (!onGround) {
      camera.position.y -= 0.05;
    }

    // Updating bounding boxes
    BoundingBox cubeBox = cube.aabb;
    cubeBox.min = Vector3Add(cubeBox.min, Vector3{0, cubeSize / 2, 0});
    cubeBox.max = Vector3Add(cubeBox.max, Vector3{0, cubeSize / 2, 0});

    BoundingBox shopBox = shopkeeper.aabb;
    shopBox.min = Vector3Add(shopBox.min, Vector3{5, 1, 5});
    shopBox.max = Vector3Add(shopBox.max, Vector3{5, 1, 5});

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

    // Placing Blocks
    PlaceBlocks(camera, maxBlockDistance, blockData, blockSize, blocks);
    
    // Breaking Blocks
    BreakBlocks(camera, maxBlockDistance, blockData, blockSize, blocks);

    // Drawing
    BeginDrawing();
    // R3D Drawing
    R3D_Begin(camera);
    R3D_DrawMesh(plane, planeMaterial, Vector3{0, 0, 0}, 1.0f);
    R3D_DrawMesh(cube, cubeMaterial, cubeLocation, 1.0f);
    R3D_DrawMesh(shopkeeper, shopMaterial, shopkeeperLocation, 1.0f);
    for (auto &pos_index : blockData) {
      R3D_DrawMesh(block, blockMaterial,
                   pos_index.pos,
                   (float)blockSize);
    }
    R3D_End();

    BeginMode3D(camera);
    Vector2 screenCenter = {400, 300};
    Ray blockPos = GetMouseRay(screenCenter, camera);
    RayCollision blockPosPlane = GetRayCollisionQuad(
        blockPos, Vector3{-1000, 0, 1000}, Vector3{1000, 0, 1000},
        Vector3{1000, 0, -1000}, Vector3{-1000, 0, -1000});
    DrawBlocks(camera, maxBlockDistance, blockData, blockSize, blockPosPlane);
    EndMode3D();

    // Crosshair
    DrawLine(GetScreenWidth()/2, (GetScreenHeight()/2)-10, GetScreenWidth()/2, (GetScreenHeight()/2)+10, GRAY);
    DrawLine((GetScreenWidth()/2)-10, GetScreenHeight()/2, (GetScreenWidth()/2)+10, GetScreenHeight()/2, GRAY);

    // Coins text
    DrawText(TextFormat("Coins: $%d", coins), 10, 10, 30, BLACK);

    // Blocks text
    DrawText(TextFormat("Blocks: %d", blocks), 10, 60, 30, BLACK);

    // Shop menu logic
    DrawShopMenu(openShopMenu, coins, framesTarget, menu, doubleClicker, clickerVal, autoClicker, autoClickerVal, pauseMenu, blocks, canJump);

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
