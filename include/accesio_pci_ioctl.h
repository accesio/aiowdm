#pragma once
#include <linux/ioctl.h>
#ifndef __KERNEL__
#include <stdint.h>
#include <linux/pci_regs.h>
#endif

#define ACCESIO_PCI_MAGIC 0x0E

#ifndef ACCESIO_PCI_MAGIC
#error ACCESIO_PCI_MAGIC  not defined.
#endif

struct accesio_pci_card_descriptor
{
  uint16_t device_id;
  size_t port_base;
  size_t port_size; //TODO: Better name?
  unsigned long name_size;
  char name[64];
  size_t bars[PCI_STD_NUM_BARS];
};


#define ACCESIO_PCI_CARD_DESCRIPTOR_GET _IOR(ACCESIO_PCI_MAGIC, 1, struct accesio_pci_card_info *)
#define ACCESIO_PCI_IRQ_ENABLE _IO(ACCESIO_PCI_MAGIC, 2)
#define ACCESIO_PCI_IRQ_DISABLE _IO(ACCESIO_PCI_MAGIC, 3)
#define ACCESIO_PCI_IRQ_WAIT _IO(ACCESIO_PCI_MAGIC, 4)
#define ACCESIO_PCI_IRQ_WAIT_CANCLE _IO(ACCESIO_PCI_MAGIC, 5)