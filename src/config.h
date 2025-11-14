#ifndef CONFIG_H
#define CONFIG_H

#include "raylib.h"

// Screen settings
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

// Player settings
#define PLAYER_SIZE 60.0f
#define PLAYER_THRUST_POWER 300.0f
#define PLAYER_TURN_RATE 180.0f
#define PLAYER_MAX_SPEED 200.0f
#define PLAYER_SHOOT_COOLDOWN 0.2f
#define PLAYER_DRAG 0.98f
#define PLAYER_VISION_RADIUS 400.0f

// Monster settings
#define MONSTER_COUNT 5
#define MONSTER_MIN_SIZE 80.0f
#define MONSTER_MAX_SIZE 150.0f
#define MONSTER_SHOOT_COOLDOWN 0.25f
#define MONSTER_SPREAD_SHOT_COOLDOWN 3.0f
#define MONSTER_SPREAD_SHOT_INITIAL_DELAY 1.5f
#define MONSTER_SPREAD_SHOT_COUNT 12
#define MONSTER_INVINCIBILITY_DURATION 1.5f
#define MONSTER_BLINK_FREQUENCY 3.5f

// Star settings
#define STAR_COUNT 100
#define STAR_MIN_SIZE 1
#define STAR_MAX_SIZE 3
#define STAR_BLINK_SPEED 2.0f
#define STAR_OPACITY_MIN 100
#define STAR_OPACITY_MAX 255

// Projectile settings
#define PROJECTILE_SPEED 400.0f
#define PROJECTILE_SIZE 10.0f
#define PROJECTILE_PLAYER_COLOR ORANGE
#define PROJECTILE_MONSTER_COLOR PURPLE
#define PROJECTILE_MONSTER_SPREAD_COLOR ((Color){0, 255, 255, 255})

// Damage settings
#define DAMAGE_PLAYER_PROJECTILE 0.05f      // 5% damage per player projectile hit
#define DAMAGE_MONSTER_PROJECTILE 0.15f     // 15% damage per monster projectile hit
#define DAMAGE_MONSTER_SPREAD_SHOT 0.08f    // 8% damage per spread shot hit
#define DAMAGE_PROXIMITY_PER_SECOND 0.3f    // 30% damage per second when near monster

// Explosion settings
#define EXPLOSION_FRAME_DURATION (1.0f / 6.0f)
#define EXPLOSION_FRAME_COUNT 5
#define EXPLOSION_FADE_START_TIME 1.0f      // Time before explosion starts fading
#define EXPLOSION_FADE_DURATION 2.0f        // Duration of fade out

// Hit effect settings
#define HIT_EFFECT_MAX_COUNT 100
#define HIT_EFFECT_DURATION 0.3f
#define HIT_EFFECT_SIZE 15.0f
#define HIT_EFFECT_PARTICLE_COUNT 8

// Death explosion settings
#define DEATH_EXPLOSION_DURATION 1.0f
#define DEATH_EXPLOSION_SIZE 80.0f
#define DEATH_EXPLOSION_PARTICLE_COUNT 24

#endif // CONFIG_H
