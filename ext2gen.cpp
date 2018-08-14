#include <iostream>
#include <cstdio>
#include "hexdump.h"

struct __attribute__((__packed__)) Superblock {
    uint32_t inodes = 1000;
    uint32_t blocks = 65536;    // no of blocks
    uint32_t rblocks = 0;        //
    uint32_t un_blocks = 65536;    //
    uint32_t un_inodes = 1000;    // unallocated indoes
    uint32_t first_block = 1;    //
    uint32_t block_size = 0;    //
    uint32_t fragment_size = 0; //
    uint32_t blocks_per_group = 1;
    uint32_t fragments_per_group = 1;
    uint32_t inodes_per_group = 1;
    uint32_t last_mount_time = 1526484613;
    uint32_t last_written_time = 1526484613;
    uint16_t mount_count = 2;
    uint16_t max_mount_count = 10;
    uint16_t ext2_signature = 0xef53;
    uint16_t fs_state = 1;        // no errors
    uint16_t fs_resolution = 1;    // continue
    uint16_t version_minor = 1;
    uint32_t last_consistency_time = 1526484613;
    uint32_t int_consistency_time = 100000;
    uint32_t os_creator_type = 4;
    uint32_t version_major = 0;
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

void readFs(std::string &location) {
    Superblock super;

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
    Superblock super;

    std::cout << super << std::endl;

    FILE * pFile;
    pFile = fopen("hdd.ext2" , "r+");
    fseek(pFile, 1024, SEEK_SET);
    fwrite(&super, 1, sizeof(super), pFile);
    fclose(pFile);

    std::string ext4("/dev/sda2");
    readFs(ext4);

    std::string test2("hdd2.ext2");
    readFs(test2);

    std::string test("hdd.ext2");
    readFs(test);

    //util::hexdump(abc, sizeof(abc));
    return 0;
}