#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include "../src/aiowdm_ioctl.h"


int main (int argc, char **argv)
{
  int fd = -1;
  volatile __uint8_t *bar_mem = NULL;
  int i;
  struct aiowdm_card_info card_info = { 0 };
  char name[1024] = {0};
  int status = 0;

  fd = open ("/dev/aiowdm/mPCIe-II-16", O_RDWR);
  if ( fd < 0)
  {
    printf("Unable to open device file\n");
    return 1;
  }

  //card_info.name = name;
  //card_info.name_size = sizeof(name);
  status = ioctl(fd, AIOWDM_CARD_INFO_GET, &card_info);

  if (status)
  {
    perror("ioctl error:");
  }

  printf("card_info->device_id = 0x%x\n", card_info.device_id);
  printf("card_info->port_base = 0x%x\n", card_info.port_base);
  printf("card_info->name = %s\n", card_info.name);


  close(fd);

  return 0;
}