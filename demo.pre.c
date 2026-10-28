# 0 "demo.c"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 0 "<command-line>" 2
# 1 "demo.c"
# 1 "sum.h" 1
int sum(int a, int b)
{
  return a + b;
}
# 2 "demo.c" 2

void __exit(void)
{
    __asm__(
        "     mov rax, 60\n"
  );
}

int main(void)
{
  int a = 10;
  int b = 20;
  int c = sum(a, b);
  __exit();
  return 0;
}

int _start(void)
{
  return main();
}
