#ifndef STDLIB_H
#define STDLIB_H

unsigned short lfsr = 0xACE1u;
unsigned bit;

unsigned ksrand(unsigned b) {
	lfsr = 0xACE1u;
	bit = b;
}

unsigned krand() {
	bit = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5) ) & 1;
	return lfsr =  (lfsr >> 1) | (bit << 15);
}

#endif
