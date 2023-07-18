#include <linux/ioctl.h>
#ifndef __KERNEL__
#include <stdint.h>
#endif

#define AIOWDM_MAGIC 0x0E

#ifndef AIOWDM_MAGIC
#error AIOWDM_MAGIC  not defined.
#endif

struct aiowdm_bar_resource_info
{
  int bar;
  size_t start;
  size_t end;
  unsigned long flags;
//  struct resource resource;
};

struct aiowdm_card_info
{
  uint16_t device_id;
  unsigned short int port_base;
  size_t port_size; //TODO: Better name?
  unsigned long name_size;
  char name[64];
  struct aiowdm_bar_resource_info bars[PCI_STD_NUM_BARS];
};




#define AIOWDM_CARD_INFO_GET _IOR(AIOWDM_MAGIC, 1, struct aiowdm_card_info *)
#define AIOWDM_IRQ_ENABLE _IO(AIOWDM_MAGIC, 2)
#define AIOWDM_IRQ_DISABLE _IO(AIOWDM_MAGIC, 3)
#define AIOWDM_IRQ_WAIT _IO(AIOWDM_MAGIC, 4)
#define AIOWDM_IRQ_WAIT_CANCLE _IO(AIOWDM_MAGIC, 5)
