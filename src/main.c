#include "raylib.h"
#include "main.h"
#include "monster.h"
#include "player.h"
#include "config.h"
#include "ui.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

bool IsPositionSafe(const Vector2 pos, const Monster* monsters, const int monsterCount, const float minDistance)
{
    for (int i = 0; i < monsterCount; i++) 
    {
        const float dx = pos.x - monsters[i].position.x;
        const float dy = pos.y - monsters[i].position.y;
        const float dist = sqrtf(dx * dx + dy * dy);
        if (dist < minDistance + monsters[i].size)
            return false;
    }

    return true;
}

void LoadSharedTextures(GameState* game)
{
    game->sharedMonsterTexture = LoadTexture("resources/boss.png");
}

void InitializeMonsters(GameState* game)
{
    for (int i = 0; i < MONSTER_COUNT; i++)
    {
        const float size = MONSTER_MIN_SIZE + (rand() / (float)RAND_MAX) * (MONSTER_MAX_SIZE - MONSTER_MIN_SIZE);
        
        Vector2 monsterPos;
        int attempts = 0;
        do
        {
            monsterPos.x = size + (rand() / (float)RAND_MAX) * (SCREEN_WIDTH - 2 * size);
            monsterPos.y = size + (rand() / (float)RAND_MAX) * (SCREEN_HEIGHT - 2 * size);
            attempts++;
        } while (attempts < 100 && !IsPositionSafe(monsterPos, game->monsters, i, size * 2));
        
        InitMonster(&game->monsters[i], monsterPos, size);
        game->monsters[i].texture = game->sharedMonsterTexture;
    }
}

void InitializePlayerAtSafePosition(GameState* game)
{
    Vector2 playerPos;
    int attempts = 0;
    do
    {
        playerPos.x = PLAYER_SIZE + (rand() / (float)RAND_MAX) * (SCREEN_WIDTH - 2 * PLAYER_SIZE);
        playerPos.y = PLAYER_SIZE + (rand() / (float)RAND_MAX) * (SCREEN_HEIGHT - 2 * PLAYER_SIZE);
        attempts++;
    } while (attempts < 100 && !IsPositionSafe(playerPos, game->monsters, MONSTER_COUNT, PLAYER_SIZE * 3));
    
    InitPlayer(&game->player, playerPos, PLAYER_SIZE);
}

void InitializeProjectileArray(GameState* game)
{
    game->projectileCapacity = 10;
    game->projectiles = (Projectile*)malloc(sizeof(Projectile) * game->projectileCapacity);
    game->projectileCount = 0;
}

void InitGameState(GameState* game) 
{
    LoadSharedTextures(game);
    InitializeMonsters(game);
    InitializePlayerAtSafePosition(game);
    InitializeProjectileArray(game);
    
    for (int i = 0; i < STAR_COUNT; i++)
    {
        game->stars[i].position = (Vector2){
            (float)GetRandomValue(0, SCREEN_WIDTH),
            (float)GetRandomValue(0, SCREEN_HEIGHT)
        };
        game->stars[i].size = GetRandomValue(STAR_MIN_SIZE, STAR_MAX_SIZE);
        game->stars[i].phase = (float)GetRandomValue(0, 628) / 100.0f;
    }
    
    for (int i = 0; i < HIT_EFFECT_MAX_COUNT; i++)
    {
        game->hitEffects[i].active = false;
    }
    
    for (int i = 0; i < MONSTER_COUNT + 1; i++)
    {
        game->deathExplosions[i].active = false;
    }
    
    game->score = 0;
    game->gameTime = 0.0f;
}

void ExpandProjectileArrayIfNeeded(GameState* game)
{
    if (game->projectileCount >= game->projectileCapacity)
    {
        game->projectileCapacity *= 2;
        game->projectiles = (Projectile*)realloc(game->projectiles, 
            sizeof(Projectile) * game->projectileCapacity);
    }
}

void CreateHitEffect(GameState* game, Vector2 position, Color color)
{
    for (int i = 0; i < HIT_EFFECT_MAX_COUNT; i++)
    {
        if (!game->hitEffects[i].active)
        {
            game->hitEffects[i].position = position;
            game->hitEffects[i].timer = HIT_EFFECT_DURATION;
            game->hitEffects[i].active = true;
            game->hitEffects[i].color = color;
            break;
        }
    }
}

void UpdateHitEffects(GameState* game, const float deltaTime)
{
    for (int i = 0; i < HIT_EFFECT_MAX_COUNT; i++)
    {
        if (game->hitEffects[i].active)
        {
            game->hitEffects[i].timer -= deltaTime;
            if (game->hitEffects[i].timer <= 0.0f)
            {
                game->hitEffects[i].active = false;
            }
        }
    }
}

void DrawHitEffects(GameState* game)
{
    for (int i = 0; i < HIT_EFFECT_MAX_COUNT; i++)
    {
        if (game->hitEffects[i].active)
        {
            const HitEffect* effect = &game->hitEffects[i];
            const float lifeRatio = effect->timer / HIT_EFFECT_DURATION;
            
            for (int p = 0; p < HIT_EFFECT_PARTICLE_COUNT; p++)
            {
                const float angle = (360.0f / HIT_EFFECT_PARTICLE_COUNT) * p;
                const float angleRad = angle * DEG2RAD;
                const float distance = HIT_EFFECT_SIZE * (1.0f - lifeRatio);

                const Vector2 particlePos =
                {
                    effect->position.x + cosf(angleRad) * distance,
                    effect->position.y + sinf(angleRad) * distance
                };
                
                Color particleColor = effect->color;
                particleColor.a = (unsigned char)(255 * lifeRatio);
                
                DrawCircleV(particlePos, 2.0f, particleColor);
            }
        }
    }
}

void CreateDeathExplosion(GameState* game, Vector2 position, Color color, float size)
{
    for (int i = 0; i < MONSTER_COUNT + 1; i++)
    {
        if (!game->deathExplosions[i].active)
        {
            game->deathExplosions[i].position = position;
            game->deathExplosions[i].timer = DEATH_EXPLOSION_DURATION;
            game->deathExplosions[i].active = true;
            game->deathExplosions[i].color = color;
            game->deathExplosions[i].size = size;
            break;
        }
    }
}

void UpdateDeathExplosions(GameState* game, float deltaTime)
{
    for (int i = 0; i < MONSTER_COUNT + 1; i++)
    {
        if (game->deathExplosions[i].active)
        {
            game->deathExplosions[i].timer -= deltaTime;
            if (game->deathExplosions[i].timer <= 0.0f)
            {
                game->deathExplosions[i].active = false;
            }
        }
    }
}

void DrawDeathExplosions(GameState* game)
{
    for (int i = 0; i < MONSTER_COUNT + 1; i++)
    {
        if (game->deathExplosions[i].active)
        {
            const DeathExplosion* explosion = &game->deathExplosions[i];
            const float lifeRatio = explosion->timer / DEATH_EXPLOSION_DURATION;
            
            for (int p = 0; p < DEATH_EXPLOSION_PARTICLE_COUNT; p++)
            {
                const float angle = (360.0f / DEATH_EXPLOSION_PARTICLE_COUNT) * p;
                const float angleRad = angle * DEG2RAD;
                const float distance = explosion->size * (1.0f - lifeRatio);

                const Vector2 particlePos =
                {
                    explosion->position.x + cosf(angleRad) * distance,
                    explosion->position.y + sinf(angleRad) * distance
                };
                
                Color particleColor = explosion->color;
                particleColor.a = (unsigned char)(255 * lifeRatio);

                const float particleSize = 3.0f + (explosion->size / 20.0f);
                DrawCircleV(particlePos, particleSize, particleColor);
            }
        }
    }
}

void ShootProjectile(GameState* game, float angleInDegrees, Vector2 position, ProjectileOwner owner, int monsterIndex)
{
    ExpandProjectileArrayIfNeeded(game);

    const float angleInRadians = angleInDegrees * DEG2RAD;
    const float dx = cosf(angleInRadians);
    const float dy = sinf(angleInRadians);
    
    Projectile* proj = &game->projectiles[game->projectileCount];
    proj->position = position;
    proj->velocity = (Vector2){ dx * PROJECTILE_SPEED, dy * PROJECTILE_SPEED };
    proj->size = PROJECTILE_SIZE;
    
    if (owner == PROJECTILE_PLAYER)
        proj->color = PROJECTILE_PLAYER_COLOR;
    else if (owner == PROJECTILE_MONSTER_SPREAD)
        proj->color = PROJECTILE_MONSTER_SPREAD_COLOR;
    else
        proj->color = PROJECTILE_MONSTER_COLOR;

    proj->active = true;
    proj->owner = owner;
    proj->monsterIndex = monsterIndex;
    
    game->projectileCount++;
}

void RemoveInactiveProjectiles(GameState* game)
{
    int writeIndex = 0;
    for (int i = 0; i < game->projectileCount; i++)
    {
        if (game->projectiles[i].active)
        {
            if (writeIndex != i)
            {
                game->projectiles[writeIndex] = game->projectiles[i];
            }
            writeIndex++;
        }
    }
    game->projectileCount = writeIndex;
}

void UpdateProjectiles(GameState* game, float deltaTime)
{
    const Rectangle playerHitbox = GetPlayerHitbox(&game->player);
    
    for (int i = 0; i < game->projectileCount; i++)
    {
        if (!game->projectiles[i].active)
            continue;
        
        Projectile* proj = &game->projectiles[i];
        
        proj->position.x += proj->velocity.x * deltaTime;
        proj->position.y += proj->velocity.y * deltaTime;
        
        if (proj->position.x < -50 || proj->position.x > SCREEN_WIDTH + 50 ||
            proj->position.y < -50 || proj->position.y > SCREEN_HEIGHT + 50)
        {
            proj->active = false;
            continue;
        }

        const Rectangle projectileRect =
        {
            proj->position.x - proj->size / 2,
            proj->position.y - proj->size / 2,
            proj->size,
            proj->size
        };
        
        if (proj->owner == PROJECTILE_PLAYER)
        {
            for (int m = 0; m < MONSTER_COUNT; m++)
            {
                if (!IsMonsterDead(&game->monsters[m]) && game->monsters[m].invincibilityTimer <= 0.0f && IsMonsterVisible(&game->monsters[m], &game->player))
                {
                    const Rectangle monsterHitbox = GetMonsterHitbox(&game->monsters[m]);
                    if (CheckCollisionRecs(projectileRect, monsterHitbox))
                    {
                        proj->active = false;
                        game->score++;
                        DamageMonster(&game->monsters[m], DAMAGE_PLAYER_PROJECTILE);
                        CreateHitEffect(game, proj->position, ORANGE);
                        break;
                    }
                }
            }
        }
        else if (proj->owner == PROJECTILE_MONSTER || proj->owner == PROJECTILE_MONSTER_SPREAD)
        {
            if (!IsPlayerDead(&game->player) && CheckCollisionRecs(projectileRect, playerHitbox))
            {
                proj->active = false;

                const float damage = (proj->owner == PROJECTILE_MONSTER_SPREAD) ? DAMAGE_MONSTER_SPREAD_SHOT : DAMAGE_MONSTER_PROJECTILE;
                game->player.health -= damage;
                if (game->player.health < 0.0f)
                    game->player.health = 0.0f;
            }
        }
    }
    
    RemoveInactiveProjectiles(game);
}

bool AreAllMonstersDead(GameState* game)
{
    for (int i = 0; i < MONSTER_COUNT; i++)
    {
        if (!IsMonsterDead(&game->monsters[i]))
        {
            return false;
        }
    }

    return true;
}

void UpdateAllMonsters(GameState* game, const float deltaTime)
{
    for (int i = 0; i < MONSTER_COUNT; i++) 
    {
        const bool wasAlive = !IsMonsterDead(&game->monsters[i]);
        UpdateMonster(&game->monsters[i], deltaTime);
        const bool isNowDead = IsMonsterDead(&game->monsters[i]);
        
        if (wasAlive && isNowDead)
            CreateDeathExplosion(game, game->monsters[i].position, RED, game->monsters[i].size * 1.5f);
    }
}

void ApplyProximityDamageToPlayer(GameState* game, float deltaTime)
{
    for (int i = 0; i < MONSTER_COUNT; i++)
    {
        if (!IsMonsterDead(&game->monsters[i]))
        {
            const float dx = game->player.position.x - game->monsters[i].position.x;
            const float dy = game->player.position.y - game->monsters[i].position.y;
            const float distance = sqrtf(dx * dx + dy * dy);
            const float dangerZone = game->monsters[i].size;
            
            if (distance < dangerZone)
            {
                game->player.health -= DAMAGE_PROXIMITY_PER_SECOND * deltaTime;
                if (game->player.health < 0.0f)
                    game->player.health = 0.0f;
            }
        }
    }
}

void BuildVisibleMonstersArray(const GameState* game, MonsterView* visibleMonsters, int* outCount)
{
    int visibleCount = 0;
    
    for (int i = 0; i < MONSTER_COUNT; i++)
    {
        if (!IsMonsterDead(&game->monsters[i]))
        {
            const float dx = game->player.position.x - game->monsters[i].position.x;
            const float dy = game->player.position.y - game->monsters[i].position.y;
            const float distance = sqrtf(dx * dx + dy * dy);
            
            if (distance <= PLAYER_VISION_RADIUS)
            {
                visibleMonsters[visibleCount].position = game->monsters[i].position;
                visibleMonsters[visibleCount].size = game->monsters[i].size;
                visibleMonsters[visibleCount].health = game->monsters[i].health;
                visibleMonsters[visibleCount].invincible = game->monsters[i].invincibilityTimer > 0.0f;
                visibleCount++;
            }
        }
    }
    *outCount = visibleCount;
}

void ProcessPlayerAIAndShooting(GameState* game)
{
    static MonsterView visibleMonsters[MONSTER_COUNT];
    int visibleCount;
    
    BuildVisibleMonstersArray(game, visibleMonsters, &visibleCount);
    
    BeginPlayerAI(&game->player, 0.0f);
    
    PlayerContext context;
    context.playerPosition = game->player.position;
    context.playerVelocity = game->player.velocity;
    context.playerRotation = game->player.rotation;
    context.visibleMonsters = visibleMonsters;
    context.visibleMonsterCount = visibleCount;
    context.deltaTime = GetFrameTime();
    
    OnPlayerUpdate(context);

    ShootDirection shootDir;
    if (PlayerDidShoot(&shootDir))
    {
        const float baseAngle = game->player.rotation;
        
        float shootAngle = baseAngle;
        if (shootDir == SHOOT_LEFT)
            shootAngle -= 90.0f;
        else if (shootDir == SHOOT_RIGHT)
            shootAngle += 90.0f;
        
        ShootProjectile(game, shootAngle, game->player.position, PROJECTILE_PLAYER, -1);
    }

    EndPlayerAI();
}

void ProcessMonsterShooting(GameState* game)
{
    for (int m = 0; m < MONSTER_COUNT; m++)
    {
        if (!IsMonsterDead(&game->monsters[m]) && !IsPlayerDead(&game->player) && game->monsters[m].canShoot)
        {
            if (game->monsters[m].shootTimer <= 0.0f)
            {
                const float dx = game->player.position.x - game->monsters[m].position.x;
                const float dy = game->player.position.y - game->monsters[m].position.y;
                const float angleToPlayer = atan2f(dy, dx) * RAD2DEG;
                
                ShootProjectile(game, angleToPlayer, game->monsters[m].position, PROJECTILE_MONSTER, m);
                game->monsters[m].shootTimer = game->monsters[m].shootCooldown;
            }
            
            if (game->monsters[m].spreadShotTimer <= 0.0f)
            {
                for (int i = 0; i < MONSTER_SPREAD_SHOT_COUNT; i++)
                {
                    const float angle = (360.0f / MONSTER_SPREAD_SHOT_COUNT) * i;
                    ShootProjectile(game, angle, game->monsters[m].position, PROJECTILE_MONSTER_SPREAD, m);
                }
                game->monsters[m].spreadShotTimer = game->monsters[m].spreadShotCooldown;
            }
        }
    }
}

void UpdateGameState(GameState* game, const float deltaTime)
{
    const bool allMonstersDead = AreAllMonstersDead(game);
    
    if (!IsPlayerDead(&game->player) && !allMonstersDead)
    {
        game->gameTime += deltaTime;
    }
    
    UpdateAllMonsters(game, deltaTime);
    
    const bool wasPlayerAlive = !IsPlayerDead(&game->player);
    UpdatePlayer(&game->player, deltaTime);
    const bool isPlayerNowDead = IsPlayerDead(&game->player);
    
    if (wasPlayerAlive && isPlayerNowDead)
    {
        CreateDeathExplosion(game, game->player.position, BLUE, PLAYER_SIZE * 1.5f);
    }
    
    if (!IsPlayerDead(&game->player) && !allMonstersDead)
    {
        ApplyProximityDamageToPlayer(game, deltaTime);
        ProcessPlayerAIAndShooting(game);
    }
    
    ProcessMonsterShooting(game);
    UpdateProjectiles(game, deltaTime);
    UpdateHitEffects(game, deltaTime);
    UpdateDeathExplosions(game, deltaTime);
}

void DrawGameState(GameState* game)
{
    ClearBackground((Color){ 10, 10, 50, 255 });

    DrawStars(game);

    for (int i = 0; i < MONSTER_COUNT; i++)
    {
        if (!IsMonsterDead(&game->monsters[i]))
        {
            const float dx = game->monsters[i].position.x - game->player.position.x;
            const float dy = game->monsters[i].position.y - game->player.position.y;
            const float distance = sqrtf(dx * dx + dy * dy);
            
            float opacity = 1.0f;
            if (distance > PLAYER_VISION_RADIUS)
            {
                const float fadeDistance = 100.0f;
                const float fadeAmount = (distance - PLAYER_VISION_RADIUS) / fadeDistance;
                opacity = 1.0f - fadeAmount;
                if (opacity < 0.0f) opacity = 0.0f;
            }
            
            DrawMonster(&game->monsters[i], game->player.position, opacity);
        }
    }

    if (!IsPlayerDead(&game->player))
    {
        DrawPlayer(&game->player);
    }

    for (int i = 0; i < game->projectileCount; i++)
    {
        if (game->projectiles[i].active)
        {
            Projectile* proj = &game->projectiles[i];
            DrawRectangle(
                (int)(proj->position.x - proj->size / 2),
                (int)(proj->position.y - proj->size / 2),
                (int)proj->size,
                (int)proj->size,
                proj->color
            );
        }
    }

    DrawHitEffects(game);
    DrawDeathExplosions(game);

    DrawUI(game);
}

void CleanupGameState(GameState* game)
{
    for (int i = 0; i < MONSTER_COUNT; i++)
    {
        CleanupMonster(&game->monsters[i]);
    }
    UnloadTexture(game->sharedMonsterTexture);

    CleanupPlayer(&game->player);
    free(game->projectiles);
}

#ifdef PLATFORM_WEB
#include <emscripten.h>
static GameState* g_gameState = NULL;

static void game_loop(void)
{
    if (!g_gameState) return;

    const float deltaTime = GetFrameTime();
    
    UpdateGameState(g_gameState, deltaTime);

    BeginDrawing();
    DrawGameState(g_gameState);
    EndDrawing();
}
#endif

int main(void)
{
    srand((unsigned int)time(NULL));
    
#ifdef PLATFORM_WEB
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Espace Confiote - Web Edition");
#else
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Espace Confiote - C11 Edition");
#endif
    SetTargetFPS(60);

#ifdef PLATFORM_WEB
    // For web, no script path needed - user provides code via UI
    if (!InitPythonAI(NULL))
    {
        fprintf(stderr, "Warning: Failed to initialize Python AI. The game will run but player AI won't work.\n");
    }
#endif

    GameState game;
    InitGameState(&game);

#ifdef PLATFORM_WEB
    g_gameState = &game;
    // Use Emscripten's main loop for web
    emscripten_set_main_loop(game_loop, 0, 1);
    // This won't return, but we need it for compilation
    return 0;
#else
    while (!WindowShouldClose())
    {
        float deltaTime = GetFrameTime();
        
        UpdateGameState(&game, deltaTime);

        BeginDrawing();
        DrawGameState(&game);
        EndDrawing();
    }

    CleanupGameState(&game);
    CloseWindow();

    return 0;
#endif
}
