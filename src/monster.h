#ifndef MONSTER_H
#define MONSTER_H

#include "raylib.h"
#include "player.h"
#include <stdbool.h>

typedef struct Monster 
{
    Vector2 position;
    float size;
    Texture2D texture;
    Texture2D explosionTexture;
    Color fallbackColor;
    float health;
    bool isDead;
    float explosionTimer;
    int explosionFrame;
    float shootCooldown;
    float shootTimer;
    float spreadShotCooldown;
    float spreadShotTimer;
    bool canShoot;
    float invincibilityTimer;
} Monster;

void InitMonster(Monster* monster, Vector2 position, float size);
void UpdateMonster(Monster* monster, float deltaTime);
void DrawMonster(const Monster* monster, Vector2 targetPosition, float opacity);
void CleanupMonster(Monster* monster);
Rectangle GetMonsterHitbox(const Monster* monster);
bool IsMonsterDead(const Monster* monster);
bool IsMonsterFullyExploded(const Monster* monster);
void DamageMonster(Monster* monster, float damage);
bool IsMonsterVisible(const Monster* monster, const Player* player);

#endif // MONSTER_H
