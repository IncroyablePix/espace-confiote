#ifndef UI_H
#define UI_H

#include "raylib.h"

typedef struct GameState GameState;

void DrawUI(GameState* game);
void DrawStars(const GameState* game);

#endif // UI_H
