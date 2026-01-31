#include <allegro.h>
#include "game/main/main.h"

int main(int argc, char *argv[]) {
	if(main_init() != PROGRAM_OK) return PROGRAM_ERROR;
	main_loop();
	main_clean();
	return PROGRAM_OK;
}
END_OF_MAIN()
