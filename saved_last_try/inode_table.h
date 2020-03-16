#ifndef INODE_TABLE_H
#define INODE_TABLE_H

#include "ino_scope.h"

struct InodeTable {
	Ext2Dev *ext_dev;
	uint8_t *_ptr = NULL;
	uint32_t start_blk = 0;
	uint32_t blk_cnt = -1;

	std::map<uint32_t, uint32_t> aquire_cnt;

	InodeTable() {}

	InodeTable(Ext2Dev *ext_dev, uint32_t start_blk, uint32_t blk_cnt)
	: ext_dev(ext_dev), start_blk(start_blk), blk_cnt(blk_cnt) {}

	uint32_t get_first_blk() {
		return start_blk;
	}

	uint32_t get_blk_cnt() {
		return blk_cnt;
	}

	uint32_t size() {
		uint32_t ino_per_blk = BLK_SIZE / INO_SIZE;
		return blk_cnt * ino_per_blk;
	}

	InoScope aquire_ino(int rel_ino) {
		uint32_t ino_per_blk = BLK_SIZE / INO_SIZE;
		uint32_t block = rel_ino / ino_per_blk;
		return InoScope(BlockScope(ext_dev, block, aquire_cnt), rel_ino);
	}
};

#endif