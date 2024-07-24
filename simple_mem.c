int main() {
  int *addr = (int *)0x2000;
  *addr = 0x848;
  return 123;
}
