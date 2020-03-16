all: clean ext2gen

ext2gen:
	g++-7 *.cpp */*.cpp -g -std=c++17 -rdynamic -lz -lstdc++fs -I utils -o ext2gen

test128K: example128K.img ext2gen
	./ext2gen example128K.img boot_example/dev_layout.json 
	sudo python testfs.py example128K.img

test2M: example2M.img ext2gen
	./ext2gen example2M.img boot_example/dev_layout.json 
	sudo python testfs.py example2M.img

test20M: example20M.img ext2gen
	./ext2gen example20M.img boot_example/dev_layout.json 
	sudo python testfs.py example20M.img

test200M: example200M.img ext2gen
	./ext2gen example200M.img boot_example/dev_layout.json 
	sudo python testfs.py example200M.img

example128K.img:
	dd if=/dev/zero of=example128K.img bs=1k count=128

example2M.img:
	dd if=/dev/zero of=example2M.img bs=1M count=2

example20M.img:
	dd if=/dev/zero of=example20M.img bs=1M count=20

example200M.img:
	dd if=/dev/zero of=example200M.img bs=1M count=200

clean:
	rm -f ext2gen
	rm -f example128K.img
	rm -f example2M.img
	rm -f example20M.img
	rm -f example200M.img
