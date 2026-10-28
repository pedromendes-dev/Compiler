#include "sum.h"

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


