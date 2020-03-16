#ifndef EXT2FS_H
#define EXT2FS_H

#include "dev.h"
#include "ext2.h"
#include "ext2_dev.h"
#include "group_table.h"
#include "superblock.h"
#include "round_util.h"
#include "inode.h"

/*
	Each structure has the following members:
		- get_first_blk()
		- get_blk_cnt()
		- load() -> returns a pointer
		- ptr() -> returns pointer to loaded memmory
		- unload() // will unload memory from dev
		- commit() // will write back to dev

	Some of the structures will be Ranges as documented in iterator.h
*/

inline uint32_t log2_int(uint32_t num) {
	uint32_t res = 0;
	while (num != 1) {
		res++;
		num >>= 1;
	}
	return res;
}

struct Ext2Fs {
	Ext2Dev ext2dev;
	GroupTable group_table;
	Superblock superblock;

	Ext2Fs(Dev &dev, size_t lba_start, size_t lba_cnt)
	: ext2dev(dev, lba_start, lba_cnt), group_table(ext2dev),
			superblock(ext2dev, false)
	{
		/* we read only our fs, so we don't know how to read someone else's
		file system. Others may read ours but if they don't put the same
		parameters as we put, this won't work */
		if (superblock.ptr()->magic == 0)
			fill_superblock();


		/* add backups */
		for (auto grp : group_table) {
			if (grp.has_backup()) {
				auto&& desc = group_table.get_desc_table();
				memcpy(grp.grp_desc_table_back.ptr(), desc.ptr(),
						BLK_SIZE * desc.get_blk_cnt());
				memcpy(grp.superblock_back.ptr(), superblock.ptr(),
						BLK_SIZE * desc.get_blk_cnt());
			}
		}

		// printf("%s\n", ext_sup_str(superblock.ptr()).c_str());
		printf("%s\n", ext_grp_table_str(group_table.get_desc_table().ptr(),
				group_table.size()).c_str());
	}

	void fill_superblock() {
		superblock.ptr()->blk_cnt = 0;
		for (auto&& grp : group_table.groups)
			superblock.ptr()->blk_cnt += grp.blk_table.size();
		
		superblock.ptr()->ino_cnt = 0;
		for (auto&& grp : group_table.groups)
			superblock.ptr()->ino_cnt += grp.ino_table.size();

		superblock.ptr()->ino_per_grp = group_table.groups[0].ino_table.size();
		superblock.ptr()->blk_per_grp = group_table.groups[0].blk_table.size();
		superblock.ptr()->reserved_blk_cnt = 0;
		superblock.ptr()->free_blk_cnt = superblock.ptr()->blk_cnt;
		superblock.ptr()->free_ino_cnt = superblock.ptr()->ino_cnt;

		/* we hardcode value 1024 here */
		superblock.ptr()->first_data_blk = superblock.start_blk;
		superblock.ptr()->log_blk_size = log2_int(BLK_SIZE) - 10;
		superblock.ptr()->log_frag_size = 0;

		superblock.ptr()->frag_per_grp = superblock.ptr()->blk_per_grp;

		superblock.ptr()->mtime = 0;
		superblock.ptr()->wtime = 0;

		superblock.ptr()->mnt_cnt = 0;
		superblock.ptr()->max_mnt_cnt = 0x7fff;

		superblock.ptr()->magic = EXT2_SUPER_MAGIC;
		superblock.ptr()->state = EXT2_VALID_FS;
		superblock.ptr()->errors = EXT2_ERRORS_CONTINUE;

		superblock.ptr()->rev_level = 1;
		superblock.ptr()->minor_rev_level = 0;

		superblock.ptr()->last_check = 0;
		superblock.ptr()->check_interval = 0x7fff'ffff;
		superblock.ptr()->creator_os = EXT2_OS_OS;
		superblock.ptr()->default_resuid = EXT2_DEF_RESUID;
		superblock.ptr()->default_resgid = EXT2_DEF_RESGID;

		/* ext2 revision specific */
		superblock.ptr()->ino_size = EXT2_GOOD_OLD_INODE_SIZE;
		superblock.ptr()->first_ino = EXT2_GOOD_OLD_FIRST_INO;

		/* used for backup */
		superblock.ptr()->blk_grp_nr = superblock.ptr()->first_data_blk;

		superblock.ptr()->feature_compat = 0;
		superblock.ptr()->feature_incompat = 0;
		superblock.ptr()->feature_ro_compat = EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER;
		strcpy((char *)superblock.ptr()->volume_name, "volume name");
		superblock.ptr()->algo_bitmap = 0;
	}

	int create_inode(uint16_t mode, uint16_t uid = 0,
			uint16_t gid = 0, int req_inode = -1)
	{
		if (req_inode == 0)
			return -1;
		if (req_inode > 0)
			req_inode -= 1;
		int ino = group_table.alloc_inode(req_inode);
		if (ino < 0)
			return -1;
		Inode inode(group_table.get_inode(ino));
		inode.init(mode, uid, gid);
		return ino + 1;
	}

	int delete_inode(int req_inode) {
		if (req_inode <= 0)
			return -1;
		req_inode--;
		return group_table.free_inode(req_inode);
	}

	inode_t get_inode_t(int ino) {
		if (ino <= 0)
			return {0};
		ino--;
		Inode inode(group_table.get_inode(ino));
		return *inode.ino_scope.ptr();
	}

	void set_inode_t(int ino, inode_t &inode) {
		/* this functions seems really unsafe and useless */
		if (ino <= 0)
			return ;
		ino--;
		Inode old_inode(group_table.get_inode(ino));
		*old_inode.ino_scope.ptr() = inode;
	}

	void truncate(int ino, size_t new_size) {
		/* TO DO: more error handling if there can be any */
		if (ino <= 0)
			return ;
		ino--;
		Inode inode(group_table.get_inode(ino));

		inode.truncate(new_size,
			[this]{
				return group_table.alloc_block();
			},
			[this](int blk){
				group_table.free_block(blk);
			});
		inode.commit();
	}

	void read(int ino, char *dst, size_t file_off, size_t len) {
		/* TO DO: more error handling if there can be any */
		if (ino <= 0)
			return ;
		ino--;
		Inode inode(group_table.get_inode(ino));
		inode.read(dst, file_off, len);
	}

	void write(int ino, char *src, size_t file_off, size_t len) {
		/* TO DO: more error handling if there can be any */
		if (ino <= 0)
			return ;
		ino--;
		Inode inode(group_table.get_inode(ino));
		inode.write(src, file_off, len);
	}
};

#endif