#include "hexdump.h"
#include <iostream>
#include <cstdlib>

namespace util
{
	void hexdump(void *ptr, size_t buflen) {
	  unsigned char *buf = (unsigned char*)ptr;
	  int i, j;
	  for (i=0; i<buflen; i+=16) {
	    printf("%06x: ", i);
	    for (j=0; j<16; j++) 
	      if (i+j < buflen)
	        printf("%02x ", buf[i+j]);
	      else
	        printf("   ");
	    printf(" ");
	    for (j=0; j<16; j++) 
	      if (i+j < buflen)
	        printf("%c", isprint(buf[i+j]) ? buf[i+j] : '.');
	    printf("\n");
	  }
	}
	// void hexdump (void *start, size_t size) {
	// 	uint8_t *address = (uint8_t *)start;
	// 	uint32_t printedCount = 0;

	// 	std::string last = "";
	// 	std::string current = "";
	// 	bool printedStar = false;

	// 	char hex[] = "0123456789abcdef";
	// 	auto printNext = [&]() {
	// 		if (printedCount != size) {
	// 			current += hex[(address[printedCount] >> 4) & 0xf];
	// 			current += hex[address[printedCount] & 0xf];
	// 			printedCount++;
	// 		}
	// 	};
	// 	auto hexToStrPadded = [&](uint32_t number, int padding) {
	// 		std::string strNumber;

	// 		while (number) {
	// 			strNumber = hex[number % 16] + strNumber;
	// 			number /= 16;
	// 			padding--;
	// 		}
	// 		while (padding > 0) {
	// 			strNumber = "0" + strNumber;
	// 			padding--;
	// 		}
	// 		return strNumber;
	// 	};
	// 	while (printedCount != size) {
	// 		for (int i = 0; i < 8 && printedCount != size; i++) {
	// 			current += " ";
	// 			printNext();
	// 			printNext();
	// 		}
	// 		if (current == last) {
	// 			if (!printedStar) {
	// 				printedStar = true;
	// 				std::cout << "*" << std::endl;
	// 			}
	// 		}
	// 		else {
	// 			last = current;
	// 			std::cout << hexToStrPadded((printedCount - 1) / 16, 8) << "0";
	// 			std::cout << current << std::endl;
	// 			printedStar = false;
	// 		}
	// 		current = "";
	// 	}
	// 	/// strange bug here if args is not a reference 
	// }
}