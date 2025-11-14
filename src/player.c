#include "player.h"

#include <assert.h>

#include "config.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

static Player* g_currentPlayer = NULL;
static float g_deltaTime = 0.0f;
static ShootDirection g_shootDirection = SHOOT_FORWARD;
static bool g_shootRequested = false;
static float g_leftThruster = 0.0f;
static float g_rightThruster = 0.0f;

void InitPlayer(Player* player, Vector2 position, float size)
{
    player->position = position;
    player->velocity = (Vector2){0, 0};
    player->rotation = 0.0f;
    player->size = size;
    player->texture = LoadTexture("resources/player.png");
    player->explosionTexture = (Texture2D){0};
    player->fallbackColor = YELLOW;
    player->shootCooldown = PLAYER_SHOOT_COOLDOWN;
    player->shootTimer = 0.0f;
    player->thrustPower = PLAYER_THRUST_POWER;
    player->turnRate = PLAYER_TURN_RATE;
    player->maxSpeed = PLAYER_MAX_SPEED;
    player->health = 1.0f;
    player->isDead = false;
    player->explosionTimer = 0.0f;
    player->explosionFrame = 0;
}

void UpdatePlayer(Player* player, float deltaTime)
{
    if (player->health <= 0.0f && !player->isDead)
    {
        player->isDead = true;
        player->explosionTimer = 0.0f;
        player->explosionFrame = 0;
    }
    
    if (player->isDead)
    {
        player->explosionTimer += deltaTime;
        player->explosionFrame = (int)(player->explosionTimer / EXPLOSION_FRAME_DURATION);
        if (player->explosionFrame >= EXPLOSION_FRAME_COUNT)
            player->explosionFrame = EXPLOSION_FRAME_COUNT - 1;

        return;
    }
    
    if (player->shootTimer > 0.0f)
        player->shootTimer -= deltaTime;

    const float avgThrust = (g_leftThruster + g_rightThruster) / 2.0f;
    const float thrustDiff = g_rightThruster - g_leftThruster;
    
    player->rotation += thrustDiff * player->turnRate * deltaTime;

    const float thrustAngleRad = player->rotation * DEG2RAD;
    const float thrustX = cosf(thrustAngleRad) * avgThrust * player->thrustPower;
    const float thrustY = sinf(thrustAngleRad) * avgThrust * player->thrustPower;
    
    player->velocity.x += thrustX * deltaTime;
    player->velocity.y += thrustY * deltaTime;
    
    player->velocity.x *= PLAYER_DRAG;
    player->velocity.y *= PLAYER_DRAG;

    const float speed = sqrtf(player->velocity.x * player->velocity.x +
                       player->velocity.y * player->velocity.y);
    if (speed > player->maxSpeed)
    {
        player->velocity.x = (player->velocity.x / speed) * player->maxSpeed;
        player->velocity.y = (player->velocity.y / speed) * player->maxSpeed;
    }
    
    player->position.x += player->velocity.x * deltaTime;
    player->position.y += player->velocity.y * deltaTime;

    const int screenWidth = GetScreenWidth();
    const int screenHeight = GetScreenHeight();
    
    if (player->position.x < 0)
        player->position.x = (float)screenWidth;
    else if (player->position.x > (float)screenWidth)
        player->position.x = 0;
    
    if (player->position.y < 0)
        player->position.y = (float)screenHeight;
    else if (player->position.y > (float)screenHeight)
        player->position.y = 0;
}

void BeginPlayerAI(Player* player, float deltaTime)
{
    g_currentPlayer = player;
    g_deltaTime = deltaTime;
    g_shootRequested = false;
}

void EndPlayerAI(void)
{
    g_currentPlayer = NULL;
}

bool PlayerDidShoot(ShootDirection* outDirection)
{
    if (g_shootRequested && outDirection)
    {
        *outDirection = g_shootDirection;
        return true;
    }
    return false;
}

void DrawPlayer(const Player* player)
{
    if (player->isDead)
    {
        if (player->explosionTexture.id != 0 && player->explosionFrame < EXPLOSION_FRAME_COUNT)
        {
            const int frameWidth = player->explosionTexture.width / EXPLOSION_FRAME_COUNT;
            const int frameHeight = player->explosionTexture.height;

            const Rectangle source =
            {
                (float)(player->explosionFrame * frameWidth),
                0,
                (float)frameWidth,
                (float)frameHeight
            };

            const Rectangle dest =
            {
                player->position.x,
                player->position.y,
                player->size * 2.0f,
                player->size * 2.0f
            };

            const Vector2 origin = { player->size, player->size };
            
            float explosionOpacity = 1.0f;
            if (player->explosionTimer > EXPLOSION_FADE_START_TIME)
            {
                const float fadeProgress = (player->explosionTimer - EXPLOSION_FADE_START_TIME) / EXPLOSION_FADE_DURATION;
                explosionOpacity = 1.0f - fadeProgress;
                if (explosionOpacity < 0.0f)
                    explosionOpacity = 0.0f;
            }

            const Color tint = (Color){255, 255, 255, (unsigned char)(explosionOpacity * 255)};
            DrawTexturePro(player->explosionTexture, source, dest, origin, 0, tint);
        }
        return;
    }
    
    if (player->texture.id != 0)
    {
        const float playerRotationAngle = player->rotation + 90.0f;

        const float playerAspectRatio = (float)player->texture.width / (float)player->texture.height;
        const float playerScaledHeight = player->size;
        const float playerScaledWidth = playerScaledHeight * playerAspectRatio;

        const Rectangle playerSource = { 0, 0, (float)player->texture.width, (float)player->texture.height };
        const Rectangle playerDest =
        {
            player->position.x, 
            player->position.y, 
            playerScaledWidth, 
            playerScaledHeight 
        };
        const Vector2 playerOrigin = { playerScaledWidth / 2, playerScaledHeight / 2 };
        
        DrawTexturePro(player->texture, playerSource, playerDest, playerOrigin, playerRotationAngle, WHITE);
    }
    else
    {
        DrawRectangle(
            (int)(player->position.x - player->size / 2),
            (int)(player->position.y - player->size / 2),
            (int)player->size,
            (int)player->size,
            player->fallbackColor
        );
    }
}

void CleanupPlayer(const Player* player)
{
    UnloadTexture(player->texture);
    UnloadTexture(player->explosionTexture);
}

Rectangle GetPlayerHitbox(const Player* player)
{
    return (Rectangle)
    {
        player->position.x - player->size / 2,
        player->position.y - player->size / 2,
        player->size,
        player->size
    };
}

void PlayerShoot(ShootDirection direction)
{
    if (g_currentPlayer == NULL)
        return;
    if (g_currentPlayer->isDead)
        return;
    if (g_currentPlayer->shootTimer > 0.0f)
        return;
    
    g_currentPlayer->shootTimer = g_currentPlayer->shootCooldown;
    g_shootRequested = true;
    g_shootDirection = direction;
}

void PlayerSetThrusters(float leftPower, float rightPower)
{
    if (g_currentPlayer == NULL)
    {
        printf("ERROR: g_currentPlayer is NULL, returning early!\n");
        return;
    }

    if (g_currentPlayer->isDead)
        return;
    
    if (leftPower < -1.0f)
        leftPower = -1.0f;
    if (leftPower > 1.0f)
        leftPower = 1.0f;
    if (rightPower < -1.0f)
        rightPower = -1.0f;
    if (rightPower > 1.0f)
        rightPower = 1.0f;
    
    g_leftThruster = leftPower;
    g_rightThruster = rightPower;
}

Vector2 PlayerGetPosition(void)
{
    if (g_currentPlayer == NULL)
        return (Vector2){0, 0};
    return g_currentPlayer->position;
}

bool IsPlayerDead(const Player* player)
{
    return player->isDead;
}

bool IsPlayerFullyExploded(const Player* player)
{
    return player->isDead && player->explosionFrame >= (EXPLOSION_FRAME_COUNT - 1);
}
