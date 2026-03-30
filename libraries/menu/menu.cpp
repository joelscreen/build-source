#include "r3d/r3d.h"
#include "raylib.h"
#include "raymath.h"
#include <cmath>
#include <algorithm>

#include <iostream>

void DrawShopMenu(bool &openShopMenu, int &coins, int &framesTarget, bool &menu, bool &doubleClicker, int &clickerVal, bool &autoClicker, int &autoClickerVal, bool &pauseMenu, int &blocks, bool &canJump, bool &isShopMoved, Vector2 &shopToMouseOffset, Vector2 &shopPos, Vector3 &shopSize) {
  if (openShopMenu) {
    canJump = false;
    DrawRectangle(shopPos.x, shopPos.y, shopSize.x, shopSize.y, GRAY); // shop menu rectangle
    int items_y = shopPos.y + 30;
    for (int o = 0; o < 3; o++) {
      for (int i = 0; i <= 4; i++) {
        DrawRectangle((i * 145) + shopPos.x + 5, items_y, 140, 135, DARKGRAY);
        DrawCircle((i * 145) + shopPos.x + 80, items_y + 35, 30, YELLOW);
      }
      items_y += 140;
    }
    if (coins < 100) {
      DrawText("Double clicker", shopPos.x + 7, shopPos.y + 110, 20, GRAY);
      DrawText("$100", shopPos.x + 55, shopPos.y + 130, 20, GRAY);
    } else {
      DrawText("Double clicker", shopPos.x + 7, shopPos.y + 110, 20, BLACK);
      DrawText("$100", shopPos.x + 55, shopPos.y + 130, 20, BLACK);
    }
    if (coins < 200 || framesTarget == 1) {
      DrawText("Auto clicker", shopPos.x + 160, shopPos.y + 110, 20, GRAY);
      DrawText("$200", shopPos.x + 195, shopPos.y + 130, 20, GRAY);
    }
    else {
      DrawText("Auto clicker", shopPos.x + 160, shopPos.y + 110, 20, BLACK);
      DrawText("$200", shopPos.x + 195, shopPos.y + 130, 20, BLACK);
    }
    if (coins < 50) {
      DrawText("Bricks", shopPos.x + 333, shopPos.y + 110, 20, GRAY);
      DrawText("$50", shopPos.x + 345, shopPos.y + 130, 20, GRAY);
    }
    else {
      DrawText("Bricks", shopPos.x + 333, shopPos.y + 110, 20, BLACK);
      DrawText("$50", shopPos.x + 345, shopPos.y + 130, 20, BLACK);
    }
    if (IsKeyDown(KEY_ESCAPE)) {
      openShopMenu = false;
      menu = false;
      DisableCursor();
      canJump = true;
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
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && GetMouseX() > 325 &&
        GetMouseX() < 465 && GetMouseY() > 60 && GetMouseY() < 195 &&
        coins >= 50) {
      coins -= 50;
      blocks++;
    }
  } else if (IsKeyPressed(KEY_ESCAPE) && !openShopMenu && !pauseMenu) {
    pauseMenu = true;
    menu = true;
    EnableCursor();
  } else if (IsKeyPressed(KEY_ESCAPE) && !openShopMenu && pauseMenu) {
    pauseMenu = false;
    menu = false;
    DisableCursor();
    canJump = true;
  }
  if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !isShopMoved && openShopMenu && GetMouseX() > shopPos.x && GetMouseY() > shopPos.y && GetMouseX() < shopSize.x + shopPos.x && GetMouseY() < shopSize.y + shopPos.y) {
    shopToMouseOffset = Vector2Subtract(GetMousePosition(), shopPos);
    isShopMoved = true;
  }
  if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
    isShopMoved = false;
  }
  if (isShopMoved) {
    shopPos = Vector2Subtract(GetMousePosition(), shopToMouseOffset);
  }
}

void DrawPauseMenu(bool &pauseMenu, bool &menu, bool &shouldClose) {
  if (pauseMenu) {
    // Back to game
    DrawRectangle((GetScreenWidth()/2) - 200, (GetScreenHeight()/2) - 120, 400, 70, GRAY);
    DrawText("Back to game", (GetScreenWidth()/2) - 100, (GetScreenHeight()/2) - 100, 30, BLACK);
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      if (GetMouseX() > (GetScreenWidth()/2) - 200 && GetMouseX() < ((GetScreenWidth()/2) - 200) + 400) {
        if (GetMouseY() > (GetScreenHeight()/2) - 120 && GetMouseY() < ((GetScreenHeight()/2) - 120) + 70) {
          pauseMenu = false;
          menu = false;
          DisableCursor();
        }
      }
    }
    // Quit game
    DrawRectangle((GetScreenWidth()/2) - 200, (GetScreenHeight()/2) + 80, 400, 70, GRAY);
    DrawText("Quit game", (GetScreenWidth()/2) - 100, (GetScreenHeight()/2) + 100, 30, BLACK);
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      if (GetMouseX() > (GetScreenWidth()/2) - 200 && GetMouseX() < ((GetScreenWidth()/2) - 200) + 400) {
        if (GetMouseY() > (GetScreenHeight()/2) + 80 && GetMouseY() < ((GetScreenHeight()/2) + 80) + 70) {
          shouldClose = false;
        }
      }
    }
  }
}
