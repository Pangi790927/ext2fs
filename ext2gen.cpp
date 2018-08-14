#include <iostream>
#include <cstdio>
#include "hexdump.h"

struct __attribute__((__packed__)) Superblock {
    uint32_t inodes = 8192;
    uint32_t blocks = 32768;        // no of blocks
    uint32_t rblocks = 0;           //
    uint32_t un_blocks = 32768;     //
    uint32_t un_inodes = 8192;      // unallocated indoes
    uint32_t first_block = 0;       //
    uint32_t block_size = 0;        //
    uint32_t fragment_size = 0;     //
    uint32_t blocks_per_group = 32768;
    uint32_t fragments_per_group = 32768;
    uint32_t inodes_per_group = 8192;
    uint32_t last_mount_time = 1526484613;
    uint32_t last_written_time = 1526484613;
    uint16_t mount_count = 11;
    uint16_t max_mount_count = 65535;
    uint16_t ext2_signature = 0xef53;
    uint16_t fs_state = 1;          // no errors
    uint16_t fs_resolution = 1;     // continue
    uint16_t version_minor = 0;
    uint32_t last_consistency_time = 1526484613;
    uint32_t int_consistency_time = 0;
    uint32_t os_creator_type = 4;
    uint32_t version_major = 1;
    uint16_t reserved_user_id = 0;
    uint16_t reserved_group_id = 0;

    friend std::ostream& operator << (std::ostream& stream,
            const Superblock& arg)
    {
        stream << "inodes:                   " << arg.inodes << std::endl;
        stream << "blocks:                   " << arg.blocks << std::endl;
        stream << "rblocks:                  " << arg.rblocks << std::endl;
        stream << "un_blocks:                " << arg.un_blocks << std::endl;
        stream << "un_inodes:                " << arg.un_inodes << std::endl;
        stream << "first_block:              " << arg.first_block << std::endl;
        stream << "block_size:               " << arg.block_size << std::endl;
        stream << "fragment_size:            " << arg.fragment_size << std::endl;
        stream << "blocks_per_group:         " << arg.blocks_per_group << std::endl;
        stream << "fragments_per_group:      " << arg.fragments_per_group << std::endl;
        stream << "inodes_per_group:         " << arg.inodes_per_group << std::endl;
        stream << "last_mount_time:          " << arg.last_mount_time << std::endl;
        stream << "last_written_time:        " << arg.last_written_time << std::endl;
        stream << "mount_count:              " << arg.mount_count << std::endl;
        stream << "max_mount_count:          " << arg.max_mount_count << std::endl;
        stream << "ext2_signature:           " << arg.ext2_signature << std::endl;
        stream << "fs_state:                 " << arg.fs_state << std::endl;
        stream << "fs_resolution:            " << arg.fs_resolution << std::endl;
        stream << "version_minor:            " << arg.version_minor << std::endl;
        stream << "last_consistency_time:    " << arg.last_consistency_time << std::endl;
        stream << "int_consistency_time:     " << arg.int_consistency_time << std::endl;
        stream << "os_creator_type:          " << arg.os_creator_type << std::endl;
        stream << "version_major:            " << arg.version_major << std::endl;
        stream << "reserved_user_id:         " << arg.reserved_user_id << std::endl;
        stream << "reserved_group_id:        " << arg.reserved_group_id << std::endl;
        
        return stream;
    }
};

struct __attribute__((__packed__)) SuperblockEx : public Superblock {
    uint32_t first_unreserved_ino = 11;
    uint16_t ino_size = 128;
    uint16_t group_ownership = 0;
    uint32_t opt_features = 0;
    uint32_t req_features = 0;
    uint32_t ro_features = 0;
    char fsid[16] = "ffffffffffff";
    char vol_name[16] = "Volume name";
    char last_mounted_path[64] = "/";
    uint32_t compr_alg = 0;
    uint8_t prealoc_blocks_file = 0;
    uint8_t prealoc_blocks_dir = 0;
    uint16_t empty_1 = 0;
    char journal_id[16] = "aaaaaaaaaaaa";
    uint32_t journal_inode = 0;
    uint32_t journal_device = 0;
    uint32_t head_orphan_inode = 0;

    friend std::ostream& operator << (std::ostream& stream,
            const SuperblockEx& arg)
    {
        operator << (stream, static_cast<const Superblock&>(arg));

        stream << std::endl << "\t### EXTENDED ###" << std::endl;
        stream << "first_unreserved_ino:     " << arg.first_unreserved_ino << std::endl;
        stream << "ino_size:                 " << arg.ino_size << std::endl;
        stream << "group_ownership:          " << arg.group_ownership << std::endl;
        stream << "opt_features:             " << arg.opt_features << std::endl;
        stream << "req_features:             " << arg.req_features << std::endl;
        stream << "ro_features:              " << arg.ro_features << std::endl;
        stream << "fsid:                     " << arg.fsid << std::endl;
        stream << "vol_name:                 " << arg.vol_name << std::endl;
        stream << "last_mounted_path:        " << arg.last_mounted_path << std::endl;
        stream << "compr_alg:                " << arg.compr_alg << std::endl;
        stream << "prealoc_blocks_file:      " << (int)arg.prealoc_blocks_file << std::endl;
        stream << "prealoc_blocks_dir:       " << (int)arg.prealoc_blocks_dir << std::endl;
        stream << "empty_1:                  " << arg.empty_1 << std::endl;
        stream << "journal_id:               " << arg.journal_id << std::endl;
        stream << "journal_inode:            " << arg.journal_inode << std::endl;
        stream << "journal_device:           " << arg.journal_device << std::endl;
        stream << "head_orphan_inode:        " << arg.head_orphan_inode << std::endl;

        return stream;
    }
};

void readFs(std::string &location) {
    SuperblockEx super;

    FILE * pFile;
    pFile = fopen(location.c_str(), "r+");

    if (pFile == NULL) {
        std::cout << "Couldn't open " << location << ". R u root?" << std::endl;
        return;
    }

    fseek(pFile, 1024, SEEK_SET);
    fread(&super, 1, sizeof(super), pFile);
    fclose(pFile);

    std::cout << "Reading: " << location << std::endl;
    std::cout << "====================================" << std::endl;
    std::cout << super;
}

int main (int argc, char const *argv[]) {
    SuperblockEx super;

    FILE * pFile;
    pFile = fopen("hdd.ext2" , "r+");
    fseek(pFile, 1024, SEEK_SET);
    fwrite(&super, 1, sizeof(super), pFile);
    fclose(pFile);

    std::string test2("hdd2.ext2");
    readFs(test2);

    std::string test("hdd.ext2");
    readFs(test);

    //util::hexdump(abc, sizeof(abc));
    return 0;
}
