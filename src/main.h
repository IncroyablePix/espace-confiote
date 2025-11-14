#ifndef MAIN_H
#define MAIN_H

#include "raylib.h"
#include "monster.h"
#include "player.h"
#include "config.h"
#include "player_ai_backend/player_ai.h"

typedef enum ProjectileOwner 
{
    PROJECTILE_PLAYER,
    PROJECTILE_MONSTER,
    PROJECTILE_MONSTER_SPREAD
} ProjectileOwner;

typedef struct Projectile 
{
    Vector2 position;
    Vector2 velocity;
    float size;
    Color color;
    bool active;
    ProjectileOwner owner;
    int monsterIndex;
} Projectile;

typedef struct Star
{
    Vector2 position;
    int size;
    float phase;
} Star;

typedef struct HitEffect
{
    Vector2 position;
    float timer;
    bool active;
    Color color;
} HitEffect;

typedef struct DeathExplosion
{
    Vector2 position;
    float timer;
    bool active;
    Color color;
    float size;
} DeathExplosion;

typedef struct GameState 
{
    Monster monsters[MONSTER_COUNT];
    Player player;
    
    Projectile* projectiles;
    int projectileCount;
    int projectileCapacity;
    
    Star stars[STAR_COUNT];
    
    HitEffect hitEffects[HIT_EFFECT_MAX_COUNT];
    DeathExplosion deathExplosions[MONSTER_COUNT + 1];
    
    Texture2D sharedMonsterTexture;

    int score;
    float gameTime;
} GameState;

#endif // MAIN_H
