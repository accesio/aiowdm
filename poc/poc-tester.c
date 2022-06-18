#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

int main (int argc, char **argv)
{
  int fd = 0;
  u_int8_t *mapped_addr = NULL;

  if (argc != 2)
  {
    printf("Usage: %s <path/to/file>\n", argv[0]);
  }

  fd = open(argv[1], O_RDWR);

  if (fd < 0)
  {
    printf("Unable to open file errno = %d", errno);
    return -1;
  }

  mapped_addr = mmap(NULL, 256, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

  if (mapped_addr == MAP_FAILED)
  {
    printf("mmap failed. errno = %d", errno);
    return -1;
  }

  printf("mapped_addr = 0x%p\n", mapped_addr);

}
