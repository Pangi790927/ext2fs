
all:
	g++-7 ext2gen.cpp

hdd:
	dd if=/dev/zero of=hdd.ext2 bs=1M count=32