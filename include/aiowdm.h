#pragma once

long int GetNumCards(void);
unsigned int QueryCardInfo(long CardNum, unsigned long *pDeviceID, unsigned long *pBase, unsigned long *pNameSize, unsigned char *pName);
