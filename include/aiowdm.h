#pragma once

#include <cstdint>

//https://accesio.com/MANUALS/Software%20Reference.pdf

namespace AIOWDM {

long int GetNumCards(void);
unsigned int QueryCardInfo(uint32_t CardNum, unsigned long *pDeviceID, unsigned long *pBase, unsigned long *pNameSize, unsigned char *pName);

uint8_t RelInPortB (uint32_t CardNum, uint32_t Register);
uint16_t RelInPort (uint32_t CardNum, uint32_t Register);
uint32_t RelInPortL (uint32_t CardNum, uint32_t Register);

int RelOutPortB (uint32_t CardNum, uint32_t Register, uint8_t Value);
int RelOutPort (uint32_t CardNum, uint32_t Register, uint16_t Value);
int RelOutPortL (uint32_t CardNum, uint32_t Register, uint32_t Value);

int WaitForIRQ (uint32_t CardNum);
int AbortRequest (uint32_t CardNum);
// returns the value from the interrupt status register during the most recent
// interrupt on applicable cards
int COSWaitForIRQ (uint32_t CardNum, uint32_t PPIs, void *pData);


//proposed APIs below. not finalized

int DmaBufferInit (uint32_t CardNum, uint8_t NumSlots, ssize_t SlotSize, volatile uint8_t *Base);
int DmaDataReady (uint32_t CardNum, int *StartIndex, int *Slots, int *DataDiscarded);
int DmaDataTaken (uint32_t CardNum, int Slots);


} //names AIOWDM
