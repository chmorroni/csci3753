#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>

#define SYS_HELLOWORLD (326)
#define SYS_CS3753_ADD (327)

int main()
{
  int res, num1 = 2, num2 = 4;
  syscall(SYS_CS3753_ADD, num1, num2, &res);
  printf("%d + %d = %d\n", num1, num2, res);
  return 0;
}
