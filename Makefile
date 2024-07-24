simple_bin:
	mkdir -p bin && gcc -o bin/simple_bin simple_bin.c

simple_asm:
	mkdir -p bin && gcc -o bin/simple_asm -S simple_bin.c

simple_mem:
	mkdir -p bin && gcc -o bin/simple_mem simple_mem.c

run_mem:
	qemu-system-x86_64 -device loader,file=bin/simple_mem,cpu-num=0
