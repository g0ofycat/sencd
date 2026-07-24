#include "clear.h"

//--============
// -- LOGIC
//--============

void cmd_clear() { printf("\e[1;1H\e[2J"); }
