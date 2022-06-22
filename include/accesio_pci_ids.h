#ifndef ACCESIO_PCI_IDS_H
#define ACCESIO_PCI_IDS_H

//ISR contexts: For cards that have device or functionality specific
//data that needs to be accessed in the ISR
struct dma_isr_context
{
       dma_addr_t dma_addr;
     void *dma_virt_addr;
     int dma_last_buffer; //last dma started to fill
     int dma_first_valid; //first buffer containing valid data
     int dma_num_slots;
     size_t dma_slot_size;
     int dma_data_discarded;
     spinlock_t dma_data_lock;



#endif //ACCESIO_PCI_IDS_H