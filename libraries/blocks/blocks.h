#pragma once
#include "raylib.h"
#include <vector>

struct BLOCKS {
  Vector3 pos;
};

void PlaceBlocks(Camera3D camera, float maxBlockDistance, std::vector<BLOCKS> &blockData, int blockSize, int &locks, bool menu);
void DrawBlocks(Camera3D camera, float maxBlockDistance, std::vector<BLOCKS> blockData, int blockSize, RayCollision blockPosPlane);
void BreakBlocks(Camera3D camera, float maxBlockDistance, std::vector<BLOCKS> &blockData, int blockSize, int &blocks, bool menu);
