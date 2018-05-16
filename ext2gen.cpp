#include <iostream>
#include "hexdump.h"

int main (int argc, char const *argv[]) {
	int abc[100] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

	util::hexdump(abc, sizeof(abc));
	return 0;
}