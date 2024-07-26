extern void kmain()
{
    const short color = 0xFFFF;
    const char* hello = "Hello from a C file!";
    short* vga = (short*)0xb8000;
    for (int i = 0; i<16;++i) {
        vga[i+80] = color | hello[i];
    }
}
