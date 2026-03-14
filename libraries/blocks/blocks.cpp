#include <iostream>
#include "r3d/r3d.h"
#include "raylib.h"
#include "raymath.h"
#include <vector>
#include <algorithm>

struct BLOCKS {
  Vector3 pos;
};

void PlaceBlocks(Camera3D camera, float maxBlockDistance, std::vector<BLOCKS> &blockData, int blockSize) {
  Vector2 screenCenter = {400, 300};
  Ray blockPos = GetMouseRay(screenCenter, camera);

  BLOCKS* closestBlock = nullptr;
  float closestDistance = maxBlockDistance;
  RayCollision closestHit;

  for (auto &block : blockData) {
      BoundingBox bb{
          Vector3{block.pos.x - blockSize / 2.0f,
                  block.pos.y - blockSize / 2.0f,
                  block.pos.z - blockSize / 2.0f},
          Vector3Add(block.pos, Vector3{blockSize / 2.0f, blockSize / 2.0f, blockSize / 2.0f})
      };
      RayCollision hit = GetRayCollisionBox(blockPos, bb);
      if (hit.hit && hit.distance < closestDistance) {
          closestDistance = hit.distance;
          closestBlock = &block;
          closestHit = hit;
      }
  }

  RayCollision blockPosPlane = GetRayCollisionQuad(
      blockPos, Vector3{-1000, 0, 1000}, Vector3{1000, 0, 1000},
      Vector3{1000, 0, -1000}, Vector3{-1000, 0, -1000});

  for (auto &blockPosition : blockData) {
    BoundingBox bb = BoundingBox{
        Vector3{blockPosition.pos.x - blockSize / 2.0f,
                         blockPosition.pos.y - blockSize / 2.0f,
                         blockPosition.pos.z - blockSize / 2.0f},
        Vector3Add(
            blockPosition.pos,
            Vector3{blockSize / 2.0f, blockSize / 2.0f, blockSize / 2.0f})};
      RayCollision blockPosBlock = GetRayCollisionBox(blockPos, bb);
      float maxDistance = fminf(blockPosBlock.distance, maxBlockDistance);
    if (blockPosBlock.hit && blockPosBlock.distance <= maxDistance) {
      maxDistance = blockPosBlock.distance;
      if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && closestBlock) {
        Vector3 newPos = Vector3Add(blockPosition.pos, Vector3Scale(blockPosBlock.normal, (float)blockSize));
        for (auto &blocks : blockData) {
          if (blocks.pos.x == newPos.x && blocks.pos.y == newPos.y && blocks.pos.z == newPos.z) {
            Vector3Add(newPos, blockPosBlock.normal);
          }
        }
        blockData.push_back(BLOCKS{newPos.x, newPos.y, newPos.z});
        return;
      }
    }
  }
  float maxDistance = fminf(blockPosPlane.distance, maxBlockDistance);
  if (blockPosPlane.hit && blockPosPlane.distance <= maxDistance) {
    Vector3 point = blockPosPlane.point;
    int x = std::round(point.x / blockSize) * blockSize;
    int y = std::round(point.y / blockSize) * blockSize;
    int z = std::round(point.z / blockSize) * blockSize;

    if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
      BLOCKS pos = BLOCKS{(float)x, (float)y + 0.5f, (float)z};
      blockData.push_back(pos);
      return;
    }
  }
}

void DrawBlocks(Camera3D camera, float maxBlockDistance, std::vector<BLOCKS> blockData, int blockSize, RayCollision blockPosPlane) {
  Vector2 screenCenter = {400, 300};
  Ray blockPos = GetMouseRay(screenCenter, camera);

  for (auto &blockPosition : blockData) {
    BoundingBox bb = BoundingBox{
        Vector3{blockPosition.pos.x - blockSize / 2.0f,
                         blockPosition.pos.y - blockSize / 2.0f,
                         blockPosition.pos.z - blockSize / 2.0f},
        Vector3Add(
            blockPosition.pos,
            Vector3{blockSize / 2.0f, blockSize / 2.0f, blockSize / 2.0f})};
    RayCollision blockPosBlock = GetRayCollisionBox(blockPos, bb);

    if (blockPosBlock.hit && blockPosBlock.distance == maxBlockDistance) {
      Vector3 blockNormal = blockPosBlock.normal;
      Vector3 oldPos = blockPosition.pos;
      Vector3 newPos = Vector3Add(oldPos, blockNormal);
      DrawCubeWires(oldPos, 1.0f, 1.0f, 1.0f, BLACK);
    }
    if (blockPosPlane.hit && blockPosPlane.distance <= maxBlockDistance && !blockPosBlock.hit) {
          Vector3 point = blockPosPlane.point;
          int x = std::round(point.x / blockSize) * blockSize;
          int y = std::round(point.y / blockSize) * blockSize;
          int z = std::round(point.z / blockSize) * blockSize;

          Vector3 pos = Vector3{(float)x, (float)y - 0.49f, (float)z};
          DrawCubeWires(pos, 1.0f, 1.0f, 1.0f, BLACK);
      }
  }
}

void BreakBlocks(Camera3D camera, float maxBlockDistance, std::vector<BLOCKS> &blockData, int blockSize) {
  Vector2 screenCenter = {400, 300};
  Ray blockPos = GetMouseRay(screenCenter, camera);

  BLOCKS* closestBlock = nullptr;
  float closestDistance = maxBlockDistance;
  
  RayCollision blockPosPlane = GetRayCollisionQuad(
    blockPos, Vector3{-1000, 0, 1000}, Vector3{1000, 0, 1000},
    Vector3{1000, 0, -1000}, Vector3{-1000, 0, -1000});

  for (auto &block : blockData) {
    BoundingBox bb{
        Vector3{block.pos.x - blockSize / 2.0f,
                block.pos.y - blockSize / 2.0f,
                block.pos.z - blockSize / 2.0f},
        Vector3Add(block.pos, Vector3{blockSize / 2.0f, blockSize / 2.0f, blockSize / 2.0f})
    };

    RayCollision hit = GetRayCollisionBox(blockPos, bb);
    if (hit.hit && hit.distance < closestDistance) {
        closestDistance = hit.distance;
        closestBlock = &block;
    }
  }

  for (auto &blockPosition : blockData) {
    BoundingBox bb = BoundingBox{
        Vector3{blockPosition.pos.x - blockSize / 2.0f,
                        blockPosition.pos.y - blockSize / 2.0f,
                        blockPosition.pos.z - blockSize / 2.0f},
        Vector3Add(
            blockPosition.pos,
            Vector3{blockSize / 2.0f, blockSize / 2.0f, blockSize / 2.0f})};
    RayCollision blockPosBlock = GetRayCollisionBox(blockPos, bb);
    float maxDistance = fminf(blockPosBlock.distance, maxBlockDistance);

    if (blockPosBlock.hit && blockPosBlock.distance <= maxDistance) {
      // WARNING: May interfere with previous block placements!!
      maxDistance = blockPosBlock.distance;

      if (closestBlock && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        blockData.erase(
            std::remove_if(
                blockData.begin(),
                blockData.end(),
                [&](const BLOCKS& b) { return b.pos.x == closestBlock->pos.x &&
                                             b.pos.y == closestBlock->pos.y &&
                                             b.pos.z == closestBlock->pos.z; })
        , blockData.end());
      }
    }
  }
}
