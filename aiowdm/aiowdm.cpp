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

namespace AIOWDM {

struct card
{
	int fd;
	struct accesio_pci_card_descriptor *descriptor;
};

static struct card cards[32]; //TODO: Put MAX_CHAR_DEVICES in ioctl header
static uint32_t num_cards;
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

unsigned int QueryCardInfo(uint32_t CardNum, unsigned long *pDeviceID, unsigned long *pBase, unsigned long *pNameSize, unsigned char *pName)
{
	if (CardNum >= num_cards) return -EINVAL;
	if (pDeviceID) *pDeviceID = cards[CardNum].descriptor->device_id;
	if (pBase) *pBase = cards[CardNum].descriptor->port_base;
	if ((nullptr == pNameSize) && (nullptr != pName)) return -EINVAL;
	if ((pNameSize != nullptr) && (*pNameSize == 0)) *pNameSize = cards[CardNum].descriptor->name_size;
	if (pName) strncpy ((char*)pName, cards[CardNum].descriptor->name, *pNameSize);
  return 0;
}

uint8_t RelInPortB (uint32_t CardNum, uint32_t Register)
{
	int status = 0;
	libaiowdm_debug_print("<<<");
	struct accesio_pci_register_io_context context = {0};
	context.size = sizeof(uint8_t);
	context.read = true;
	context.offset = Register;
	status = ioctl(cards[CardNum].fd, ACCESIO_PCI_REGISTER_IO, &context);

	if (status)
	{
		libaiowdm_err_print("status=%d", status);
	}
	libaiowdm_debug_print(">>>");

	return context.data.byte;
}
uint16_t RelInPort (uint32_t CardNum, uint32_t Register)
{
	int status = 0;
	libaiowdm_debug_print("<<<");
	struct accesio_pci_register_io_context context = {0};
	context.size = sizeof(uint16_t);
	context.read = true;
	context.offset = Register;
	status = ioctl(cards[CardNum].fd, ACCESIO_PCI_REGISTER_IO, &context);

	if (status)
	{
		libaiowdm_err_print("status=%d", status);
	}
	libaiowdm_debug_print(">>>");

	return context.data.word;
}

uint32_t RelInPortL (uint32_t CardNum, uint32_t Register)
{
	int status = 0;
	libaiowdm_debug_print("<<<");
	struct accesio_pci_register_io_context context = {0};
	context.size = sizeof(uint32_t);
	context.read = true;
	context.offset = Register;
	status = ioctl(cards[CardNum].fd, ACCESIO_PCI_REGISTER_IO, &context);

	if (status)
	{
		libaiowdm_err_print("status=%d", status);
	}
	libaiowdm_debug_print(">>>");

	return context.data.dword;
}

int RelOutPortB (uint32_t CardNum, uint32_t Register, uint8_t Value)
{
	int status = 0;
	libaiowdm_debug_print("<<<");
	struct accesio_pci_register_io_context context = {0};
	context.read = false;
	context.size = sizeof(uint8_t);
	context.offset = Register;
	context.data.byte = Value;
	status = ioctl(cards[CardNum].fd, ACCESIO_PCI_REGISTER_IO, &context);
	libaiowdm_debug_print(">>>");
	return status;
}

int RelOutPort (uint32_t CardNum, uint32_t Register, uint16_t Value)
{
	int status = 0;
	libaiowdm_debug_print("<<<");
	struct accesio_pci_register_io_context context = {0};
	context.read = false;
	context.size = sizeof(uint16_t);
	context.offset = Register;
	context.data.word = Value;
	status = ioctl(cards[CardNum].fd, ACCESIO_PCI_REGISTER_IO, &context);
	libaiowdm_debug_print(">>>");
	return status;
}
int RelOutPortL (uint32_t CardNum, uint32_t Register, uint32_t Value)
{
	int status = 0;
	libaiowdm_debug_print("<<<");
	struct accesio_pci_register_io_context context = {0};
	context.read = false;
	context.size = sizeof(uint32_t);
	context.offset = Register;
	context.data.dword = Value;
	status = ioctl(cards[CardNum].fd, ACCESIO_PCI_REGISTER_IO, &context);
	libaiowdm_debug_print(">>>");
	return status;
}

int WaitForIRQ (uint32_t CardNum)
{
	int status = 0;
	libaiowdm_debug_print("<<<");
	status = ioctl(cards[CardNum].fd, ACCESIO_PCI_IRQ_WAIT);
	libaiowdm_debug_print(">>>");
	return status;
}

int AbortRequest (uint32_t CardNum)
{
	int status = 0;
	libaiowdm_debug_print("<<<");
	status = ioctl(cards[CardNum].fd, ACCESIO_PCI_IRQ_WAIT_CANCEL);
	libaiowdm_debug_print(">>>");
	return status;
}

int COSWaitForIRQ (uint32_t CardNum, uint32_t PPIs, void *pData)
{
	libaiowdm_debug_print("<<<");
	libaiowdm_err_print("Stub called");
	libaiowdm_debug_print(">>>");
	return -1;
}

} //namespace AIOWDM
