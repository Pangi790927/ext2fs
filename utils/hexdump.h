#ifndef HEXDUMP_H
#define HEXDUMP_H

#include <cstdlib>

namespace util
{
	void hexdump (void *start, size_t size);
}

#endif