#pragma once
#include <linux/ioctl.h>
#ifndef __KERNEL__
#include <stdint.h>
#include <linux/pci_regs.h>
#include <stdio.h>
#include <unistd.h>
#endif

#define ACCESIO_PCI_MAGIC 0x0E

#ifndef ACCESIO_PCI_MAGIC
#error ACCESIO_PCI_MAGIC  not defined.
#endif



//TODO: Consider moving mmap stuff to a different header?
#define ACCESIO_MMAP_OFFSET_DMA 0 /* ADIO DMA operations */
#define ACCESIO_MMAP_OFFSET_BUFF 1 /* Used for writing buffers DAC Waveform */

struct accesio_pci_card_descriptor
{
  uint16_t device_id;
  size_t port_base;
  size_t port_size; //TODO: Better name?
  unsigned long name_size;
  char name[64];
  size_t bars[PCI_STD_NUM_BARS];
};

struct accesio_pci_register_io_context
{
  bool read;
  int offset;
  int size;
  union
  {
    uint8_t byte;
    uint16_t word;
    uint32_t dword;
  }data;
};

#define ACCESIO_PCI_CARD_DESCRIPTOR_GET _IOR(ACCESIO_PCI_MAGIC, 1, struct accesio_pci_card_info *)
#define ACCESIO_PCI_IRQ_ENABLE _IO(ACCESIO_PCI_MAGIC, 2)
#define ACCESIO_PCI_IRQ_DISABLE _IO(ACCESIO_PCI_MAGIC, 3)
#define ACCESIO_PCI_IRQ_WAIT _IO(ACCESIO_PCI_MAGIC, 4)
#define ACCESIO_PCI_IRQ_WAIT_CANCEL _IO(ACCESIO_PCI_MAGIC, 5)
#define ACCESIO_PCI_REGISTER_IO _IOWR(ACCESIO_PCI_MAGIC, 6, struct accesio_pci_register_io_context *)

//proposed APIs below. not finalized
//ACCES recommends against accessing DMA related ioctls directly

//configure the ring buffer for DMA acquisition.
struct aiowdm_dma_init
{
  int num_slots;
  size_t slot_size;
};
#define ACCESIO_PCI_DMA_INIT _IOW(ACCESIO_PCI_MAGIC, 7, struct aiowdm_dma_init *)

//Get information about slots containing valid data.
struct aiowdm_dma_data_ready
{
  int start_index;
  int slots;
  int data_discarded;
};
#define ACCESIO_PCI_DMA_DATA_READY _IOR(ACCESIO_PCI_MAGIC, 8, struct aiowdm_dma_data_ready *)

//Tell the driver how many slots we're done processing
#define ACCESIO_PCI_DMA_DATA_DONE _IO(ACCESIO_PCI_MAGIC, 9)
