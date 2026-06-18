#include "r3d/r3d.h"
#include <raymath.h>
#include <vector>
#include "menu/menu.h"
#include <iostream>
#include <cmath>
#include <algorithm>

#include "blocks/blocks.h"

void MovePosForward(Camera3D camera, Vector3& position, float speed);

struct ENEMIES {
  Vector3 pos;
  int health;
};

int main() {
  // ----- SETTING UP THE SCREEN -----
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(800, 600, "Build alpha-0.0.3-test-2");
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
  bool isShopMoved = false;
  Vector2 shopToMouseOffset;
  Vector2 shopPos = {30, 30};
  Vector3 shopSize = {740, 540};
  Vector3 zombiePos = {5, 1, -5};
  float zombieHealth = 10;
  float zombieKBTime = 0.0f;
  bool isZombieHit = false;
  int playerHealth = 10;
  int zombieHitCooldown = 0.0;
  bool shouldZombieMove = true;
  float playerHitCooldown = 0.0f;
  int cooldownIndicatorX = -50;
  int perfectHitStartX = GetRandomValue(60, 70);
  int perfectHitEndX = GetRandomValue(90, 100);
  bool perfectHit = false;
  Vector2 screenCenter = {float(GetScreenWidth())/2, float(GetScreenHeight())/2};
  bool onCooldown = false;
  bool firstHit = false;
  std::vector<ENEMIES> enemyData = {{{5, 1, -5}, 10}, {{10, 1, 10}, 10}};

  // ----- MESHES -----

  // Generating meshes
  R3D_Mesh plane = R3D_GenMeshPlane(1000, 1000, 1, 1);
  R3D_Mesh cube = R3D_GenMeshCube(cubeSize, cubeSize, cubeSize);
  R3D_Mesh shopkeeper = R3D_GenMeshCube(1, 2, 1);
  R3D_Mesh block = R3D_GenMeshCube(cubeSize, cubeSize, cubeSize);
  R3D_Mesh zombieMesh = R3D_GenMeshCube(1, 2, 1);

  // Textures
  R3D_Material cubeMaterial = R3D_GetDefaultMaterial();
  cubeMaterial.albedo.texture = LoadTexture("../assets/cubeTexture.png");

  R3D_Material blockMaterial = R3D_GetDefaultMaterial();
  blockMaterial.albedo.texture = LoadTexture("../assets/brickTexture.png");

  R3D_Material planeMaterial = R3D_GetDefaultMaterial();
  planeMaterial.albedo.texture = LoadTexture("../assets/planeTexture.png");

  R3D_Material shopMaterial = R3D_GetDefaultMaterial();

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

    if (IsWindowResized()) R3D_SetResolution(GetScreenWidth(), GetScreenHeight());

    Vector3 oldCamPos = camera.position;

    perfectHit = false;

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
    Vector3 closestEnemy = {999999.0f, 999999.0f, 9999999.0f};
    for (auto &block : blockData) {
      if (sqrt(std::pow(block.pos.x-camera.position.x, 2) + std::pow(block.pos.y-camera.position.y, 2) + std::pow(block.pos.z-camera.position.z, 2)) < sqrt(std::pow(closestEnemy.x-camera.position.x, 2) + std::pow(closestEnemy.y-camera.position.y, 2) + std::pow(closestEnemy.z-camera.position.z, 2))) {
        closestEnemy = block.pos;
      }
    }
    if (CheckCollisionBoxes(
        BoundingBox{Vector3{ camera.position.x - 0.5f,
                                 camera.position.y - 2.0f - 1.0f,
                                 camera.position.z - 0.5f },
                      Vector3{ camera.position.x + 0.5f,
                                 camera.position.y - 2.0f + 1.0f,
                                 camera.position.z + 0.5f }},
        BoundingBox{Vector3{ closestEnemy.x - 0.5f,
                                 closestEnemy.y - 0.5f,
                                 closestEnemy.z - 0.5f },
                      Vector3{ closestEnemy.x + 0.5f,
                                 closestEnemy.y + 0.5f,
                                 closestEnemy.z + 0.5f }})) {
        bool frontX = oldCamPos.x < camera.position.x && closestEnemy.x - 0.5f > camera.position.x && oldCamPos.y - 2.0f <= closestEnemy.y + 0.5f;
        bool backX = oldCamPos.x > camera.position.x && closestEnemy.x + 0.5f < camera.position.x && oldCamPos.y - 2.0f <= closestEnemy.y + 0.5f;

        bool frontZ = oldCamPos.z > camera.position.z && closestEnemy.z + 0.5f < camera.position.z && oldCamPos.y - 2.0f <= closestEnemy.y + 0.5f;
        bool backZ = oldCamPos.z < camera.position.z && closestEnemy.z - 0.5f > camera.position.z && oldCamPos.y - 2.0f <= closestEnemy.y + 0.5f;

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
        float blockTop = closestEnemy.y + 0.5f;

        bool insideX = camera.position.x >= closestEnemy.x - 0.5f &&
                          camera.position.x <= closestEnemy.x + 0.5f;

        bool insideZ = camera.position.z >= closestEnemy.z - 0.5f &&
                        camera.position.z <= closestEnemy.z + 0.5f;

        if (insideX && insideZ && playerBottom <= blockTop + 0.1f && playerBottom >= blockTop - 0.2f) {
          camera.position.y = blockTop + 2.0f;
          onGround = true;
          jumpVelocity = 0.0f;
        }
        else {
          onGround = false;
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
        playerBottom = camera.position.y - 2.0f;
        blockTop = closestEnemy.y + 0.5f;
        if (round(playerBottom) != blockTop && playerBottom > 0) {
          onGround = false;
        }
        if (camera.position.y - 2.0f < 0) camera.position.y = 2.0f;
      }

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
    shopBox.min = Vector3Add(shopBox.min, shopkeeperLocation);
    shopBox.max = Vector3Add(shopBox.max, shopkeeperLocation);

    // Click detection
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      Vector2 screenCenter = {float(GetScreenWidth())/2, float(GetScreenHeight())/2};
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
    PlaceBlocks(camera, maxBlockDistance, blockData, blockSize, blocks, menu);
    
    // Breaking Blocks
    BreakBlocks(camera, maxBlockDistance, blockData, blockSize, blocks, menu);

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
    // Monster Drawing
    for (auto &enemy : enemyData) {
      float dx_path = enemy.pos.x - camera.position.x;
      float dy_path = enemy.pos.y - camera.position.y;
      float dz_path = enemy.pos.z - camera.position.z;

      float distance_path = sqrtf(dx_path*dx_path + dy_path*dy_path + dz_path*dz_path);

      ENEMIES* closestEnemyToPlayer = nullptr;
      float closestDistance = 99999.0f;

      if (distance_path < closestDistance) {
          closestDistance = distance_path;
          closestEnemyToPlayer = &enemy;
      }
      if (closestEnemyToPlayer->health > 0) R3D_DrawMesh(zombieMesh, planeMaterial, closestEnemyToPlayer->pos, 1.0f);
    }
    R3D_End();

    BeginMode3D(camera);
    Ray blockPos = GetMouseRay(screenCenter, camera);
    RayCollision blockPosPlane = GetRayCollisionQuad(
        blockPos, Vector3{-1000, 0, 1000}, Vector3{1000, 0, 1000},
        Vector3{1000, 0, -1000}, Vector3{-1000, 0, -1000});
    DrawBlocks(camera, maxBlockDistance, blockData, blockSize, blockPosPlane);
    EndMode3D();

    // Crosshair
    Color transGray = Color{ 130, 130, 130, 180 };
    Color transBlue = Color{ 0, 121, 241, 180 };
    DrawLine(GetScreenWidth()/2, (GetScreenHeight()/2)-10, GetScreenWidth()/2, (GetScreenHeight()/2)+10, transGray);
    DrawLine((GetScreenWidth()/2)-10, GetScreenHeight()/2, (GetScreenWidth()/2)+10, GetScreenHeight()/2, transGray);

    // ----- COOLDOWN -----
    if (!menu && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
      onCooldown = true;
    }
    if (onCooldown) {
      // Cooldown bar
      DrawRectangle((GetScreenWidth()/2)-50, (GetScreenHeight()/2)+20, 100, 13, transGray);

      // Cooldown line indicator
      DrawLine((GetScreenWidth()/2)+cooldownIndicatorX, (GetScreenHeight()/2)+20, ((GetScreenWidth()/2)+cooldownIndicatorX), (GetScreenHeight()/2)+33, DARKGRAY);

      // Perfect hit bar
      DrawRectangle(((GetScreenWidth()/2)-50)+perfectHitStartX, (GetScreenHeight()/2)+20, perfectHitEndX-perfectHitStartX, 13, transBlue);
    }

    // Coins text
    DrawText(TextFormat("Coins: $%d", coins), 10, 10, 30, BLACK);

    // Blocks text
    DrawText(TextFormat("Blocks: %d", blocks), 10, 60, 30, BLACK);

    // Zombie Health text
    DrawText(TextFormat("Zombie Health: %.1f", zombieHealth), 10, 110, 30, BLACK);

    // Player Health text
    DrawText(TextFormat("Player Health: %d", playerHealth), 10, 160, 30, BLACK);

    // Shop menu logic
    DrawShopMenu(openShopMenu, coins, framesTarget, menu, doubleClicker, clickerVal, autoClicker, autoClickerVal, pauseMenu, blocks, canJump, isShopMoved, shopToMouseOffset, shopPos, shopSize);

    // Pause menu logic
    DrawPauseMenu(pauseMenu, menu, shouldClose);

    EndDrawing();

    // ----- COOLDOWN -----
    if (!menu && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
      onCooldown = true;
      firstHit = true;
    }
    if (onCooldown) {
      if (cooldownIndicatorX+50 >= perfectHitStartX && cooldownIndicatorX+50 <= perfectHitEndX && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        perfectHit = true;
      }
      // Cooldown bar
      DrawRectangle((GetScreenWidth()/2)-50, (GetScreenHeight()/2)+20, 100, 13, transGray);

      // Cooldown line indicator
      DrawLine((GetScreenWidth()/2)+cooldownIndicatorX, (GetScreenHeight()/2)+20, ((GetScreenWidth()/2)+cooldownIndicatorX), (GetScreenHeight()/2)+33, DARKGRAY);
      if (cooldownIndicatorX >= 50) {
        perfectHitStartX = GetRandomValue(30, 70);
        perfectHitEndX = GetRandomValue(90, 100);
        cooldownIndicatorX = -50;
        onCooldown = false;
      }
      cooldownIndicatorX += 2.5f * GetFrameTime() * 50;

      // Perfect hit bar
      DrawRectangle(((GetScreenWidth()/2)-50)+perfectHitStartX, (GetScreenHeight()/2)+20, perfectHitEndX-perfectHitStartX, 13, transBlue);
    }
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
      if (cooldownIndicatorX+50 >= perfectHitStartX && cooldownIndicatorX+50 <= perfectHitEndX && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        perfectHit = true;
      }
      else {
        cooldownIndicatorX = -50;
      }
    }
    if (cooldownIndicatorX+50 >= perfectHitStartX &&
        cooldownIndicatorX+50 <= perfectHitEndX &&
        IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        perfectHit = true;
    }

    // ----- MONSTERS -----
    // Zombie
    // Delete dead enemies
    enemyData.erase(
      std::remove_if(enemyData.begin(), enemyData.end(),
        [](const ENEMIES& e) {
          return e.health <= 0;
        }),
      enemyData.end()
    );
    
    // Pathfinding to Player
    ENEMIES* closestEnemyToPlayer = nullptr;
    float closestDistance = 99999.0f;

    for (auto &enemy : enemyData) {
      float dx_path = enemy.pos.x - camera.position.x;
      float dy_path = enemy.pos.y - camera.position.y;
      float dz_path = enemy.pos.z - camera.position.z;

      float distance_path = sqrtf(dx_path*dx_path + dy_path*dy_path + dz_path*dz_path);

      if (distance_path < closestDistance) {
          closestDistance = distance_path;
          closestEnemyToPlayer = &enemy;
      }

      float minDist = 2.5f;

      float dx = camera.position.x - enemy.pos.x;
      float dz = camera.position.z - enemy.pos.z;

      float distance = sqrt(dx*dx + dz*dz);

      if (distance > minDist && !pauseMenu && shouldZombieMove) {
        dx /= distance;
        dz /= distance;

        float speed = 4.0f;

        enemy.pos.x += dx * speed * GetFrameTime();
        enemy.pos.z += dz * speed * GetFrameTime();
      }

      // Knockback

      // Hit by Player
      Vector2 screenCenter = {float(GetScreenWidth())/2, float(GetScreenHeight())/2};
      Ray hit = GetMouseRay(screenCenter, camera);

      BoundingBox zombieBox = zombieMesh.aabb;
      zombieBox.min = Vector3Add(zombieBox.min, closestEnemyToPlayer->pos);
      zombieBox.max = Vector3Add(zombieBox.max, closestEnemyToPlayer->pos);

      RayCollision zombieHit = GetRayCollisionBox(hit, zombieBox);

      if (zombieHit.hit && distance <= 4.0f && !menu && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && playerHitCooldown <= 0.0f) {

        float damage = 1.0f;

        if (perfectHit) {
            damage = 1.5f;
        }

        closestEnemyToPlayer->health -= damage;
        playerHitCooldown = 0.5f;

        cooldownIndicatorX = -50;

        perfectHitStartX = GetRandomValue(25, 40);
        perfectHitEndX = GetRandomValue(45, 100);
      }
      if (zombieHit.hit && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && distance <= 4.0f && !menu && playerHitCooldown >= 0.0f
          && !(cooldownIndicatorX+50 >= perfectHitStartX && cooldownIndicatorX+50 <= perfectHitEndX && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
          || (cooldownIndicatorX+50 > perfectHitEndX)
          && onCooldown) {
        playerHitCooldown = 0.5;
      }

      if (playerHitCooldown > 0.0f){
        playerHitCooldown -= GetFrameTime();

        if (playerHitCooldown < 0.0f) {
          playerHitCooldown = 0.0f;
        }
      }
      if (zombieKBTime < 1.0f && isZombieHit && !pauseMenu) {
        zombieKBTime += 0.2f;
      }
      if (zombieKBTime >= 1.0f) {
        zombieKBTime = 0.0f;
        isZombieHit = false;
      }
      if (zombieHealth < 0) {
        zombieHealth = 0;
      }
      // Hit by Zombie
      if (distance < 2.8f && zombieHitCooldown == 0.0f && zombieHealth > 0 && !pauseMenu) {
        playerHealth--;
        zombieHitCooldown += 1.0;
      }
      if (zombieHitCooldown > 0.0f && distance <= 2.8f) {
        zombieHitCooldown += 1.0;
        shouldZombieMove = false;
      }
      if (distance > 2.8f) shouldZombieMove = true;
      if (zombieHitCooldown >= GetFPS()) {
        zombieHitCooldown = 0.0f;
        shouldZombieMove = true;
      }
      if (playerHealth <= 0) {
        shouldClose = false;
      }
    }

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
    firstHit = false;
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
void MovePosForward(Camera3D camera, Vector3& position, float speed) {
    Vector3 forward = Vector3Normalize(Vector3Subtract(camera.position, position));
    forward = Vector3Scale(forward, speed * GetFrameTime());

    position = Vector3Subtract(position, forward);
    position.y = 1.0f;
}
