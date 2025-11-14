#include "monster.h"
#include "config.h"
#include <math.h>

void InitMonster(Monster* monster, const Vector2 position, const float size)
{
    monster->position = position;
    monster->size = size;

    monster->texture = (Texture2D){0};
    monster->explosionTexture = (Texture2D){0};
    monster->fallbackColor = RED;
    
    // Health scales with size
    const float sizeRatio = (size - MONSTER_MIN_SIZE) / (MONSTER_MAX_SIZE - MONSTER_MIN_SIZE);
    monster->health = 0.5f + (sizeRatio * 0.5f); // Health from 0.5 to 1.0 based on size
    
    monster->isDead = false;
    monster->explosionTimer = 0.0f;
    monster->explosionFrame = 0;
    monster->shootCooldown = MONSTER_SHOOT_COOLDOWN;
    monster->shootTimer = 0.0f;
    monster->spreadShotCooldown = MONSTER_SPREAD_SHOT_COOLDOWN;
    monster->spreadShotTimer = MONSTER_SPREAD_SHOT_INITIAL_DELAY;
    monster->canShoot = false; // Default: monsters don't shoot
    monster->invincibilityTimer = 0.0f;
}

void UpdateMonster(Monster* monster, const float deltaTime)
{
    if (monster->health <= 0.0f && !monster->isDead) 
    {
        monster->isDead = true;
        monster->explosionTimer = 0.0f;
        monster->explosionFrame = 0;
    }
    
    if (monster->isDead) 
    {
        monster->explosionTimer += deltaTime;
        monster->explosionFrame = (int)(monster->explosionTimer / EXPLOSION_FRAME_DURATION);
        if (monster->explosionFrame >= EXPLOSION_FRAME_COUNT)
            monster->explosionFrame = EXPLOSION_FRAME_COUNT - 1;
    } 
    else
    {
        if (monster->invincibilityTimer > 0.0f)
            monster->invincibilityTimer -= deltaTime;
        
        if (monster->shootTimer > 0.0f)
            monster->shootTimer -= deltaTime;
        
        if (monster->spreadShotTimer > 0.0f)
            monster->spreadShotTimer -= deltaTime;
    }
}

void DrawMonster(const Monster* monster, const Vector2 targetPosition, const float opacity)
{
    if (monster->isDead)
    {
        if (monster->explosionTexture.id != 0 && monster->explosionFrame < EXPLOSION_FRAME_COUNT)
        {
            const int frameWidth = monster->explosionTexture.width / EXPLOSION_FRAME_COUNT;
            const int frameHeight = monster->explosionTexture.height;

            const Rectangle source =
            {
                (float)(monster->explosionFrame * frameWidth),
                0,
                (float)frameWidth,
                (float)frameHeight
            };

            const Rectangle dest =
            {
                monster->position.x,
                monster->position.y,
                monster->size * 2.0f,
                monster->size * 2.0f
            };

            const Vector2 origin = { monster->size, monster->size };
            
            float explosionOpacity = 1.0f;
            if (monster->explosionTimer > EXPLOSION_FADE_START_TIME)
            {
                const float fadeProgress = (monster->explosionTimer - EXPLOSION_FADE_START_TIME) / EXPLOSION_FADE_DURATION;
                explosionOpacity = 1.0f - fadeProgress;
                if (explosionOpacity < 0.0f) explosionOpacity = 0.0f;
            }

            const Color tint = (Color){255, 255, 255, (unsigned char)(explosionOpacity * 255)};
            DrawTexturePro(monster->explosionTexture, source, dest, origin, 0, tint);
        }
        return;
    }
    
    if (monster->texture.id != 0)
    {
        const float dx = targetPosition.x - monster->position.x;
        const float dy = targetPosition.y - monster->position.y;
        const float rotationAngle = atan2f(dy, dx) * RAD2DEG - 90.0f;

        const float aspectRatio = (float)monster->texture.width / (float)monster->texture.height;
        const float scaledHeight = monster->size;
        const float scaledWidth = scaledHeight * aspectRatio;

        const Rectangle source = { 0, 0, (float)monster->texture.width, (float)monster->texture.height };
        const Rectangle dest =
        {
            monster->position.x, 
            monster->position.y, 
            scaledWidth, 
            scaledHeight 
        };
        const Vector2 origin = { scaledWidth / 2, scaledHeight / 2 };
        
        float finalOpacity = opacity;
        if (monster->invincibilityTimer > 0.0f)
        {
            const float blinkPhase = monster->invincibilityTimer * MONSTER_BLINK_FREQUENCY;
            if (fmodf(blinkPhase, 1.0f) < 0.5f)
                finalOpacity *= 0.3f;
        }

        const Color tint = (Color){255, 255, 255, (unsigned char)(finalOpacity * 255)};
        DrawTexturePro(monster->texture, source, dest, origin, rotationAngle, tint);
    }
    else
    {
        float finalOpacity = opacity;
        if (monster->invincibilityTimer > 0.0f)
        {
            const float blinkPhase = monster->invincibilityTimer * MONSTER_BLINK_FREQUENCY;
            if (fmodf(blinkPhase, 1.0f) < 0.5f)
                finalOpacity *= 0.3f;
        }
        
        Color tintedColor = monster->fallbackColor;
        tintedColor.a = (unsigned char)(finalOpacity * 255);
        DrawRectangle(
            (int)(monster->position.x - monster->size / 2),
            (int)(monster->position.y - monster->size / 2),
            (int)monster->size,
            (int)monster->size,
            tintedColor
        );
    }
}

void CleanupMonster(Monster* monster)
{
    monster->texture = (Texture2D){0};
    monster->explosionTexture = (Texture2D){0};
}

bool IsMonsterDead(const Monster* monster)
{
    return monster->isDead;
}

bool IsMonsterFullyExploded(const Monster* monster)
{
    return monster->isDead && monster->explosionFrame >= (EXPLOSION_FRAME_COUNT - 1);
}

void DamageMonster(Monster* monster, const float damage)
{
    if (!(!monster->isDead && monster->invincibilityTimer <= 0.0f))
    {
        return;
    }

    monster->health -= damage;
    if (monster->health < 0.0f)
        monster->health = 0.0f;
    monster->invincibilityTimer = MONSTER_INVINCIBILITY_DURATION;
}

Rectangle GetMonsterHitbox(const Monster* monster)
{
    return (Rectangle)
    {
        monster->position.x - monster->size / 2,
        monster->position.y - monster->size / 2,
        monster->size,
        monster->size
    };
}

bool IsMonsterVisible(const Monster* monster, const Player* player)
{
    const Vector2 position = monster->position;
    const float dx = position.x - player->position.x;
    const float dy = position.y - player->position.y;
    const float distance = hypotf(dx, dy);
    return distance <= (PLAYER_VISION_RADIUS + monster->size);
}
