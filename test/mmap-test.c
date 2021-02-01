#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

#include "../module/aiowdm_ioctl.h"

int main (int argc, char **argv)
{
  int fd = open (argv[1], O_RDWR);
  volatile void *mmap_addr;
  uint8_t val = 0xaa;

  printf("fd = %d\n", fd);

  if (fd < 0)
  {
    exit(1);
  }

  mmap_addr = mmap(NULL, 64, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (mmap_addr == NULL) 	{
    printf("mmap_addr is NULL\n");
    return -1;
  }
  else
  {
    printf("mmap succeeded\n");
  }

  val = ((uint8_t *)mmap_addr)[0];
  printf("Read of Base address = %d\n", val);

  val = ((uint8_t *)mmap_addr)[1];
  printf("Read of Base[1] address = %d\n", val);



  close(fd);
  return 0;
}