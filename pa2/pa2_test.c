
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define STR_SIZE (256)

void dev_read(int dev_num)
{
  int num_bytes = 0;
  printf("Enter the number of bytes you want to read:\n");
  scanf("%d", &num_bytes);

  // flush stdin
  char ignore;
  while( (ignore = getchar()) != '\n' );

  char buf[STR_SIZE];
  pread(dev_num, buf, num_bytes, 0);
  buf[num_bytes] = 0;
  printf("%s\n", buf);
}

void dev_write(int dev_num)
{
  char str_in[STR_SIZE];
  printf("Enter the data you want to write to the device:\n");
  fgets(str_in, STR_SIZE - 1, stdin);
  write(dev_num, str_in, strlen(str_in));
}

void dev_seek(int dev_num)
{
  int pos = 0, whence = 0;
  printf("Enter an offset value:\n");
  scanf("%d", &pos);
  printf("Enter a value for whence (thrid parameter):\n");
  scanf("%d", &whence);

  // flush stdin
  char ignore;
  while( (ignore = getchar()) != '\n' );

  lseek(dev_num, pos, whence);
}

void dev_exit(int dev_num)
{
  printf("Exiting\n");
  close(dev_num);
}

int main()
{
  int dev_num = open("/dev/simple_character_device", O_RDWR);
  if(dev_num < 0)
  {
    printf("Failed to open device\n");
    return 0;
  }

  char char_in;
  printf("Hello!\n");

  while(1)
  {
    printf("Press r to read from device\n");
    printf("Press w to write to the device\n");
    printf("Press s to seek into the device\n");
    printf("Press e to exit from the device\n");
    printf("Press anything else to keep reading or writing from the device\n");
    printf("Enter command:\n");
    char_in = getchar();

    // flush stdin
    char ignore;
    while( (ignore = getchar()) != '\n' );

    switch(char_in){
      case 'r':
        dev_read(dev_num);
        break;
      case 'w':
        dev_write(dev_num);
        break;
      case 's':
        dev_seek(dev_num);
        break;
      case 'e':
        dev_exit(dev_num);
        return 0;
        break;
      default:
        break;
    }
  }
}
