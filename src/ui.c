#include "ui.h"
#include "main.h"
#include "config.h"
#include "player.h"
#include "monster.h"
#include <math.h>

void DrawStars(const GameState* game)
{
    for (int i = 0; i < STAR_COUNT; i++)
    {
        const Star* star = &game->stars[i];
        
        float blinkFactor = sinf(game->gameTime * STAR_BLINK_SPEED + star->phase);
        blinkFactor = (blinkFactor + 1.0f) / 2.0f;

        const int opacity = STAR_OPACITY_MIN + (int)(blinkFactor * (STAR_OPACITY_MAX - STAR_OPACITY_MIN));
        const Color starColor = (Color){255, 255, 255, (unsigned char)opacity};
        
        for (int py = 0; py < star->size; py++)
        {
            for (int px = 0; px < star->size; px++)
            {
                DrawPixel((int)star->position.x + px, (int)star->position.y + py, starColor);
            }
        }
    }
}

void DrawPlayerHealthBar(Player* player)
{
    if (IsPlayerDead(player))
    {
        return;
    }
    
    int barWidth = 200;
    int barHeight = 20;
    int barX = 10;
    int barY = 10;
    
    DrawRectangle(barX, barY, barWidth, barHeight, (Color){50, 50, 50, 200});

    const int healthWidth = (int)((float)barWidth * player->health);
    Color healthColor = GREEN;
    if (player->health < 0.3f)
        healthColor = RED;
    else if (player->health < 0.6f)
        healthColor = ORANGE;
    
    DrawRectangle(barX, barY, healthWidth, barHeight, healthColor);
    DrawRectangleLines(barX, barY, barWidth, barHeight, WHITE);
    DrawText(TextFormat("Player: %.0f%%", player->health * 100), barX + 5, barY + 2, 16, WHITE);
}

void DrawPlayerCooldownBar(Player* player)
{
    if (IsPlayerDead(player))
        return;

    const int barWidth = 200;
    const int barHeight = 20;
    const int barX = 10;
    const int barY = 10;

    const int cooldownBarY = barY + barHeight + 5;
    const int cooldownBarHeight = 8;
    
    DrawRectangle(barX, cooldownBarY, barWidth, cooldownBarHeight, (Color){50, 50, 50, 200});
    
    float cooldownRatio = 1.0f - (player->shootTimer / player->shootCooldown);
    if (cooldownRatio < 0.0f)
        cooldownRatio = 0.0f;
    if (cooldownRatio > 1.0f)
        cooldownRatio = 1.0f;

    const int cooldownWidth = (int)((float)barWidth * cooldownRatio);
    const Color cooldownColor = cooldownRatio >= 1.0f ?
        SKYBLUE :
        (Color){100, 100, 150, 255};
    
    DrawRectangle(barX, cooldownBarY, cooldownWidth, cooldownBarHeight, cooldownColor);
    DrawRectangleLines(barX, cooldownBarY, barWidth, cooldownBarHeight, WHITE);
}

void DrawGameInfo(const int score, const float gameTime)
{
    const int infoY = 10 + 20 + 5 + 8 + 10;
    
    DrawText(TextFormat("Score: %d", score), 10, infoY, 20, WHITE);

    const int minutes = (int)(gameTime / 60.0f);
    const int seconds = (int)gameTime % 60;
    const int milliseconds = (int)((gameTime - (int)gameTime) * 1000);

    DrawText(TextFormat("Time: %02d:%02d.%03d", minutes, seconds, milliseconds), 10, infoY + 25, 20, WHITE);
    DrawFPS(10, infoY + 50);
}

void DrawMonsterHealthBars(Monster* monsters, int monsterCount)
{
    int barY = 10;
    
    for (int i = 0; i < monsterCount; i++)
    {
        if (!IsMonsterDead(&monsters[i]))
        {
            const int barWidth = 150;
            const int barHeight = 15;
            const int barX = SCREEN_WIDTH - barWidth - 10;
            
            DrawRectangle(barX, barY, barWidth, barHeight, (Color){50, 50, 50, 200});

            const int healthWidth = (int)((float)barWidth * monsters[i].health);
            Color healthColor = GREEN;
            if (monsters[i].health < 0.3f)
                healthColor = RED;
            else if (monsters[i].health < 0.6f)
                healthColor = ORANGE;
            
            DrawRectangle(barX, barY, healthWidth, barHeight, healthColor);
            DrawRectangleLines(barX, barY, barWidth, barHeight, WHITE);
            DrawText(TextFormat("M%d: %.0f%%", i+1, monsters[i].health * 100), barX + 5, barY + 1, 12, WHITE);
            
            barY += barHeight + 5;
        }
    }
}

void DrawGameOverScreen(int score)
{
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){0, 0, 0, 180});
    
    const char* gameOverText = "Game Over";
    const int gameOverSize = 80;
    const int gameOverWidth = MeasureText(gameOverText, gameOverSize);
    DrawText(gameOverText, (SCREEN_WIDTH - gameOverWidth) / 2, SCREEN_HEIGHT / 2 - 60, gameOverSize, RED);
    
    const char* scoreText = TextFormat("Score: %d", score);
    const int scoreSize = 40;
    const int scoreWidth = MeasureText(scoreText, scoreSize);
    DrawText(scoreText, (SCREEN_WIDTH - scoreWidth) / 2, SCREEN_HEIGHT / 2 + 40, scoreSize, WHITE);
}

bool AreAllMonstersFullyExploded(Monster* monsters, int monsterCount)
{
    for (int i = 0; i < monsterCount; i++)
    {
        if (!IsMonsterFullyExploded(&monsters[i]))
        {
            return false;
        }
    }
    return true;
}

void DrawVictoryScreen(const int score, const float gameTime)
{
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){0, 0, 0, 180});
    
    const char* victoryText = "Victory!";
    const int victorySize = 80;
    const int victoryWidth = MeasureText(victoryText, victorySize);
    DrawText(victoryText, (SCREEN_WIDTH - victoryWidth) / 2, SCREEN_HEIGHT / 2 - 150, victorySize, GREEN);

    const int minutes = (int)(gameTime / 60.0f);
    const int seconds = (int)gameTime % 60;
    const int milliseconds = (int)((gameTime - (int)gameTime) * 1000);
    const char* timeText = TextFormat("%02d:%02d.%03d", minutes, seconds, milliseconds);
    const int timeSize = 70;
    const int timeWidth = MeasureText(timeText, timeSize);
    DrawText(timeText, (SCREEN_WIDTH - timeWidth) / 2, SCREEN_HEIGHT / 2 - 50, timeSize, YELLOW);
    
    const char* scoreText = TextFormat("Score: %d", score);
    const int scoreSize = 40;
    const int scoreWidth = MeasureText(scoreText, scoreSize);
    DrawText(scoreText, (SCREEN_WIDTH - scoreWidth) / 2, SCREEN_HEIGHT / 2 + 60, scoreSize, WHITE);
}

void DrawUI(GameState* game)
{
    DrawPlayerHealthBar(&game->player);
    DrawPlayerCooldownBar(&game->player);
    DrawGameInfo(game->score, game->gameTime);
    DrawMonsterHealthBars(game->monsters, MONSTER_COUNT);
    
    if (IsPlayerFullyExploded(&game->player))
        DrawGameOverScreen(game->score);
    else if (AreAllMonstersFullyExploded(game->monsters, MONSTER_COUNT))
        DrawVictoryScreen(game->score, game->gameTime);
}
