#ifndef BLOCK_SCOPE_H
#define BLOCK_SCOPE_H

#include "ext2_dev.h"

struct BlockScope {
	/* This tructure will only be moved, will have a pointer to the
	respective block and it will write it back on destruction if not
	specified otherwise  */
	std::map<uint32_t, uint32_t> &aquire_cnt;
	Ext2Dev *ext_dev = NULL;
	uint32_t block;
	uint8_t *_ptr = NULL;
	bool write_back = true;
	bool active_instance = true;

	BlockScope(const BlockScope&) = delete;
	BlockScope& operator = (const BlockScope&) = delete;
	BlockScope& operator = (BlockScope&& oth) = delete;
	
	BlockScope(BlockScope&& oth) : aquire_cnt(oth.aquire_cnt) {
		oth.active_instance = false;
		this->active_instance = true;

		this->ext_dev = oth.ext_dev;
		this->_ptr = oth._ptr;
		this->write_back = oth.write_back;
		this->block = oth.block;
	}

	BlockScope(Ext2Dev *ext_dev, uint32_t block,
			std::map<uint32_t, uint32_t> &aquire_cnt, bool write_back = true)
	: ext_dev(ext_dev), block(block), aquire_cnt(aquire_cnt),
			write_back(write_back)
	{
		aquire_cnt[block]++;
		_ptr = ext_dev->load_blk(block, 1);
	}

	~BlockScope() {
		if (active_instance) {
			if (write_back)
				ext_dev->commit_blk(block, 1);
			aquire_cnt[block]--;
			if (aquire_cnt[block] == 0)
				ext_dev->unload_blk(block, 1);
		}
	}

	void togle_write(bool value) {
		write_back = value;
	}

	uint32_t get_first_blk() {
		return block;
	}

	uint32_t get_blk_cnt() {
		return 1;
	}

	uint8_t *ptr() {
		return _ptr;
	}
};


#endif