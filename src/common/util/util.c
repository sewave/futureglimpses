#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "common/util.h"
#ifdef __MSDOS__
    #include <dpmi.h>
	#define DOS_RESERVED_PAGES 372
#elif defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#endif

unsigned int get_total_physical_memory_mb(void) {
#ifdef __MSDOS__
	__dpmi_free_mem_info info;
	if (__dpmi_get_free_memory_information(&info) == 0) {
		int pages = info.total_number_of_physical_pages + DOS_RESERVED_PAGES;
		if (pages > 0) {
			return (unsigned int) (pages * 4096 / (1024 * 1024));
		}
	}
#elif defined(_WIN32) || defined(_WIN64)
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof(statex);
	if (GlobalMemoryStatusEx(&statex)) {
		return (unsigned int) (statex.ullTotalPhys / (1024 * 1024));
	}
#endif
	return 0;
}

int distance_sq(int x1, int y1, int x2, int y2) {
	int xDif = x2 - x1;
	int yDif = y2 - y1;
	return xDif * xDif + yDif * yDif;
}

/**
 * Generates a random int between [minVal, maxVal].
 */
int random_int(int minVal, int maxVal) {
	return minVal + (rand() % (maxVal - minVal + 1));
}

void get_parent_directory(const char *path, char *outBuffer, unsigned int bufferSize) {
	if (!path || !outBuffer || bufferSize == 0) {
		if (outBuffer && bufferSize > 0) {
			outBuffer[0] = '\0';
		}
		return;
	}

	snprintf(outBuffer, bufferSize, "%s", path);

	char *lastSep = strrchr(outBuffer, '\\');
	char *lastSepUnix = strrchr(outBuffer, '/');

	char *sep = lastSep;
	if (lastSepUnix && (!sep || lastSepUnix > sep)) {
		sep = lastSepUnix;
	}

	if (sep) {
		*sep = '\0';
	} else {
		outBuffer[0] = '\0';
	}
}

void separate_ints_in_string(char *buffer, char* separator, int first, int second) {
	itoa(first, buffer, BASE_TEN_NUMBER);
	int length = strlen(buffer);
	buffer[length++] = *separator;
	itoa(second, buffer + length, BASE_TEN_NUMBER);
}
