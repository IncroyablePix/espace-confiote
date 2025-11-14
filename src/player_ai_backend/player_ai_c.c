#include "../player.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

// This is where you implement your player AI!
// You have access to:
// - context.playerPosition - your current position
// - context.playerVelocity - your current velocity
// - context.playerRotation - your current rotation (degrees)
// - context.visibleMonsters - array of visible monster views
// - context.visibleMonsterCount - number of visible monsters
// - context.deltaTime - time since last frame
//
// Available functions:
// - PlayerShoot(direction) - shoot in direction: SHOOT_FORWARD, SHOOT_LEFT, or SHOOT_RIGHT
// - PlayerSetThrusters(leftPower, rightPower) - set thruster power from -1.0 to 1.0
//   - Both same = move forward/backward
//   - Different = turn (right > left = turn right)

typedef struct
{
    MonsterView monster;
    float distance;
} MonsterDistance;

int CompareDistance(const void *a, const void *b)
{
    float distA = ((MonsterDistance *)a)->distance;
    float distB = ((MonsterDistance *)b)->distance;
    if (distA < distB)
        return -1;
    if (distA > distB)
        return 1;
    return 0;
}

MonsterDistance* SortMonstersByDistance(PlayerContext context)
{
    if (context.visibleMonsterCount == 0)
    {
        return NULL;
    }

    MonsterDistance *distances = malloc(sizeof(MonsterDistance) * context.visibleMonsterCount);
    for (int i = 0; i < context.visibleMonsterCount; i++)
    {
        float dx = context.playerPosition.x - context.visibleMonsters[i].position.x;
        float dy = context.playerPosition.y - context.visibleMonsters[i].position.y;
        float dist = hypotf(dx, dy);
        distances[i].monster = context.visibleMonsters[i];
        distances[i].distance = dist;
    }

    qsort(distances, context.visibleMonsterCount, sizeof(MonsterDistance), CompareDistance);
    return distances;
}

MonsterView* GetNearestMonster(PlayerContext* context, MonsterDistance* distances, bool skipInvincible)
{
    if (distances == NULL)
    {
        return NULL;
    }

    size_t index = 0;

    if (skipInvincible)
    {
        while (distances[index].monster.invincible)
        {
            index++;
            if (index >= context->visibleMonsterCount) // Reached end of array
            {
                return NULL;
            }
        }
    }

    return &distances[index].monster;
}

float ClampAngle(float angle)
{
    while (angle < 0) angle += 360.0f;
    while (angle >= 360.0f) angle -= 360.0f;
    return angle;
}

void ThrustBackward(void)
{
    printf("Too close to monster! Thrusting backward.\n");
    PlayerShoot(SHOOT_FORWARD);
    PlayerSetThrusters(-1.0f, -1.0f); // Thrust backward
}

void ShootForward(void)
{
    printf("Close angle - Shooting forward.\n");
    PlayerSetThrusters(1.0f, 1.0f); // Move forward
    PlayerShoot(SHOOT_FORWARD);
}

void ThrustToMatchAngle(float angleDiff)
{
    if (angleDiff > 180.0f)
    {
        printf("Turning left.\n");
        PlayerSetThrusters(1.f, 0.f); // Turn left
        PlayerShoot(SHOOT_LEFT);
    }
    else
    {
        printf("Turning right.\n");
        PlayerSetThrusters(0.f, 1.f); // Turn right
        PlayerShoot(SHOOT_RIGHT);
    }
}

void ShootRandomly(void)
{
    ShootDirection dir = (ShootDirection)(rand() % 3);
    PlayerShoot(dir);
}

void Roam(void)
{
    printf("No visible monsters. Moving forward.\n");
    ShootRandomly();
    PlayerSetThrusters(1.0f, 1.0f); // Just move forward if no monsters
}

bool IsMonsterOnTheWay(Vector2 playerPosition, float angle, MonsterView* monster)
{
    Vector2 toMonster =
    {
        monster->position.x - playerPosition.x,
        monster->position.y - playerPosition.y
    };

    float angleToMonster = ClampAngle(atan2f(toMonster.y, toMonster.x) * RAD2DEG);
    float angleDiff = angleToMonster - angle;
    angleDiff = ClampAngle(angleDiff);

    return fabs(angleDiff) < 15.0f; // Consider "on the way" if within 15 degrees
}

void OnPlayerUpdate(PlayerContext context)
{
    const float rangeAngle = 7.5f;
    const float minDistance = 250.0f;
    MonsterDistance* sortedDistances = SortMonstersByDistance(context);
    MonsterView *nearestMonsterAll = GetNearestMonster(&context, sortedDistances, true);
    MonsterView *nearestMonster = GetNearestMonster(&context, sortedDistances, false);

    if (nearestMonster == NULL)
    {
        if (nearestMonsterAll != NULL && IsMonsterOnTheWay(context.playerPosition, context.playerRotation, nearestMonsterAll))
        {
            ShootRandomly();
            goto end;
        }
        Roam();
        goto end;
    }

    Vector2 toMonster =
    {
        nearestMonster->position.x - context.playerPosition.x,
        nearestMonster->position.y - context.playerPosition.y
    };

    float distToMonster = hypotf(toMonster.x, toMonster.y);
    float angleToMonster = ClampAngle(atan2f(toMonster.y, toMonster.x) * RAD2DEG);
    
    float angleDiff = angleToMonster - context.playerRotation; // Rotate toward target
    angleDiff = ClampAngle(angleDiff);

    printf("Distance to monster: %.2f, Angle diff: %.2f\n", distToMonster, angleDiff);
    
    if (distToMonster < minDistance) // Check if too close to monster
    {
        ThrustBackward();
        goto end;
    }
    else
    {
        if (fabs(angleDiff) > rangeAngle) // Far angle - turn in place
        {
            if (nearestMonster != NULL && IsMonsterOnTheWay(context.playerPosition, context.playerRotation, nearestMonster))
            {
                printf("Monster on the way! Thrusting forward while turning.\n");
                PlayerSetThrusters(1.0f, 0.0f); // Turn while moving forward
                if (angleDiff > 180.0f)
                {
                    PlayerShoot(SHOOT_LEFT);
                }
                else
                {
                    PlayerShoot(SHOOT_RIGHT);
                }
                goto end;
            }
            ThrustToMatchAngle(angleDiff);
            goto end;
        }

        // Close angle - move forward and shoot
        ShootForward();
    }

end:
    free(sortedDistances);
    printf("------------------------\n");
}