#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>

#define AIOWDM_DEV_PATH "/dev/accesio/"

#include "aiowdm.h"
#include "accesio_pci_ioctl.h"

#define libaiowdm_err_print(fmt, ...) \
				do { printf( "%s:%d:%s(): " fmt "\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); } while (0);

#ifndef AIO_DEBUG
#define AIO_DEBUG 0
#endif

#ifndef AIO_VERBOSE
#define AIO_VERBOSE 0
#endif

#define libaiowdm_debug_print(fmt, ...) \
				do { if (AIO_DEBUG) printf ("%s:%d:%s(): " fmt "\n" , __FILE__, \
																__LINE__, __func__, ##__VA_ARGS__); } while (0)

//TODO: remove when done with initial driver development
#define libaiowdm_dev_print(fmt, ...) \
				do { if (AIO_VERBOSE) printf ("%s:%d:%s(): " fmt "\n" , __FILE__, \
																__LINE__, __func__, ##__VA_ARGS__); } while (0)


void AiowdmInit () __attribute__((constructor));

void AiowdmInit ()
{
	libaiowdm_debug_print(">>>");
	printf("<<<\n");
}


long int GetNumCards(void)
{
  return 0;
}

unsigned int QueryCardInfo(long CardNum, unsigned long *pDeviceID, unsigned long *pBase, unsigned long *pNameSize, unsigned char *pName)
{
  return 0;
}