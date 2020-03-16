#ifndef GROUP_DESC_TABLE_H
#define GROUP_DESC_TABLE_H

struct GroupDescTable {
	Ext2Dev &ext_dev;
	blk_grp_desc_t *_ptr = NULL;
	uint32_t start_blk = -1;
	uint32_t blk_cnt = -1;
	uint32_t grp_cnt = 0;
	bool is_backup = false;

	GroupDescTable(Ext2Dev &ext_dev, bool is_backup, uint32_t start_blk,
			uint32_t blk_cnt)
	: ext_dev(ext_dev), start_blk(start_blk), blk_cnt(blk_cnt),
			is_backup(is_backup)
	{
		if (is_backup)
			_ptr = (blk_grp_desc_t *)ext_dev.load_blk(start_blk, blk_cnt);
	}

	~GroupDescTable() {
		if (is_backup)
			ext_dev.unload_blk(start_blk, blk_cnt);
	}

	blk_grp_desc_t *ptr() {
		return _ptr;
	}

	uint32_t get_first_blk() {
		return start_blk;
	}

	uint32_t get_blk_cnt() {
		return blk_cnt;
	}

	blk_grp_desc_t *begin() {
		return _ptr;
	}

	blk_grp_desc_t *end() {
		return _ptr + grp_cnt;
	}
};

#endif