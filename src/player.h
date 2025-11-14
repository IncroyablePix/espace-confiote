#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"

typedef struct MonsterView // read-only data exposed to player AI
{
    Vector2 position;
    float size;
    float health;
    bool invincible;
} MonsterView;


typedef struct PlayerContext // limited information exposed to player code
{
    Vector2 playerPosition;
    Vector2 playerVelocity;
    float playerRotation;
    const MonsterView* visibleMonsters; // Array of monster views
    int visibleMonsterCount;
    float deltaTime;
} PlayerContext;

typedef struct Player 
{
    Vector2 position;
    Vector2 velocity;
    float rotation;
    float size;
    Texture2D texture;
    Texture2D explosionTexture;
    Color fallbackColor;
    float shootCooldown;
    float shootTimer;
    float thrustPower;
    float turnRate;
    float maxSpeed;
    float health;
    bool isDead;
    float explosionTimer;
    int explosionFrame;
} Player;

typedef enum ShootDirection 
{
    SHOOT_FORWARD,
    SHOOT_LEFT,
    SHOOT_RIGHT
} ShootDirection;

// API functions available to player code
void PlayerShoot(ShootDirection direction);
void PlayerSetThrusters(float leftPower, float rightPower); // -1.0 to 1.0
Vector2 PlayerGetPosition(void);
Vector2 PlayerGetMonsterPosition(void);

// Function that user implements
void OnPlayerUpdate(PlayerContext context);

// Internal functions (not for player code)
void InitPlayer(Player* player, Vector2 position, float size);
void UpdatePlayer(Player* player, float deltaTime);
void BeginPlayerAI(Player* player, float deltaTime);
void EndPlayerAI(void);
void DrawPlayer(const Player* player);
void CleanupPlayer(const Player* player);
Rectangle GetPlayerHitbox(const Player* player);
bool PlayerDidShoot(ShootDirection* outDirection);
bool IsPlayerDead(const Player* player);
bool IsPlayerFullyExploded(const Player* player);

#endif // PLAYER_H
