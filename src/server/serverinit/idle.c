#include "idle.h"

//--============
// -- GLOBALS
//--============

static int idle_f = 0;

//--============
// -- PRIVATE
//--============

/// @brief enable raw mode
/// @param *old
static void enable_raw_mode(struct termios *old) {
	struct termios raw;
	tcgetattr(STDIN_FILENO, old);
	raw = *old;
	raw.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

/// @brief disable raw mode
/// @param *old
static void disable_raw_mode(struct termios *old) {
	tcsetattr(STDIN_FILENO, TCSANOW, old);
}

//--============
// -- LOGIC
//--============

void idle_mode(void) {
	struct termios old;
	enable_raw_mode(&old);

	idle_f = 1;

	cmd_clear();
	printf("C-x to exit\n\n");

	while (1) {
		if (getchar() == EXIT_KEY)
			break;
	}

	idle_f = 0;

	disable_raw_mode(&old);
	cmd_clear();
}

int is_idle(void) { return idle_f; }
