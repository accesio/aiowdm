#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <dirent.h>
#include <errno.h>

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


struct card
{
	int fd;
	struct accesio_pci_card_descriptor *descriptor;
};

static struct card cards[32]; //TODO: Put MAX_CHAR_DEVICES in ioctl header
static int num_cards;
static bool init_complete;

void AiowdmInit () __attribute__((constructor));

void AiowdmInit ()
{
	libaiowdm_debug_print(">>>");
	DIR *dir;
	struct dirent *entry;
	char fname[512];
	int status;

	dir = opendir("/dev/accesio_pci/");

	if (NULL == dir)
	{
		libaiowdm_err_print("opendir failed: %d", errno);
		return;
	}

	while ((entry = readdir(dir)))
	{
		if (strstr(entry->d_name, "pci"))
		{
			sprintf(fname, "/dev/accesio_pci/%s", entry->d_name);
			cards[num_cards].fd = open(fname, O_RDWR);
			if (cards[num_cards].fd < 0)
			{
				libaiowdm_err_print("Unable to open file %s. errno=%d", fname, errno);
				continue;
			}
			cards[num_cards].descriptor = (struct accesio_pci_card_descriptor *) malloc(sizeof (struct accesio_pci_card_descriptor));
			status = ioctl(cards[num_cards].fd, ACCESIO_PCI_CARD_DESCRIPTOR_GET, cards[num_cards].descriptor);
			if (status)
			{
				libaiowdm_err_print("ioctl failed: %d", status);
				continue;
			}
			num_cards++;
		}
	}
	init_complete = true;
	libaiowdm_debug_print("<<<\n");
}

long int GetNumCards(void)
{
  return num_cards;
}

unsigned int QueryCardInfo(long CardNum, unsigned long *pDeviceID, unsigned long *pBase, unsigned long *pNameSize, unsigned char *pName)
{
	if (pDeviceID) *pDeviceID = cards[CardNum].descriptor->device_id;
	if (pBase) *pBase = cards[CardNum].descriptor->port_base;
	if ((NULL == pNameSize) && (NULL != pName)) return -EINVAL;
	if ((pNameSize != NULL) && (*pNameSize == 0)) *pNameSize = cards[CardNum].descriptor->name_size;
	else
	{
		strncpy ((char*)pName, cards[CardNum].descriptor->name, *pNameSize);
	}
  return 0;
}
