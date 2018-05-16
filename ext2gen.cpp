#include <iostream>
#include <cstdio>
#include "hexdump.h"

struct __attribute__((__packed__)) Superblock {
	uint32_t inodes = 1000;
	uint32_t blocks = 65536;	// no of blocks
	uint32_t rblocks = 0;		//
	uint32_t un_blocks = 65536;	//
	uint32_t un_inodes = 1000;	// unallocated indoes
	uint32_t first_block = 1;	//
	uint32_t block_size = 10;	//
	uint32_t fragment_size = 10;//
	uint32_t blocks_per_group = 1;
	uint32_t fragments_per_group = 1;
	uint32_t inodes_per_group = 1;
	uint32_t last_mount_time = 1526484613;
	uint32_t last_written_time = 1526484613;
	uint16_t mount_count = 2;
	uint16_t max_mount_count = 10;
	uint16_t ext2_signature = 0xef53;
	uint16_t fs_state = 1;		// no errors
	uint16_t fs_resolution = 1;	// continue
	uint16_t version_minor = 0;
	uint32_t last_consistency_time = 1526484613;
	uint32_t int_consistency_time = 100000;
	uint32_t os_creator_type = 4;
	uint32_t version_major = 1;
	uint16_t reserved_user_id = 0;
	uint16_t reserved_group_id = 0;
};

int main (int argc, char const *argv[]) {
	//int abc[100] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	Superblock super;

	FILE * pFile;
	pFile = fopen("hdd.ext2" , "r+");
	fseek(pFile, 1024, SEEK_SET);
	fwrite(&super, 1, sizeof(super), pFile);
	fclose(pFile);

	//util::hexdump(abc, sizeof(abc));
	return 0;
}