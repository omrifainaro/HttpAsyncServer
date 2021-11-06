#include "FileHandler.h"

char* g_root_path;

static char* getFullPath(char* path) {
	char* fullPath = NULL;
	if (strstr(path, "..")) {
		path = "/kaka.html";
	}
	fullPath = malloc(strlen(g_root_path) + strlen(path) + 1);
	fullPath[0] = 0;
	strcat(fullPath, g_root_path);
	strcat(fullPath, path);
	return fullPath;
}

void initFileHandler(char* root_path) {
	g_root_path = root_path; 
}

int getFileContent(char* filePath, BYTE** buffer, SIZE_T* bufferLen) {
	HANDLE file = INVALID_HANDLE_VALUE;
	DWORD size = 0;
	char* path = getFullPath(filePath);
	file = CreateFileA(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == INVALID_HANDLE_VALUE) {
		return 0;
	}
	
	size = GetFileSize(file, NULL);
	if (size == INVALID_FILE_SIZE) {
		CloseHandle(file);
		return 0;
	}

	*buffer = (BYTE*)malloc(size);
	*bufferLen = size;

	if (ReadFile(file, *buffer, size, NULL, NULL)) {
		CloseHandle(file);
		return 1;
	}

	CloseHandle(file);
	return 0;
}