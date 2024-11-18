A good readMe

> rm -rf build
> mkdir build && cd build
> cmake ..
> make


Для створення обєкта (із файликом + mnt шлях):


> dd if=/dev/zero of=fat32.img bs=1M count=64
> mkfs.fat -F 32 fat32.img
> sudo mkdir -p /mnt/fat32
> sudo mount -o loop,rw,uid=$(id -u),gid=$(id -g) fat32.img /mnt/fat32
> echo "this is aome text for test file" > /mnt/fat32/test_file.txt
> ls /mnt/fat32
> ./FATChecker fat32.img
