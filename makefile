
all:
	g++-7 ext2gen.cpp

test:
	dumpe2fs hdd.ext2

hdd.ext2:
	dd if=/dev/zero of=hdd.ext2 bs=1M count=32