#pragma once

#include "WindowsWrapper.h"

extern int g_GameFlags;
extern int gCounter;

int Random(int min, int max);
void PutNumber4(int x, int y, int value, BOOL bZero);

BOOL Game(HWND hWnd);
