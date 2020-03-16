#ifndef INODE_H
#define INODE_H

#include "ino_scope.h"
#include <set>

struct Inode {
	InoScope ino_scope;
	std::set<uint32_t> to_commit;

	Inode(InoScope&& ino_scope) : ino_scope(std::move(ino_scope)) {}

	void init(uint16_t mode, uint16_t uid = 0, uint16_t gid = 0) {
		inode_t *ino = ino_scope.ptr();
		ino->mode = mode;
		ino->gid = gid;
		ino->uid = uid;
		ino->size = 0;
		ino->atime = 0;
		ino->ctime = 0;
		ino->mtime = 0;
		ino->links_cnt = 0;
		ino->blocks = 0;
		ino->flags = 0;
		ino->generation = 0;
		ino->file_acl = 0;
		ino->dir_acl = 0;
		ino->faddr = 0;
	}

	template <typename FnAlloc, typename FnFree>
	void truncate(size_t new_size, FnAlloc&& alloc_block_fn,
			FnFree&& free_block_fn)
	{
		/* 0 is never a valid block */
		inode_t *ino = ino_scope.ptr();
		uint32_t old_size = ino->size;

		while (old_size < new_size)
			alloc_entry(++old_size, alloc_block_fn);
		while (old_size > new_size)
			free_entry(old_size--, free_block_fn);

		ino->size = new_size;
	}

	void read(char *dst, size_t file_off, size_t len) {
		if (len <= 0)
			return ;

		/* inclusive */
		auto entry_start = file_off / BLK_SIZE;
		auto entry_start_off = file_off % BLK_SIZE;

		/* inclusive */
		auto entry_end = (file_off + len - 1) / BLK_SIZE;
		auto entry_end_off = (file_off + len - 1) % BLK_SIZE;

		memcpy(dst,
				(char *)ino_scope.ext_dev().load_blk(get_entry(entry_start)) + 
				entry_start_off, BLK_SIZE - entry_start_off);
		ino_scope.ext_dev().unload_blk(get_entry(entry_start));
		dst += BLK_SIZE - entry_start_off;

		for (int i = entry_start + 1; i < entry_end; i++) {
			memcpy(dst, (char *)ino_scope.ext_dev().load_blk(get_entry(i)),
					BLK_SIZE);
			ino_scope.ext_dev().unload_blk(get_entry(i));
			dst += BLK_SIZE;
		}

		memcpy(dst, (char *)ino_scope.ext_dev().load_blk(get_entry(entry_end)),
				entry_end_off + 1);
		ino_scope.ext_dev().unload_blk(get_entry(entry_end));
	}

	void write(char *src, size_t file_off, size_t len) {
		if (len <= 0)
			return ;

		/* inclusive */
		auto entry_start = file_off / BLK_SIZE;
		auto entry_start_off = file_off % BLK_SIZE;

		/* inclusive */
		auto entry_end = (file_off + len - 1) / BLK_SIZE;
		auto entry_end_off = (file_off + len - 1) % BLK_SIZE;

		memcpy((char *)ino_scope.ext_dev().load_blk(get_entry(entry_start)) + 
				entry_start_off, src, BLK_SIZE - entry_start_off);
		to_commit.insert(get_entry(entry_end));
		src += BLK_SIZE - entry_start_off;

		for (int i = entry_start + 1; i < entry_end; i++) {
			memcpy((char *)ino_scope.ext_dev().load_blk(get_entry(i)), src,
					BLK_SIZE);
			to_commit.insert(get_entry(i));
			src += BLK_SIZE;
		}

		memcpy((char *)ino_scope.ext_dev().load_blk(get_entry(entry_end)), src,
				entry_end_off + 1);
		to_commit.insert(get_entry(entry_end));
	}

	void commit() {
		/* TO DO take all the used blocks and save their data */
		for (auto&& blk : to_commit)
			ino_scope.ext_dev().unload_blk(blk);
	}

private:
	uint32_t root_of(uint32_t node) {
		while (node >= 256)
			node /= 256;
		return node;
	}

	uint32_t rest_of(uint32_t node) {
		uint32_t to_decrease = 1;
		while (to_decrease * 256 < node)
			to_decrease *= 256;
		if (to_decrease == 1)
			to_decrease = 0;
		return node - to_decrease;
	}

	uint32_t get_entry(uint32_t entry) {
		inode_t *ino = ino_scope.ptr();

		if (entry <= 11)
			return ino->block[entry];

		int indir_level = -1;
		if (entry >= 12 && entry < 12 + 256)
			indir_level = 1;
		if (entry >= 12 + 256 && entry < 12 + 256 + 65536)
			indir_level = 2;
		if (entry >= 12 + 256 + 65536 && entry < 12 + 256 + 65536 + 16777216)
			indir_level = 3;

		auto indir = (ext2_indir_t *)ino_scope.ext_dev().load_blk(
				ino->block[11 + indir_level]);
		while (true) {
			indir = (ext2_indir_t *)ino_scope.ext_dev().load_blk(
					ino->block[root_of(entry)]);

			if (indir_level == 1)
				return indir->block[root_of(entry)];

			entry = rest_of(entry);
			indir_level--;
		}
		return 0;
	}

	template <typename Fn>
	void alloc_entry(uint32_t entry, Fn&& alloc_block_fn) {
		int unload_list_size = 0;
		uint32_t unload_list[5] = {0};
		inode_t *ino = ino_scope.ptr();
		if (entry <= 11) {
			ino->block[entry] = alloc_block_fn();
			return ;
		}
		int indir_level = -1;
		if (entry >= 12 && entry < 12 + 256)
			indir_level = 1;
		if (entry >= 12 + 256 && entry < 12 + 256 + 65536)
			indir_level = 2;
		if (entry >= 12 + 256 + 65536 && entry < 12 + 256 + 65536 + 16777216)
			indir_level = 3;

		if (ino->block[11 + indir_level] == 0)
			ino->block[11 + indir_level] = alloc_indir(alloc_block_fn);
		auto indir = (ext2_indir_t *)ino_scope.ext_dev().load_blk(
				ino->block[11 + indir_level]);
		unload_list[unload_list_size++] = ino->block[11 + indir_level];
		entry -= 12;

		while (indir_level >= 1) {
			if (indir->block[root_of(entry)] == 0) {
				if (indir_level != 1) {
					indir->block[root_of(entry)] = alloc_indir(alloc_block_fn);
				}
				else {
					indir->block[root_of(entry)] = alloc_block_fn();
				}
			}
			indir = (ext2_indir_t *)ino_scope.ext_dev().load_blk(
					indir->block[root_of(entry)]);
			unload_list[unload_list_size++] = indir->block[root_of(entry)];
			entry = rest_of(entry);
			indir_level--;
		}

		for (int i = 0; i < unload_list_size; i++)
			ino_scope.ext_dev().unload_blk(i);
	}

	template <typename Fn>
	void free_entry(uint32_t entry, Fn&& free_block_fn) {
		int unload_list_size = 0;
		uint32_t unload_list[5] = {0};

		inode_t *ino = ino_scope.ptr();
		if (entry <= 11) {
			free_block_fn(ino->block[entry]);
			return ;
		}
		int indir_level = -1;
		if (entry >= 12 && entry < 12 + 256)
			indir_level = 1;
		if (entry >= 12 + 256 && entry < 12 + 256 + 65536)
			indir_level = 2;
		if (entry >= 12 + 256 + 65536 && entry < 12 + 256 + 65536 + 16777216)
			indir_level = 3;

		auto indir = (ext2_indir_t *)ino_scope.ext_dev().load_blk(
				ino->block[11 + indir_level]);
		unload_list[unload_list_size++] = ino->block[11 + indir_level];
		entry -= 12;
		if (entry == 0)
			free_indir(ino->block[11 + indir_level], free_block_fn);

		std::function<bool(ext2_indir_t *, int, uint32_t)> rec_free;
		rec_free = [&free_block_fn, this, &rec_free]
				(ext2_indir_t *ind, int ind_level, uint32_t entry)
		{
			if (ind_level == 1) {
				free_block_fn(ind->block[root_of(entry)]);
				return true;
			}

			bool need_free = rec_free(
					(ext2_indir_t *)ino_scope.ext_dev().load_blk(
							ind->block[root_of(entry)]),
					ind_level - 1, rest_of(entry));
			if (need_free) {
				if (root_of(entry) == 0) {
					free_indir(ind->block[root_of(entry)], free_block_fn);
					return true;
				}
			}
			return false;
		};

		if (rec_free(indir, entry, indir_level))
			free_indir(ino->block[root_of(entry)], free_block_fn);

		for (int i = 0; i < unload_list_size; i++)
			ino_scope.ext_dev().unload_blk(i);
	}

	template <typename Fn>
	uint32_t alloc_indir(Fn&& alloc_block_fn) {
		uint32_t blk = alloc_block_fn();
		auto indir = (ext2_indir_t *)ino_scope.ext_dev().load_blk(blk, 1);
		memset(indir, 0, sizeof(ext2_indir_t));
		ino_scope.ext_dev().unload_blk(blk, 1);
		to_commit.insert(blk);
		return blk;
	}

	template <typename Fn>
	void free_indir(uint32_t blk, Fn&& free_block_fn) {
		free_block_fn(blk);
		to_commit.erase(blk);
	}
};

#endif