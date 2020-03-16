#ifndef EXT2_DEV_H
#define EXT2_DEV_H

struct Ext2Dev {
	Dev &dev;
	size_t start_lba;
	size_t lba_cnt;

	Ext2Dev(Dev &dev, size_t start_lba, size_t lba_cnt)
	: dev(dev), start_lba(start_lba), lba_cnt(lba_cnt) {}

	uint32_t get_blk_cnt() {
		return lba_cnt / (BLK_SIZE / SECTOR_SZ);
	}

	uint8_t *load_blk(uint32_t index, int cnt = 0) {
		uint32_t sec_cnt = BLK_SIZE / SECTOR_SZ;
		return dev.get_sect(index * sec_cnt + start_lba, sec_cnt * cnt);
	}

	void unload_blk(uint32_t index, int cnt = 0) {
		uint32_t sec_cnt = BLK_SIZE / SECTOR_SZ;
		return dev.unload_sect(index * sec_cnt + start_lba, sec_cnt * cnt);
	}

	void commit_blk(uint32_t index, int cnt = 0) {
		uint32_t sec_cnt = BLK_SIZE / SECTOR_SZ;
		return dev.commit_sect(index * sec_cnt + start_lba, sec_cnt * cnt);	
	}
};

#endif
