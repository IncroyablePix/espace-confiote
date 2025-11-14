#include "player_ai.h"
#include "../player.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef PLATFORM_WEB
#include <emscripten.h>
#endif

static bool g_webAIInitialized = false;
static char* g_userPythonCode = NULL;

#ifdef PLATFORM_WEB
// JavaScript functions exposed to C
EM_JS(void, js_init_pyodide, (void), {
    if (typeof window.initPyodide === 'function') {
        window.initPyodide();
    }
});

EM_JS(void, js_load_python_code, (const char* code), {
    const codeStr = UTF8ToString(code);
    if (typeof window.loadPythonCode === 'function') {
        window.loadPythonCode(codeStr);
    }
});

EM_JS(void, js_call_python_update, (const char* contextJson), {
    const jsonStr = UTF8ToString(contextJson);
    if (typeof window.callPythonUpdate === 'function') {
        window.callPythonUpdate(jsonStr);
    }
});

EM_JS(void, js_set_thrusters, (float left, float right), {
    if (typeof window.setThrusters === 'function') {
        window.setThrusters(left, right);
    }
});

EM_JS(void, js_shoot, (int direction), {
    if (typeof window.shoot === 'function') {
        window.shoot(direction);
    }
});

// Functions called from JavaScript
EMSCRIPTEN_KEEPALIVE
void web_player_shoot(int direction)
{
    // printf("web_player_shoot called with direction: %d\n", direction);
    if (direction < 0 || direction > 2)
    {
        return;
    }
    PlayerShoot((ShootDirection)direction);
}

EMSCRIPTEN_KEEPALIVE
void web_player_set_thrusters(float leftPower, float rightPower)
{
    PlayerSetThrusters(leftPower, rightPower);
}
#endif // PLATFORM_WEB

static char* ContextToJSON(PlayerContext* context)
{
    // Estimate buffer size (should be enough for typical data)
    const size_t bufferSize = 4096; // should be enough
    char* json = (char*)malloc(bufferSize);
    if (!json) return NULL;
    
    int written = snprintf(json, bufferSize,
        "{"
        "\"playerPosition\":{\"x\":%.2f,\"y\":%.2f},"
        "\"playerVelocity\":{\"x\":%.2f,\"y\":%.2f},"
        "\"playerRotation\":%.2f,"
        "\"deltaTime\":%.4f,"
        "\"visibleMonsterCount\":%d,"
        "\"visibleMonsters\":[",
        context->playerPosition.x, context->playerPosition.y,
        context->playerVelocity.x, context->playerVelocity.y,
        context->playerRotation,
        context->deltaTime,
        context->visibleMonsterCount
    );
    
    for (int i = 0; i < context->visibleMonsterCount; i++)
    {
        if (i > 0)
        {
            written += snprintf(json + written, bufferSize - written, ",");
        }
        written += snprintf(json + written, bufferSize - written,
            "{"
            "\"position\":{\"x\":%.2f,\"y\":%.2f},"
            "\"size\":%.2f,"
            "\"health\":%.2f,"
            "\"invincible\":%s"
            "}",
            context->visibleMonsters[i].position.x,
            context->visibleMonsters[i].position.y,
            context->visibleMonsters[i].size,
            context->visibleMonsters[i].health,
            context->visibleMonsters[i].invincible ? "true" : "false"
        );
    }
    
    written += snprintf(json + written, bufferSize - written, "]}");
    
    return json;
}

bool InitPythonAI(const char* scriptPath)
{
    if (g_webAIInitialized)
    {
        return true;
    }
    
#ifdef PLATFORM_WEB
    // Initialize Pyodide in JavaScript
    js_init_pyodide();
#else
    // Not a web build - this shouldn't be called
    return false;
#endif
    
    // For web, we don't load from file - user provides code via UI
    // But we can load default code if provided
    if (scriptPath)
    {
        // In web build, scriptPath might be user-provided code or a URL
        // For now, we'll just mark as initialized
        // The actual code loading happens via JavaScript
    }
    
    g_webAIInitialized = true;
    return true;
}

void CleanupPythonAI(void)
{
    if (g_userPythonCode)
    {
        free(g_userPythonCode);
        g_userPythonCode = NULL;
    }
    g_webAIInitialized = false;
}

void OnPlayerUpdate(PlayerContext context)
{
    if (!g_webAIInitialized)
    {
        return;
    }
    
    // Convert context to JSON
    char* contextJson = ContextToJSON(&context);
    if (!contextJson)
    {
        return;
    }
    
#ifdef PLATFORM_WEB
    // Call JavaScript function which will use Pyodide to execute Python
    js_call_python_update(contextJson);
#endif
    
    free(contextJson);
}

#ifdef PLATFORM_WEB
// Function to set user Python code (called from JavaScript)
EMSCRIPTEN_KEEPALIVE
void set_user_python_code(const char* code)
{
    if (g_userPythonCode)
    {
        free(g_userPythonCode);
        g_userPythonCode = NULL;
    }
    if (code && strlen(code) > 0)
    {
        g_userPythonCode = (char*)malloc(strlen(code) + 1);
        if (g_userPythonCode)
        {
            strcpy(g_userPythonCode, code);
            js_load_python_code(g_userPythonCode);
        }
    }
}
#endif // PLATFORM_WEB
