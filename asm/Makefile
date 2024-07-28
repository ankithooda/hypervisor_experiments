hello:
	mkdir -p bin && yasm -f elf64 -g dwarf2 -l bin/hello.lst -o bin/hello.o hello.asm

link_hello:
	ld -o bin/hello bin/hello.o

