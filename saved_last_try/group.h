#ifndef GROUP_H
#define GROUP_H

#include "superblock.h"
#include "group_desc_table.h"
#include "ino_bitmap.h"
#include "blk_bitmap.h"
#include "inode_table.h"
#include "block_table.h"
#include "round_util.h"

inline bool is_power_of(uint32_t num, uint32_t base) {
	while (num % base == 0 && num)
		num /= base;
	return num == 1;
}

struct Group {
	Superblock superblock_back;
	GroupDescTable grp_desc_table_back;
	blk_grp_desc_t *desc = NULL;
	InoBitmap ino_bitmap;
	BlkBitmap blk_bitmap;
	InodeTable ino_table;
	BlockTable blk_table;

	Ext2Dev &ext_dev;
	uint8_t *_ptr = NULL;

	uint32_t start_blk = 0;
	uint32_t blk_cnt = -1;
	uint32_t index = -1;

	Group(Ext2Dev &ext_dev, uint32_t index, uint32_t start_blk,
			uint32_t blk_cnt, uint32_t desc_blk_cnt)
	: 		ext_dev(ext_dev),
			index(index),
			start_blk(start_blk),
			blk_cnt(blk_cnt),
			superblock_back(ext_dev, has_backup(index), start_blk),
			grp_desc_table_back(ext_dev, has_backup(index), start_blk + 1,
					desc_blk_cnt),
			ino_bitmap(ext_dev, start_blk +
					(has_backup(index) ? desc_blk_cnt + 1 : 0)),
			blk_bitmap(ext_dev, start_blk + 1 +
					(has_backup(index) ? desc_blk_cnt + 1 : 0))
	{
		/* so, because I don't know the alg to chose those values I will do my own:
			- we have 8192 max blocks per group
			- there are 8 inodes per block maximum
			- so if we have 1 inode for every 4 blocks(a page entry btw) => we have
				8 inodes for 32 data blocks so we split our space in chunks of 33
				blocks
			- because we will asume first that each group has a backup(easier this
				way) => 8192 - 4 = 8188 from this we will compute 8188 / 33 = nr of
				inodes per group and the rest are the data blocks
		*/

		uint32_t avail_blk_cnt = blk_cnt + 2 -
				(has_backup() ? desc_blk_cnt + 1: 0);

		uint32_t ino_per_blk = BLK_SIZE / INO_SIZE;
		uint32_t mini_groups_blk_cnt = ino_per_blk * 4 + 1;
		
		uint32_t desired_ino_cnt = BLK_SIZE * 8 / mini_groups_blk_cnt;

		uint32_t ino_cnt = avail_blk_cnt < roundup_div(desired_ino_cnt, ino_per_blk)
				? avail_blk_cnt * ino_per_blk : desired_ino_cnt;
		uint32_t ino_blk_cnt = roundup_div(ino_cnt, ino_per_blk);
		uint32_t data_blk_cnt = avail_blk_cnt - ino_blk_cnt;

		uint32_t ino_start =  start_blk + 2 +
				(has_backup(index) ? desc_blk_cnt + 1 : 0);

		ino_table = InodeTable(&ext_dev, ino_start, ino_blk_cnt);
		blk_table = BlockTable(&ext_dev, ino_start + ino_blk_cnt, data_blk_cnt);
	}

	void set_desc(blk_grp_desc_t *desc) {
		this->desc = desc;
	}

	uint32_t get_first_blk() {
		return start_blk;
	}

	uint32_t get_blk_cnt() {
		return blk_cnt;
	}

	bool has_backup() {
		return has_backup(index);
	}

	int alloc_inode(int rel_ino) {
		ino_bitmap.bitmap.set(rel_ino, true);
		desc->free_ino_cnt--;
		return 0;
	}

	int alloc_block(int rel_blk) {
		blk_bitmap.bitmap.set(rel_blk, true);
		desc->free_blk_cnt--;
		return 0;
	}

	int free_inode(int rel_ino) {
		ino_bitmap.bitmap.set(rel_ino, true);
		desc->free_ino_cnt++;
		return 0;
	}

	int free_block(int rel_blk) {
		blk_bitmap.bitmap.set(rel_blk, false);
		desc->free_blk_cnt++;
		return 0;
	}

	bool has_inode(int rel_ino) {
		return ino_bitmap.bitmap.get(rel_ino);
	}

	InoScope get_inode(int rel_ino) {
		return ino_table.aquire_ino(rel_ino);
	}

	bool has_block(int rel_blk) {
		return blk_bitmap.bitmap.get(rel_blk);
	}

	static bool has_backup(int index) {
		return index == 0 || is_power_of(index, 3) || is_power_of(index, 5) ||
				is_power_of(index, 7);
	}
};

#endif