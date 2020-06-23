#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

#include "devices.h"
#include "aiowdm_ioctl.h"

MODULE_DESCRIPTION("Linux counterpart to AIOWDM");
MODULE_AUTHOR("ACCES");
MODULE_LICENSE("GPL");

#define aio_driver_err_print(fmt, ...) \
				do { printk( "%s:%d:%s(): " fmt "\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); } while (0);

#ifndef AIO_DEBUG
#define AIO_DEBUG 0
#endif

#define aio_driver_debug_print(fmt, ...) \
				do { if (AIO_DEBUG) printk ("%s:%d:%s(): " fmt "\n" , __FILE__, \
																__LINE__, __func__, ##__VA_ARGS__); } while (0)

//TODO: remove when done with initial driver development
#define aio_driver_dev_print(fmt, ...) \
				do { if (AIO_DEBUG) printk ("%s:%d:%s(): " fmt "\n" , __FILE__, \
																__LINE__, __func__, ##__VA_ARGS__); } while (0)


int aio_driver_pci_probe (struct pci_dev *dev, const struct pci_device_id *id);
void aio_driver_pci_remove (struct pci_dev *dev);
int  aio_driver_pci_suspend (struct pci_dev *dev, pm_message_t state);
int  aio_driver_pci_suspend_late (struct pci_dev *dev, pm_message_t state);
int  aio_driver_pci_resume_early (struct pci_dev *dev);
int  aio_driver_pci_resume (struct pci_dev *dev);
void aio_driver_pci_shutdown (struct pci_dev *dev);
int  aio_driver_pci_sriov_configure (struct pci_dev *dev, int num_vfs);


static struct pci_driver aio_pci_driver = {
.name = "aio_driver_pci",
.probe = aio_driver_pci_probe,
.remove = aio_driver_pci_remove,
.suspend=aio_driver_pci_suspend,
.suspend_late=aio_driver_pci_suspend_late,
.resume_early=aio_driver_pci_resume_early,
.resume=aio_driver_pci_resume,
.shutdown=aio_driver_pci_shutdown,
.sriov_configure=aio_driver_pci_sriov_configure,
};

static struct pci_device_id *aio_driver_pci_device_table;
#define AIO_DRIVER_PCI_TABLE_SIZE sizeof(struct pci_device_id) * (NUM_ACCES_PCI_DEVICES + 1)



ssize_t aio_driver_read(struct file *filp, char __user *buf, size_t count,
    loff_t *f_pos);
ssize_t aio_driver_write(struct file *filp, const char __user *buf, size_t count, 
    loff_t *fpos);
int aio_driver_open(struct inode *inode, struct file *filp);
int aio_driver_release(struct inode *inode, struct file *filp);
loff_t aio_driver_llseek(struct file *filp, loff_t off, int whence);
int aio_driver_mmap(struct file *filp, struct vm_area_struct *vma);
long aio_driver_ioctl (struct file *filp, unsigned int ioctl, unsigned long arg);

static struct file_operations aio_driver_fops =
{
  .owner = THIS_MODULE,
  .read = aio_driver_read,
  .write = aio_driver_write,
  .open = aio_driver_open,
  .release = aio_driver_release,
  .llseek = aio_driver_llseek,
  .mmap = aio_driver_mmap,
  .unlocked_ioctl = aio_driver_ioctl,
};

long ioctl_AIOWDM_CARD_INFO_GET (struct file *filp, unsigned long arg);
long ioctl_AIOWDM_BAR_RESOURCE_GET (struct file *filp, unsigned long arg);
long ioctl_AIOWDM_IRQ_ENABLE (struct file *filp, unsigned long arg);
long ioctl_AIOWDM_IRQ_DISABLE (struct file *filp, unsigned long arg);
long ioctl_AIOWDM_IRQ_WAIT (struct file *filp, unsigned long arg);
long ioctl_AIOWDM_IRQ_WAIT_CANCLE (struct file *filp, unsigned long arg);

int aio_driver_register_action(void *bar_bases[6], struct register_operation op);


#define AIO_CDEV_CLASS "aio-device"

static struct class *aio_driver_cdev_class;

#define AIO_DEV_PATH "aiowdm/"

//structure to hold per device information
struct aio_device_context
{
  //TODO: verify we need to hold onto the pci_dev
  //pci stuff
  struct pci_dev *pci_dev;
  const struct aio_pci_dev_cfg *dev_cfg;
  void *bar_bases[6]; //use ioport_map() and ioremap() to get these
  //cdev stuff
  struct cdev cdev;
  dev_t dev_major;
  struct device *device;
  int default_bar;
};

static int aio_driver_init(void)
{
	int status;
	int i;
	aio_driver_debug_print("Enter");

	aio_driver_pci_device_table = kmalloc(AIO_DRIVER_PCI_TABLE_SIZE, GFP_KERNEL);
	memset(aio_driver_pci_device_table, 0, AIO_DRIVER_PCI_TABLE_SIZE);

	for ( i = 0; i < NUM_ACCES_PCI_DEVICES ; i++)
	{
		aio_driver_pci_device_table[i].vendor = AIO_PCI_VEN_ID;
		aio_driver_pci_device_table[i].device = aio_pci_dev_table[i].pciDevId;
		aio_driver_pci_device_table[i].subvendor = PCI_ANY_ID;
		aio_driver_pci_device_table[i].subdevice = PCI_ANY_ID;
    aio_driver_pci_device_table[i].driver_data = (kernel_ulong_t)&aio_pci_dev_table[i];
	}

	aio_pci_driver.id_table = aio_driver_pci_device_table;

  aio_driver_cdev_class = class_create(THIS_MODULE, AIO_CDEV_CLASS);

  if (IS_ERR(aio_driver_cdev_class))
  {
    aio_driver_err_print("Error creating cdev class");
    goto err_cdev_create;
  }

	status = pci_register_driver(&aio_pci_driver);
	if (status)
	{
		aio_driver_err_print("registration failure: %d", status);
		goto err_register;
	}



	return 0;
err_cdev_create:
//TODO: clean up pci driver registration
err_register:
//TODO: magic number
	return -1;
}

static void aio_driver_exit(void)
{
	aio_driver_debug_print("Enter");
	pci_unregister_driver(&aio_pci_driver);
  class_destroy(aio_driver_cdev_class);
}

module_init(aio_driver_init);
module_exit(aio_driver_exit);

int aio_driver_pci_probe (struct pci_dev *dev, const struct pci_device_id *id)
{
  int status = 0;
  struct aio_device_context *context = NULL;
  int i = 0;
  unsigned long flags;
	aio_driver_debug_print("Enter");


	status = pci_request_regions(dev, "aio"); //TODO: Figure out if res_name should be defined somewhere

  if (status)
  {
    aio_driver_err_print("pci_request_region failed. status = 0x%x", status);
    goto err_out;
  }

  context = kmalloc(sizeof(struct aio_device_context), GFP_KERNEL);
  memset(context, 0, sizeof(struct aio_device_context));

  aio_driver_dev_print("context = %p", context);

  for (i = 0; i < 6 ; i++) //TODO:look into magic number for BARs
  {
    flags = pci_resource_flags(dev, i);
    if ( flags & ( IORESOURCE_IO | IORESOURCE_MEM))
    {
      context->bar_bases[i] = pci_iomap(dev, i, 0);
      if (NULL == context->bar_bases[i])
      {
        aio_driver_err_print("Could not map bar %d", i);
      }
      if (flags & (IORESOURCE_IO)) 
      {
        //This should match what the Windows driver comes up with for port base
        //in StartDevice() 
        context->default_bar = i;
      }
      aio_driver_dev_print("context->bar_bases[%d] = %p", i, context->bar_bases[i]);
    }
  }

  i = 0;

  do
  {
    if (aio_pci_dev_table[i].pciDevId == dev->device)
    {
      context->dev_cfg = &aio_pci_dev_table[i];
      aio_driver_dev_print("Found config entry");
    }
    i++;
  }while(NULL == context->dev_cfg && i < NUM_ACCES_PCI_DEVICES);

  //TODO: need to keep track of the names of created devices in case there is 
  //more than one card with the same name. 
  //TODO: Figure out if name should be defined somewhere. 
  status = alloc_chrdev_region(&context->dev_major, 0, 0, "aio");

  if (status)
  {
    aio_driver_err_print("Error allocating chrdev region: %d", status);
    goto err_free_context;
  }

  aio_driver_dev_print("context->dev_major: 0x%x", context->dev_major);

  cdev_init(&context->cdev, &aio_driver_fops);

  status = cdev_add(&context->cdev, context->dev_major, 1) ;

  if (status)
  {
    aio_driver_err_print("Error calling cdev_add(): %d", status);
    goto err_free_context;
  }

  context->device = device_create(aio_driver_cdev_class, NULL, context->dev_major, NULL,  AIO_DEV_PATH "%s", context->dev_cfg->Model);
  dev_set_drvdata(context->device, context);
  aio_driver_dev_print("context->device = %p", context->device);
  aio_driver_dev_print("device name: " AIO_DEV_PATH "%s", context->dev_cfg->Model); 
  if (IS_ERR(context->device))
  {
    aio_driver_err_print("Error calling device_create()");
    status = -EPERM;
    goto err_free_cdev;
  }
  context->pci_dev = dev;
  pci_enable_device(dev);
  pci_set_drvdata(dev, context);


return 0;
//TODO: setup the cleanup properly. 
err_free_cdev:
  pci_release_selected_regions(dev, 0x3f);
  cdev_del(&context->cdev);
err_free_context: 
  kfree(context);

err_out:
	return status;
}

void aio_driver_pci_remove (struct pci_dev *dev)
{
  struct aio_device_context * context = pci_get_drvdata(dev);
  aio_driver_err_print("context = %p", context);

   device_destroy(aio_driver_cdev_class, context->dev_major);
   cdev_del(&context->cdev);
   kfree(context);
  pci_release_selected_regions(dev, 0x3f);
  pci_disable_device(dev);
	return;
}

int  aio_driver_pci_suspend (struct pci_dev *dev, pm_message_t state)
{
  aio_driver_err_print("Entering stub");
  return 0;
}

int  aio_driver_pci_suspend_late (struct pci_dev *dev, pm_message_t state)
{
  aio_driver_err_print("Entering stub");
  return 0;
}

int  aio_driver_pci_resume_early (struct pci_dev *dev)
{
  aio_driver_err_print("Entering stub");
  return 0;
}

int  aio_driver_pci_resume (struct pci_dev *dev)
{
  aio_driver_err_print("Entering stub");
  return 0;
}

void aio_driver_pci_shutdown (struct pci_dev *dev)
{
  aio_driver_err_print("Entering stub");
}
int  aio_driver_pci_sriov_configure (struct pci_dev *dev, int num_vfs)
{
  aio_driver_err_print("Entering stub");
  return 0;
}


ssize_t aio_driver_read(struct file *filp, char __user *buf, size_t count,
    loff_t *f_pos)
{
  aio_driver_debug_print("Enter");
  aio_driver_dev_print("stubbed function");
  return 0;
}

ssize_t aio_driver_write(struct file *filp, const char __user *buf, size_t count, 
    loff_t *fpos)
{
  aio_driver_debug_print("Enter");
  aio_driver_dev_print("stubbed function");
  return 0;
}

int aio_driver_open(struct inode *inode, struct file *filp)
{
  struct aio_device_context *context;
  aio_driver_debug_print("Enter");
  aio_driver_dev_print("stubbed function. private_data = %p", filp->private_data);
  aio_driver_dev_print("stubbed function. inode->i_private = %p", inode->i_private);
  aio_driver_dev_print("inode->i_rdev = 0x%x", inode->i_rdev);
  aio_driver_dev_print("inode->i_cdev = %p", inode->i_cdev);
  context = container_of(inode->i_cdev, struct aio_device_context, cdev);
  aio_driver_dev_print("context = %p", context);
  filp->private_data = context;
  
  return 0;
}

int aio_driver_release(struct inode *inode, struct file *filp)
{
  aio_driver_debug_print("Enter");
  aio_driver_dev_print("stubbed function");
  return 0;
}

loff_t aio_driver_llseek(struct file *filp, loff_t off, int whence)
{
  aio_driver_debug_print("Enter");
  aio_driver_dev_print("stubbed function");
  return 0;
}

int aio_driver_mmap(struct file *filp, struct vm_area_struct *vma)
{
  struct aio_device_context *context = filp->private_data;
  aio_driver_debug_print("Enter");
  aio_driver_dev_print("context = %p", context);
  aio_driver_dev_print("pci_resource_start(context->pci_dev, 0) = 0x%llx", pci_resource_start(context->pci_dev, 0));
  aio_driver_dev_print("pci_resource_start(context->pci_dev, 1) = 0x%llx", pci_resource_start(context->pci_dev, 1));
  aio_driver_dev_print("pci_resource_start(context->pci_dev, 2) = 0x%llx", pci_resource_start(context->pci_dev, 2));

  // if (remap_pfn_range(vma, vma->vm_start, pci_resource_start(context->pci_dev, 2),
  //                      PAGE_SIZE, vma->vm_page_prot))
  // {
  //   aio_driver_err_print("io_remap_page_range failed\n");
  //   return -EAGAIN;
  // }
  if (vm_iomap_memory(vma, pci_resource_start(context->pci_dev, 2), PAGE_SIZE))
  {
    aio_driver_err_print("io_remap_page_range failed\n");
    return -EAGAIN;
  }
  aio_driver_debug_print("Exit");
  return 0;
}

long aio_driver_ioctl (struct file *filp, unsigned int ioctl, unsigned long arg)
{
  long status = 0;
  aio_driver_debug_print("Enter");


  switch (ioctl)
  {
    case AIOWDM_CARD_INFO_GET:
      status = ioctl_AIOWDM_CARD_INFO_GET(filp, arg);
      break;
    case AIOWDM_BAR_RESOURCE_GET:
      status = ioctl_AIOWDM_BAR_RESOURCE_GET (filp, arg);
      break;
    case AIOWDM_IRQ_ENABLE:
      status = ioctl_AIOWDM_IRQ_ENABLE (filp, arg);
      break;
    case AIOWDM_IRQ_DISABLE:
      status = ioctl_AIOWDM_IRQ_DISABLE (filp, arg);
      break;
    case AIOWDM_IRQ_WAIT:
      status = ioctl_AIOWDM_IRQ_WAIT (filp, arg);
      break;
    case AIOWDM_IRQ_WAIT_CANCLE:
      status = ioctl_AIOWDM_IRQ_WAIT_CANCLE (filp, arg);
      break;
    default:
      aio_driver_err_print("Invalid IOCTL call for device");
      status = -EINVAL;
  }
  return status;
}

long ioctl_AIOWDM_CARD_INFO_GET (struct file *filp, unsigned long arg)
{
  struct aiowdm_card_info *card_info = (struct aiowdm_card_info *)arg;
  struct aio_device_context *context = filp->private_data;
  unsigned short int port_base;
  aio_driver_debug_print("Enter");

  port_base = pci_resource_start(context->pci_dev, context->default_bar);

  copy_to_user(&card_info->device_id, &context->pci_dev->device, sizeof(uint16_t));
  copy_to_user(&card_info->port_base, &port_base, sizeof(unsigned short));
  //if ( card_info->name_size != 0)
  {
    aio_driver_dev_print("attempt to copy name. context->dev_cfg->Model = %s, strlen = %lu", context->dev_cfg->Model, strlen(context->dev_cfg->Model));
    aio_driver_dev_print("attempt to copy name. context->dev_cfg->Model = %p", context->dev_cfg->Model);

    copy_to_user(card_info->name, context->dev_cfg->Model, strlen(context->dev_cfg->Model));
    //strncpy(card_info->name, context->dev_cfg->Model, 11);
  }
  return 0;
}

long ioctl_AIOWDM_BAR_RESOURCE_GET (struct file *filp, unsigned long arg)
{
  aio_driver_debug_print("Enter");
  aio_driver_dev_print("Stubbed function");
  return -EPERM;
}

long ioctl_AIOWDM_IRQ_ENABLE (struct file *filp, unsigned long arg)
{
  aio_driver_debug_print("Enter");
  aio_driver_dev_print("Stubbed function");
  return -EPERM;
}

long ioctl_AIOWDM_IRQ_DISABLE (struct file *filp, unsigned long arg)
{
  aio_driver_debug_print("Enter");
  aio_driver_dev_print("Stubbed function");
  return -EPERM;
}

long ioctl_AIOWDM_IRQ_WAIT (struct file *filp, unsigned long arg)
{
  aio_driver_debug_print("Enter");
  aio_driver_dev_print("Stubbed function");
  return -EPERM;
}

long ioctl_AIOWDM_IRQ_WAIT_CANCLE (struct file *filp, unsigned long arg)
{
  aio_driver_debug_print("Enter");
  aio_driver_dev_print("Stubbed function");
  return -EPERM;
}

int aio_driver_register_action(void *bar_bases[6], struct register_operation op)
{
  int status = 0;
  aio_driver_debug_print("Enter");
  aio_driver_dev_print("Stubbed function");
  switch (op.action)
  {
    case REGISTER_ACTION_NONE:
      break;
    case REGISTER_ACTION_READ_8:
      op.value = ioread8(bar_bases[op.bar] + op.offset);
      break;
    case REGISTER_ACTION_READ_16:
      op.value = ioread16(bar_bases[op.bar] + op.offset);
      break;
    case REGISTER_ACTION_READ_32:
      op.value = ioread32(bar_bases[op.bar] + op.offset);
      break;
    case REGISTER_ACTION_WRITE_8:
      iowrite8(op.value, bar_bases[op.bar] + op.offset);
      break;
    case REGISTER_ACTION_WRITE_16:
      iowrite16(op.value, bar_bases[op.bar] + op.offset);
      break;
    case REGISTER_ACTION_WRITE_32:
      iowrite32(op.value, bar_bases[op.bar] + op.offset);
      break;
    case REGISTER_ACTION_WRITE_48:
      aio_driver_err_print("unimplemented register action hit");
      status = -EPERM;
      break;
    case REGISTER_ACTION_TESTBITS_8:
      op.value = !((ioread8(bar_bases[op.bar] + op.offset) ^ op.value) & op.mask);
      break;
    case REGISTER_ACTION_TESTBITS_16:
      op.value = !((ioread16(bar_bases[op.bar] + op.offset) ^ op.value) & op.mask);
      break;
    case REGISTER_ACTION_TESTBITS_32:
      op.value = !((ioread32(bar_bases[op.bar] + op.offset) ^ op.value) & op.mask);
      break;
    case REGISTER_ACTION_WRITEBITS_8:
      iowrite8((ioread8(bar_bases[op.bar] + op.offset) & op.mask) | op.value, bar_bases[op.bar]); 
      break;
    case REGISTER_ACTION_WRITEBITS_16:
      iowrite16((ioread16(bar_bases[op.bar] + op.offset) & op.mask) | op.value, bar_bases[op.bar]); 
      break;
    case REGISTER_ACTION_WRITEBITS_32:
      iowrite32((ioread32(bar_bases[op.bar] + op.offset) & op.mask) | op.value, bar_bases[op.bar]); 
      break;
    case REGISTER_ACTION_TOGGLEBITS_8:
      iowrite8(ioread8(bar_bases[op.bar] + op.offset) ^ op.mask, bar_bases[op.bar]);
      break;
    case REGISTER_ACTION_TOGGLEBITS_16:
      iowrite16(ioread16(bar_bases[op.bar] + op.offset) ^ op.mask, bar_bases[op.bar]);
      break;
    case REGISTER_ACTION_TOGGLEBITS_32:
      iowrite16(ioread16(bar_bases[op.bar] + op.offset) ^ op.mask, bar_bases[op.bar]);
      break;
  };
  return status;
}
