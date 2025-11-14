#ifndef SPACE_JAM_PLAYER_AI_H
#define SPACE_JAM_PLAYER_AI_H
#include <stdbool.h>

bool InitPythonAI(const char* scriptPath);
void CleanupPythonAI(void);

#endif //SPACE_JAM_PLAYER_AI_H