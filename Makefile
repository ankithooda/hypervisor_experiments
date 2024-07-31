boot_v1:
	nasm -f bin bootloaders/boot_v1.asm -o bin/boot_v1.bin

boot_v3:
	nasm -f bin bootloaders/boot_v3.asm -o bin/boot_v3.bin

boot_v4:
	nasm -f bin bootloaders/boot_v4.asm -o bin/boot_v4.bin

hello_kernel:
	nasm -f elf32 bootloaders/boot_v4.asm -o bin/boot_v4.o && gcc -m32 simple_bin.c bin/boot_v4.o -o bin/kernel.bin -nostdlib -ffreestanding -std=c99 -mno-red-zone -fno-exceptions -nostdlib -Wall -Wextra -Werror -T bootloaders/linker.ld

run_debian:
	qemu-system-x86_64 -m 2028M -enable-kvm -drive file=images/debian-12.6.0-amd64-netinst.iso,index=0,media=disk,format=raw
