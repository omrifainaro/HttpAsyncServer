#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>

// Must not end with a /
void initFileHandler(char* root_path);

int getFileContent(char* filePath, BYTE** buffer, SIZE_T *bufferLen);