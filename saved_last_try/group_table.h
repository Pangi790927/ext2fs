#ifndef GROUP_TABLE_H
#define GROUP_TABLE_H

#include <exception>
#include "group.h"
#include "round_util.h"

/* each group has BLK_SIZE * 8 blocks in total */
struct GroupTable {
	std::vector<Group> groups;
	Ext2Dev &ext_dev;
	uint8_t *_ptr = NULL;
	uint32_t start_blk = (BLK_SIZE == 1024 ? 1 : 0);
	uint32_t blk_cnt = -1;
	uint32_t ino_per_grp;

	GroupTable(Ext2Dev &ext_dev) : ext_dev(ext_dev)
	{
		blk_cnt = ext_dev.get_blk_cnt() - (BLK_SIZE == 1024);
		uint32_t blk_per_grp = BLK_SIZE * 8;
		uint32_t desc_per_blk = BLK_SIZE / sizeof(blk_grp_desc_t);
		uint32_t grp_cnt = roundup_div(blk_cnt, blk_per_grp);

		uint32_t min_grp_blk_cnt;
		if (grp_cnt > 1) {
			/* a group must have at least space for a backup
			(desc table + superblk) and inode + block bitmaps */
			min_grp_blk_cnt = BLK_SIZE / desc_per_blk * grp_cnt + 1 + 2;
		}
		else {
			/* if we have only one group we know we have space for it */
			min_grp_blk_cnt = 0;
		}

		for (int i = 0; i < grp_cnt; i++) {
			uint32_t grp_start_blk = start_blk + blk_per_grp * i;
			uint32_t grp_blk_cnt = blk_cnt - (start_blk + blk_per_grp * i);
			if (grp_blk_cnt < min_grp_blk_cnt) {
				grp_cnt--;
				continue ;
			}
			if (grp_blk_cnt > blk_per_grp)
				grp_blk_cnt = blk_per_grp;
			groups.emplace_back(ext_dev, i, grp_start_blk, grp_blk_cnt,
					roundup_div(grp_cnt, desc_per_blk));
		}

		auto &desc = get_desc_table();
		desc.grp_cnt = groups.size();
		for (int i = 0; i < desc.grp_cnt; i++) {
			if (desc.ptr()[i].blk_bitmap != 0) {
				/* this means the table is already initialized */
				continue ;
			}
			desc.ptr()[i].blk_bitmap = groups[i].blk_bitmap.get_first_blk();
			desc.ptr()[i].ino_bitmap = groups[i].ino_bitmap.get_first_blk();
			desc.ptr()[i].ino_table = groups[i].ino_table.get_first_blk();
			desc.ptr()[i].free_blk_cnt = groups[i].blk_table.size();
			desc.ptr()[i].free_ino_cnt = groups[i].ino_table.size();
			desc.ptr()[i].used_dir_cnt = 0;
			groups[i].set_desc(desc.ptr() + i);
		}
		ino_per_grp = groups[0].ino_table.size();
	}

	int alloc_inode(int req_inode = -1) {
		if (req_inode < 0) {
			auto &desc = get_desc_table();
			for (int i = 0; i < groups.size(); i++) {
				if (desc.ptr()[i].free_ino_cnt) {
					int rel_ino = groups[i].ino_bitmap.get_first_free();
					req_inode = get_ino_abs(i, rel_ino);
					break;
				}
			}
			if (req_inode < 0)
				return -1;
		}
		auto [req_grp, rel_ino] = get_ino_rel(req_inode);
		if (req_grp < 0 || rel_ino < 0)
			return -1;
		if (groups[req_grp].has_inode(rel_ino))
			return -1;
		int res = groups[req_grp].alloc_inode(rel_ino);
		commit_descriptors();
		return req_inode;
	}

	int alloc_block(int req_block = -1) {
		if (req_block < 0) {
			auto &desc = get_desc_table();
			for (int i = 0; i < groups.size(); i++) {
				if (desc.ptr()[i].free_blk_cnt) {
					int rel_blk = groups[i].blk_bitmap.get_first_free();
					req_block = get_blk_abs(i, rel_blk);
					break;
				}
			}
			if (req_block < 0)
				return -1;
		}
		auto [req_grp, rel_blk] = get_blk_rel(req_block);
		if (req_grp < 0 || rel_blk < 0)
			return -1;
		if (groups[req_grp].has_block(rel_blk))
			return -1;
		int res = groups[req_grp].alloc_block(rel_blk);
		commit_descriptors();
		return req_block;
	}

	int free_inode(int ino) {
		auto [req_grp, rel_ino] = get_ino_rel(ino);
		if (req_grp < 0 || rel_ino < 0)
			return -1;
		int res = groups[req_grp].free_inode(rel_ino);
		commit_descriptors();
		return res;
	}

	uint32_t free_block(int blk) {
		auto [req_grp, rel_blk] = get_blk_rel(blk);
		if (req_grp < 0 || rel_blk < 0)
			return -1;
		int res = groups[req_grp].free_block(rel_blk);
		commit_descriptors();
		return res;
	}

	bool has_inode(int ino) {
		auto [req_grp, rel_ino] = get_blk_rel(ino);
		if (req_grp < 0 || rel_ino < 0)
			return false;
		return groups[req_grp].has_inode(rel_ino);
	}

	InoScope get_inode(int ino) {
		auto [req_grp, rel_ino] = get_blk_rel(ino);
		if (req_grp < 0 || rel_ino < 0)
			throw std::runtime_error("PANIC");
		return groups[req_grp].get_inode(rel_ino);
	}

	std::pair<int, int> get_ino_rel(int ino) {
		if (ino < 0)
			return {-1, -1};
		if (ino / ino_per_grp > groups.size())
			return {-1, -1};
		return {ino / ino_per_grp, ino % ino_per_grp};
	}

	std::pair<int, int> get_blk_rel(int blk) {
		for (int i = 0; i < groups.size(); i++)
			if (blk > groups[i].blk_table.size())
				blk -= groups[i].blk_table.size();
			else
				return {i, blk};
		return {-1, -1};
	}

	void commit_descriptors() {
		/* TO DO */
	}

	int get_ino_abs(int grp, int rel_ino) {
		if (grp < 0 || rel_ino < 0)
			return -1;
		if (grp >= groups.size())
			return -1;
		return grp * ino_per_grp + rel_ino;
	}

	int get_blk_abs(int grp, int rel_blk) {
		int blk = 0;
		if (grp == 0 && rel_blk == 0)
			return 0;
		for (int i = 0; i < groups.size(); i++)
			if (i == grp)
				return blk + rel_blk;
			else
				blk += groups[i].blk_table.size();
		if (blk == 0)
			return -1;
		return blk;
	}

	GroupDescTable &get_desc_table() {
		return groups[0].grp_desc_table_back; 
	}

	size_t size() {
		return groups.size();
	}

	auto begin() {
		return groups.begin();
	}

	auto end() {
		return groups.end();
	}
};

#endif