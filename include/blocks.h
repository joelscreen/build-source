#pragma once

void BlocksPlacement(Camera3D camera, float maxBlockDistance, std::vector<BLOCKS> &blockData, int blockSize);
void DrawBlocks(Camera3D camera, float maxBlockDistance, std::vector<BLOCKS> blockData, int blockSize, RayCollision blockPosPlane);
