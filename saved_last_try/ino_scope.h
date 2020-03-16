#ifndef INO_SCOPE_H
#define INO_SCOPE_H

#include "block_scope.h"

struct InoScope {
	BlockScope blk_scope;
	int rel_ino;

	InoScope(BlockScope&& blk_scope, int rel_ino)
	: blk_scope(std::move(blk_scope)), rel_ino(rel_ino) {}

	inode_t *ptr() {
		uint32_t ino_per_blk = BLK_SIZE / INO_SIZE;
		return &((inode_t *)blk_scope.ptr())[rel_ino % ino_per_blk];
	}

	Ext2Dev &ext_dev() {
		return *blk_scope.ext_dev;
	}
};

#endif