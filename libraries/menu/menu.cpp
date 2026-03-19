#include "r3d/r3d.h"
#include "raylib.h"
#include <cmath>
#include <algorithm>

void DrawShopMenu(bool &openShopMenu, int &coins, int &framesTarget, bool &menu, bool &doubleClicker, int &clickerVal, bool &autoClicker, int &autoClickerVal, bool &pauseMenu, int &blocks, bool &canJump) {
    if (openShopMenu) {
      canJump = false;
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
        }
        else {
          DrawText("Auto clicker", 190, 140, 20, BLACK);
          DrawText("$200", 225, 160, 20, BLACK);
        }
        if (coins < 150) {
          DrawText("Bricks", 363, 140, 20, GRAY);
          DrawText("$150", 375, 160, 20, GRAY);
        }
        else {
          DrawText("Bricks", 363, 140, 20, BLACK);
          DrawText("$150", 375, 160, 20, BLACK);
        }
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
          coins >= 150) {
        coins -= 150;
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
}

void DrawPauseMenu(bool &pauseMenu, bool &menu, bool &shouldClose) {
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
          shouldClose = false;
        }
      }
    }
  }
}
