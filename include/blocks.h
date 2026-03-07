#pragma once

void blocksPlacement(Camera3D camera, float maxBlockDistance, std::vector<BLOCKS> &blockData, int blockSize);
void drawBlocks(Camera3D camera, float maxBlockDistance, std::vector<BLOCKS> blockData, int blockSize, RayCollision blockPosPlane);
