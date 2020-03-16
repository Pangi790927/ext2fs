#ifndef SUPERBLOCK_H
#define SUPERBLOCK_H

struct Superblock {
	Ext2Dev &ext_dev;
	ext2_sup_t *_ptr = NULL;
	uint32_t start_blk = 0;
	uint32_t blk_cnt = 1;
	bool is_backup;
	bool is_original;

	Superblock(Ext2Dev &ext_dev, bool is_backup, uint32_t start_blk = 0)
	: ext_dev(ext_dev), is_backup(is_backup), is_original(is_original)
	{
		load();
	}

	uint32_t get_first_blk() {
		return start_blk;
	}

	uint32_t get_blk_cnt() {
		return blk_cnt;
	}

	ext2_sup_t *load() {
		if (start_blk == 0) {
			if (BLK_SIZE == 1024) {
				start_blk = 1;
				_ptr = (ext2_sup_t *)ext_dev.load_blk(start_blk);
			}
			else {
				start_blk = 0;
				_ptr = (ext2_sup_t *)(ext_dev.load_blk(start_blk) + 1024);
			}
		}
		if (is_backup) {
			_ptr = (ext2_sup_t *)ext_dev.load_blk(start_blk);
		}
		return _ptr;
	}

	ext2_sup_t *ptr() {
		return _ptr;
	}

	void unload() {
		if (start_blk == 0 || is_backup)
			ext_dev.unload_blk(start_blk);
	}

	void commit() {
		if (start_blk == 0 || is_backup)
			ext_dev.unload_blk(start_blk);
	}

	~Superblock() {
		if (start_blk == 0 || is_backup)
			unload();		
	}
};

#endif
