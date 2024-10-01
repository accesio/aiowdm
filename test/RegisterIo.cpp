#include <stdio.h>
#include <getopt.h>
#include<string>

#include "aiowdm.h"

void usage ()
{
  printf("Usage: RegisterIo [-c <card_num>][-s <size>] offset [<value>]\n");
  printf("offset and value are both always treated as hex.\n");
  printf("If value is provided it becomes a write. default is read\n");
}

int main (int argc, char **argv)
{
    int NumCards = AIOWDM::GetNumCards();
    long unsigned int device_id;
    long unsigned int base;
    unsigned char name[512];
    long unsigned int name_size = sizeof(name);
    int status = 0;
    int offset, value = 0, size = sizeof(uint32_t);
    bool write = false;
    int opt = 0;
    long card_num = 0;

    if (NumCards == 0)
    {
      printf("No cards detected\n");
    }

    while ((opt = getopt(argc, argv, "c:s:")) != -1 ) {
      switch(opt) {
        case 'c':
          card_num = std::stoi(optarg, nullptr, 16);
          break;
        case 's':
          size = std::stoi(optarg, nullptr, 16);
          break;
      }
    }

    if ((argc == optind + 1) || (argc == optind + 2))
    {
      offset = std::stoi(argv[optind], 0, 16);
      if (argc == optind + 2)
      {
        value = std::stoi(argv[optind + 1], 0, 16);
        write = true;
      }
    }
    else
    {
      usage();
      return -1;
    }

    printf("card_num=%ld\nsize=%d\noffset=0x%x\nvalue=0x%x\nwrite=%s\n", 
                                                card_num,
                                                size,
                                                offset,
                                                value,
                                                write ? "true" : "false");



    status = AIOWDM::QueryCardInfo(card_num, &device_id, &base, &name_size, name);

    if (status)
    {
      printf("status = %d\n", status);
      return 0;
    }

    if (write)
    {
      switch(size)
      {
        case sizeof(uint8_t):
          AIOWDM::RelOutPortB(card_num, offset, value & 0xff);
          break;
        case sizeof(uint16_t):
          AIOWDM::RelOutPort(card_num, offset, value & 0xffff);
          break;
        case sizeof(uint32_t):
          AIOWDM::RelOutPortL(card_num, offset, value & 0xffffffff);
          break;
        default:
          printf ("Invalid size\n");
          break;
      }
    }
    else
    {
      switch(size)
      {
        case sizeof(uint8_t):
          value = AIOWDM::RelInPort(card_num, offset);
          break;
        case sizeof(uint16_t):
          value = AIOWDM::RelInPort(card_num, offset);
          break;
        case sizeof(uint32_t):
          value = AIOWDM::RelInPortL(card_num, offset);
          break;
        default:
          printf ("Invalid size\n");
          break;
      }
      printf("value = 0x%x\n", value);
    }
}