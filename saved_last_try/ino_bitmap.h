#ifndef INO_BITMAP_H
#define INO_BITMAP_H

#include "bitmap.h"

struct InoBitmap {
	Ext2Dev &ext_dev;
	Bitmap bitmap;
	uint8_t *_ptr = NULL;
	uint32_t start_blk = 0;
	uint32_t blk_cnt = 1;

	InoBitmap(Ext2Dev &ext_dev, uint32_t start_blk)
	: ext_dev(ext_dev), start_blk(start_blk)
	{
		_ptr = ext_dev.load_blk(start_blk, blk_cnt);
		bitmap = Bitmap(_ptr, BLK_SIZE);
	}

	~InoBitmap() {
		ext_dev.unload_blk(start_blk, blk_cnt);
	}

	void commit() {
		ext_dev.commit_blk(start_blk, blk_cnt);
	}

	uint32_t get_first_free() {
		return bitmap.get_first_free();
	}

	uint32_t get_first_blk() {
		return start_blk;
	}

	uint32_t get_blk_cnt() {
		return blk_cnt;
	}
};

#endif