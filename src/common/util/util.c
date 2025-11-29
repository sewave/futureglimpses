#include "util.h"

char has_minimal_free_memory(int megaBytes) {
	void* ptr = malloc(megaBytes * 1024 * 1024);
	if (ptr) {
		free(ptr);
		return 1;
	}
	return 0;
}
