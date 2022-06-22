#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include "aiowdm.h"
#include "../module_old/aiowdm_ioctl.h"

#define AIOWDM_DEV_PATH "/dev/aiowdm/"

#define libaiowdm_err_print(fmt, ...) \
				do { printf( "%s:%d:%s(): " fmt "\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); } while (0);

#ifndef AIO_DEBUG
#define AIO_DEBUG 0
#endif

#define libaiowdm_debug_print(fmt, ...) \
				do { if (AIO_DEBUG) printf ("%s:%d:%s(): " fmt "\n" , __FILE__, \
																__LINE__, __func__, ##__VA_ARGS__); } while (0)

//TODO: remove when done with initial driver development
#define libaiowdm_dev_print(fmt, ...) \
				do { if (AIO_DEBUG) printf ("%s:%d:%s(): " fmt "\n" , __FILE__, \
																__LINE__, __func__, ##__VA_ARGS__); } while (0)



struct libaiowdm_context
{
  int fd;
  unsigned short int port_base;
  int irq_enabled;
  char name[64];
};

static struct libaiowdm_context *contexts;
static int context_count;

//internal functions
int fill_context (struct libaiowdm_context *context, int fd);


int aiowdm_card_open(const char *fname)
{
  int fd;
  char path[1024];
  struct libaiowdm_context *new_context = NULL;
  int i;
  struct aiowdm_card_info card_info;
  int handle;

  strcpy(path, AIOWDM_DEV_PATH);
  strncat(path, fname, sizeof(path) - strlen(AIOWDM_DEV_PATH));

  fd = open(path, O_RDWR);

  if ( fd < 0)
  {
    libaiowdm_err_print("Unable to open device file");
    return -1;
  }

  for ( i = 0 ; i < context_count ; i++)
  {
    if (contexts[i].fd < 0)
    {
      new_context = &contexts[i];
      handle = i;
      break;
    }
  }

  if (NULL == new_context)
  {
    handle = context_count;
    context_count++;
    contexts = realloc(contexts, context_count * sizeof(*contexts));
    new_context = &contexts[handle];
  }

  if (ioctl(fd, AIOWDM_CARD_INFO_GET, &card_info))
  {
    libaiowdm_err_print("Error retrieving card info\n");
    new_context->fd = -1;
    return -1;
  }

  new_context->fd = fd;
  new_context->port_base = card_info.port_base;
  strcpy(new_context->name, card_info.name);

  return fd;
}

int aiowdm_card_close  (int handle)
{
  if (contexts[handle].irq_enabled)
  {
    aiowdm_irq_disable(handle);
  }

  contexts[handle].fd = -1;

  context_count--;

  if ( 0 == context_count)
  {
    contexts = realloc(contexts, 0);

 }
 return 0;
}

