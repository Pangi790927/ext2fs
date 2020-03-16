#ifndef BLOCK_TABLE_H
#define BLOCK_TABLE_H

#include "iterator.h"
#include "block_scope.h"

struct BlockTable {
	Ext2Dev *ext_dev;
	uint8_t *_ptr = NULL;
	uint32_t start_blk = 0;
	uint32_t blk_cnt = -1;

	std::map<uint32_t, uint32_t> aquire_cnt;

	BlockTable() {}

	BlockTable(Ext2Dev *ext_dev, uint32_t start_blk, uint32_t blk_cnt)
	: ext_dev(ext_dev), start_blk(start_blk), blk_cnt(blk_cnt) {}

	BlockScope aquire(uint32_t block) {
		return BlockScope(ext_dev, block, aquire_cnt);
	}

	uint32_t get_first_blk() {
		return start_blk;
	}

	uint32_t get_blk_cnt() {
		return blk_cnt;
	}

	uint32_t size() {
		return blk_cnt;
	}
};

#endif