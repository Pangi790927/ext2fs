
all:
	g++-7 *.cpp */*.cpp -I utils

test:
	dumpe2fs hdd.ext2

hdd.ext2:
	dd if=/dev/zero of=hdd.ext2 bs=1k count=32