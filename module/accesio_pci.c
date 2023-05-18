#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/device.h>

#include "accesio_pci_ids.h"
#include "accesio_pci_ioctl.h"

MODULE_DESCRIPTION("ACCESIO PCI driver for aiowdm-linux");
MODULE_AUTHOR("ACCES");
MODULE_LICENSE("GPL");


#define aio_driver_err_print(fmt, ...) \
				do { printk( "%s:%d:%s(): " fmt "\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); } while (0);

#ifndef AIO_DEBUG
#define AIO_DEBUG 1
#endif

#ifndef AIO_VERBOSE
#define AIO_VERBOSE 1
#endif

//Pretty much anything that we might want customers to generate for debugging issues
#define aio_driver_debug_print(fmt, ...) \
				do { if (AIO_DEBUG) printk ("%s:%d:%s(): " fmt "\n" , __FILE__, \
																__LINE__, __func__, ##__VA_ARGS__); } while (0)

//Idea is this will be used during development or that might incur performance impact when debugging
#define aio_driver_dev_print(fmt, ...) \
				do { if (AIO_VERBOSE) printk ("%s:%d:%s(): " fmt "\n" , __FILE__, \
																__LINE__, __func__, ##__VA_ARGS__); } while (0)


struct accesio_pci_device_context
{
    struct accesio_pci_device_descriptor *descriptor;
    union  accesio_pci_isr_context       *isr_context;
    struct pci_dev                       *pci_dev;
    struct cdev                          cdev;
    dev_t                                dev_major;
    int                                 default_bar; //used when for aiowdm calls that take CardNum parameter
    void                                *bar_bases[6];
    union accesio_pci_isr_context       *accesio_pci_isr_context;
    spinlock_t                           irq_lock;
    struct device                       *char_dev;
};





/***********************init(), clear(), enable(), and handler() functions*****
 * At the time of this writing there are 13 types of PCI and 8 types of PCIe
 * handlers. Since the previous implementation had the init defined alongside
 * the IRQ operations we may end up with less function pointers than what the
 * table implementation had
******************************************************************************/

//Starting with GCC 13 the region pragma will be ignored by GCC
//For now do this to make my life easier
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma region irq_ops


#define accesio_handler_ddata() \
    struct accesio_pci_device_context *ddata = NULL; \
    ddata = (struct accesio_pci_device_context *)context; \

#define accesio_leave_if_not_latched_pci() {\
    __u8 byte; \
    byte = ioread8(ddata->bar_bases[0] + 0x4c); \
    if ((byte & 0x04) == 0) \
    { \
        return IRQ_NONE; \
    } \
}

#define accesio_leave_if_not_latched_pcie() {\
    __u8 byte; \
    byte = ioread8(ddata->bar_bases[1] + 0x69); \
    if ((byte & 0x80) == 0) \
    { \
        return IRQ_NONE; \
    } \
}

void enable_pci_style_1 (void *context)
{
    //REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x13, .value = 0x02
    accesio_handler_ddata();
    iowrite8( 0x02, ddata->bar_bases[2] + 0x13);
}

void disable_pci_style_1 (void *context)
{
    //REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x13, .value = 0x00
    accesio_handler_ddata();
    iowrite8(0x00, ddata->bar_bases[2] + 0x13);
}

irqreturn_t interrupt_pci_style_1 (int irq, void *context)
{
    accesio_handler_ddata();

    //.action=REGISTER_ACTION_TESTBITS_8, 	.bar=1, .offset=0x4C, .value=0xFF, .mask=0x04
    accesio_leave_if_not_latched_pci();
    //REGISTER_ACTION_READ_8, .bar = 2, .offset = 0x13
    //byte = ioread8(ddata->bar_bases[2] + 0x13);
    return IRQ_HANDLED;
}

void enable_pci_style_2 (void*context)
{
    accesio_handler_ddata();
    //REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x1F
    iowrite8(0, ddata->bar_bases[2] + -0x1f);
}

void disable_pci_style_2 (void *context)
{
    accesio_handler_ddata();
    //REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x1E
    iowrite8(0, ddata->bar_bases[2] + 0x1e);
}

irqreturn_t interrupt_pci_style_2 (int irq, void *context)
{
    //No clear defined for this style so disable() then enable()
    accesio_handler_ddata();
    accesio_leave_if_not_latched_pci();
    disable_pci_style_2(context);
    enable_pci_style_2(context);
    return IRQ_HANDLED;
}

void enable_pci_style_3(void * context)
{
    accesio_handler_ddata();
    //REGISTER_ACTION_READ_8, .bar = 2, .offset = 0x01
    ioread8(ddata->bar_bases[2] + 0x01);
};

void disable_pci_style_3 (void *context)
{
    accesio_handler_ddata();
    //REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x02
    iowrite8(0, ddata->bar_bases[2] + 0x02);
}

irqreturn_t interrupt_pci_style_3 (int irq, void *context)
{
    accesio_handler_ddata();
    accesio_leave_if_not_latched_pci();
    //TODO: test_latched for style 3 matches the disable
    // toggle for now. Check one of the manuals later
    //REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x02
    disable_pci_style_3(context);
    enable_pci_style_3(context);
    return IRQ_HANDLED;
}

void enable_pci_style_4 (void * context)
{
    accesio_handler_ddata();
    //REGISTER_ACTION_READ_8, .bar = 2, .offset = 0x03
    ioread8(ddata->bar_bases[2] + 0x03);
}

void disable_pci_style_4 (void *context)
{
    accesio_handler_ddata();
    //REGISTER_ACTION_READ_8, .bar = 2, .offset = 0x04
    ioread8(ddata->bar_bases[2] + 0x04);
}

irqreturn_t interrupt_pci_style_4 (int irq, void *context)
{
    accesio_handler_ddata();
    accesio_leave_if_not_latched_pci();
    //REGISTER_ACTION_READ_8, .bar = 2, .offset = 0x0C
    ioread8(ddata->bar_bases[2] + 0x0c);
    return IRQ_HANDLED;
}

void enable_pci_style_5 (void * context)
{
    accesio_handler_ddata();
    //REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x07, .value = 0x80
    iowrite8(0x80, ddata->bar_bases[2] + 0x07);
}

void disable_pci_style_5 (void *context)
{
    accesio_handler_ddata();
    //REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x07, .value = 0x00
    iowrite8(0x00, ddata->bar_bases[2] + 0x07);
}

irqreturn_t interrupt_pci_style_5 (int irq, void *context)
{
    accesio_handler_ddata();
    accesio_leave_if_not_latched_pci();
    //REGISTER_ACTION_READ_8, .bar = 2, .offset = 0x07
    ioread8(ddata->bar_bases[2] + 0x07);
    return IRQ_HANDLED;
}

void enable_pci_style_6 (void * context)
{
    accesio_handler_ddata();
    //REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x04, .value = 0x81
    iowrite8(0x81, ddata->bar_bases[2] + 0x04);
}

void disable_pci_style_6 (void *context)
{
    accesio_handler_ddata();
    //REGISTER_ACTION_READ_8, .bar = 2, .offset = 0x09
    ioread8(ddata->bar_bases[2] + 0x09);
}

irqreturn_t interrupt_pci_style_6 (int irq, void *context)
{
    accesio_handler_ddata();
    accesio_leave_if_not_latched_pci();
    disable_pci_style_6(context);
    enable_pci_style_6(context);
    return IRQ_HANDLED;
}


void enable_pci_style_7 (void * context)
{
    accesio_handler_ddata();
    //REGISTER_ACTION_READ_8, .bar = 2, .offset = 0x04
    ioread8(ddata->bar_bases[2] + 0x04);
}

//#define disable_pci_style_7 disable_pci_style_6

irqreturn_t interrupt_pci_style_7 (int irq, void *context)
{
    accesio_handler_ddata();
    accesio_leave_if_not_latched_pci();
    disable_pci_style_7(context);
    enable_pci_style_7(context);
    return IRQ_HANDLED;
}

void enable_pci_style_8 (void * context)
{
    accesio_handler_ddata();
    //REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x0B, .value = 0x00
    iowrite8(0x00, ddata->bar_bases[2] + 0x0b);
}

void disable_pci_style_8 (void *context)
{
    accesio_handler_ddata();
}

irqreturn_t interrupt_pci_style_8 (int irq, void *context)
{
    accesio_handler_ddata();
    accesio_leave_if_not_latched_pci();
    //REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x0F
    iowrite8(0x0, ddata->bar_bases[2] + 0x0f);
    return IRQ_HANDLED;
}

void enable_pci_style_9 (void * context)
{
    accesio_handler_ddata()
    //REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x0C, .value = 0x10
    iowrite8(0x10, ddata->bar_bases[2] + 0x0c);
}

void disable_pci_style_9 (void *context)
{
    accesio_handler_ddata()
    //REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x0D
    iowrite8(0x00, ddata->bar_bases[2] + 0x0d);
}

irqreturn_t interrupt_pci_style_9 (int irq, void *context)
{
    accesio_handler_ddata()
    accesio_leave_if_not_latched_pci();
    //REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x0F
    iowrite8(0x00, ddata->bar_bases[2] + 0x0f);
    return IRQ_HANDLED;
}

void enable_pci_style_10 (void * context)
{
    accesio_handler_ddata();
    //REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x0E
    iowrite8(0x00, ddata->bar_bases[2] + 0x0e);
}

void disable_pci_style_10 (void *context)
{
    accesio_handler_ddata();
    //REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x0D
    iowrite8(0x00, ddata->bar_bases[2] + 0x0d);
}

irqreturn_t interrupt_pci_style_10 (int irq, void *context)
{
    accesio_handler_ddata();
    accesio_leave_if_not_latched_pci();
    //REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x0F
    iowrite8(0x00, ddata->bar_bases[2] + 0x0f);
    return IRQ_HANDLED;
}

//PCI_11 style undefined in old header. 
//TODO: Find manuals for PCI style 11
// struct aio_irq_descriptor PCI_11_STYLE = /* S4C&04|E<00|8#6|E<FF	 */


void enable_pci_style_12 (void * context) {}

void disable_pci_style_12 (void *context) {}

irqreturn_t interrupt_pci_style_12 (int irq, void *context)
{
    accesio_handler_ddata();
    accesio_leave_if_not_latched_pci();
    //REGISTER_ACTION_READ_8, .bar = 2, .offset = 0x04
    ioread8(ddata->bar_bases[2] + 0x04);
    return IRQ_HANDLED;
}


void enable_pci_style_13 (void * context) {}

void disable_pci_style_13 (void *context) {}

irqreturn_t interrupt_pci_style_13 (int irq, void *context)
{
    accesio_handler_ddata();
    accesio_leave_if_not_latched_pci();
    //REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x7F, .value = 0x00
    iowrite8(0x00, ddata->bar_bases[2] + 0x7f);
    return IRQ_HANDLED;
}


void enable_pcie_style_1 (void * context)
{
    accesio_handler_ddata();
    //REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x1F
    iowrite8(0x00, ddata->bar_bases[2] + 0x1f);
}

void disable_pcie_style_1 (void *context)
{
    accesio_handler_ddata();
    //REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x1E
    iowrite8(0x00, ddata->bar_bases[2] + 0x1e);
}

irqreturn_t interrupt_pcie_style_1 (int irq, void *context)
{
    accesio_handler_ddata();
    accesio_leave_if_not_latched_pcie();
    disable_pcie_style_1(context);
    enable_pcie_style_1(context);
    return IRQ_HANDLED;
}

void enable_pcie_style_2 (void * context)
{
    accesio_handler_ddata();
    //REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x28, .value = 0xFF
    iowrite8(0xff, ddata->bar_bases[2] + 0x28);
}

void disable_pcie_style_2 (void *context)
{
    accesio_handler_ddata();
    //REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x28, .value = 0x00
    iowrite8(0x00, ddata->bar_bases[2] + 0x28);
}

irqreturn_t interrupt_pcie_style_2 (int irq, void *context)
{
    accesio_handler_ddata();
    accesio_leave_if_not_latched_pcie();
    //REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x29, .value = 0xFF
    iowrite8(0xff, ddata->bar_bases[2] + 0x29);
    return IRQ_HANDLED;
}

void enable_pcie_style_3 (void * context)
{
    accesio_handler_ddata();
    //REGISTER_ACTION_READ_8, .bar = 2, .offset = 0x02
    ioread8(ddata->bar_bases[2] + 0x02);
}

void disable_pcie_style_3 (void *context)
{
    accesio_handler_ddata();
    //REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x02
    iowrite8(0x00, ddata->bar_bases + 0x02);
}

irqreturn_t interrupt_pcie_style_3 (int irq, void *context)
{
    accesio_handler_ddata();
    accesio_leave_if_not_latched_pcie();
    //REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x01
    iowrite8( 0x0, ddata->bar_bases[2] + 0x01);
    return IRQ_HANDLED;
}

void enable_pcie_style_4 (void * context)
{
    accesio_handler_ddata();
    //REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x0B, .value = 0x00
    iowrite8(0x0, ddata->bar_bases[2] + 0x0b);
}

void disable_pcie_style_4 (void *context)
{
    accesio_handler_ddata();
    //REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x0B, .value = 0xFF
    iowrite8(0xff, ddata->bar_bases[2] + 0x0b);
}

irqreturn_t interrupt_pcie_style_4 (int irq, void *context)
{
    accesio_handler_ddata();
    accesio_leave_if_not_latched_pcie();
    //REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x0F
    iowrite8(0x00, ddata->bar_bases[2] + 0x0f);
    return IRQ_HANDLED;
}

//#define enable_pcie_style_5 enable_pcie_style_4

void disable_pcie_style_5 (void *context)
{
    accesio_handler_ddata();
    //REGISTER_ACTION_READ_8, .bar = 2, .offset = 0x0B
    ioread8(ddata->bar_bases[2] + 0x0b);
}

//#define interrupt_pcie_style_5 interrupt_pcie_style_4

void enable_pcie_style_6 (void * context)
{
    accesio_handler_ddata();
    //REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x0E, .value = 0xFF
    iowrite8(0xff, ddata->bar_bases[2] + 0x0e);
}

void disable_pcie_style_6 (void *context)
{
    accesio_handler_ddata();
    //REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x0E, .value = 0x00
    iowrite8(0x00, ddata->bar_bases[2] + 0x0e);
}

irqreturn_t interrupt_pcie_style_6 (int irq, void *context)
{
    accesio_handler_ddata();
    accesio_leave_if_not_latched_pcie();
    //REGISTER_ACTION_WRITE_32, .bar = 2, .offset = 0x08, .value = 0xFFFFFFFF
    iowrite32(0xffffffff, ddata->bar_bases[2] + 0x08);
    return IRQ_HANDLED;
}

void enable_pcie_style_7 (void * context)
{
    accesio_handler_ddata();
    //REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x0E, .value = 0x03
    iowrite8(0x03, ddata->bar_bases[2] + 0x0e);
}

#define disable_pcie_style_7 disable_pcie_style_6

irqreturn_t interrupt_pcie_style_7 (int irq, void *context)
{
    accesio_handler_ddata();
    accesio_leave_if_not_latched_pcie();
    //REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x0F
    iowrite8(0x00, ddata->bar_bases[2] + 0x0f);
    return IRQ_HANDLED;
}

void enable_pcie_style_8 (void * context)
{
    accesio_handler_ddata();
    //REGISTER_ACTION_READ_8, .bar = 2, .offset = 0x03
    ioread8(ddata->bar_bases[2] + 0x03);
}

void disable_pcie_style_8 (void *context)
{
    accesio_handler_ddata();
    //REGISTER_ACTION_READ_8, .bar = 2, .offset = 0x04
    ioread8(ddata->bar_bases[2] + 0x04);
}

irqreturn_t interrupt_pcie_style_8 (int irq, void *context)
{
    accesio_handler_ddata();
    accesio_leave_if_not_latched_pcie();
    //REGISTER_ACTION_READ_8, .bar = 2, .offset = 0x0C
    ioread8(ddata->bar_bases[2] + 0x0c);
    return IRQ_HANDLED;
}

void enable_adio16f_style (void *context)
{
    accesio_handler_ddata();
    aio_driver_err_print("STUB called!");
}

void disable_adio16f_style (void *context)
{
    accesio_handler_ddata();
    aio_driver_err_print("STUB called!");
}

irqreturn_t interrupt_adio16f_style (int irq, void *context)
{
    accesio_handler_ddata();
    aio_driver_err_print("STUB called");
    return IRQ_NONE;
}

#pragma endregion irq_ops

#pragma region char_driver_ops


#define ACCESIO_CHAR_DEVICE_NAME "accesio"

static int accesio_char_major;
static int accesio_char_next_minor;  //used to minor number when calling


long accesio_char_driver_unlocked_ioctl (struct file *, unsigned int, unsigned long); 
int accesio_char_driver_mmap (struct file *, struct vm_area_struct *); 
int accesio_char_driver_open (struct inode *, struct file *);
int accesio_char_driver_release (struct inode *, struct file *);


struct file_operations accesio_char_driver_fops = {
    .unlocked_ioctl = accesio_char_driver_unlocked_ioctl,
    .mmap = accesio_char_driver_mmap,
    .open = accesio_char_driver_open,
    .release = accesio_char_driver_release,
};

long accesio_char_driver_unlocked_ioctl (struct file *, unsigned int, unsigned long)
{
    aio_driver_err_print("Stub called");
    return -1;
}
int accesio_char_driver_mmap (struct file *, struct vm_area_struct *)
{
    aio_driver_err_print("Stub called");
    return -1;
}
int accesio_char_driver_open (struct inode *, struct file *)
{
    aio_driver_err_print("Stub called");
    return -1;
}
int accesio_char_driver_release (struct inode *, struct file *)
{
    aio_driver_err_print("Stub called");
    return -1;
}

#pragma endregion char_driver_ops

#pragma region pci_driver_ops

int accesio_pci_driver_probe (struct pci_dev *dev, const struct pci_device_id *id);
void accesio_pci_driver_remove (struct pci_dev *dev);
int accesio_pci_driver_suspend (struct pci_dev *dev, pm_message_t state);
int accesio_pci_driver_resume (struct pci_dev *dev);
void accesio_pci_driver_shutdown (struct pci_dev *dev);

static struct pci_driver accesio_pci_driver = {
.name = "accesio_pci_driver",
.id_table = acces_pci_id_table,
.probe = accesio_pci_driver_probe,
.remove = accesio_pci_driver_remove,
.suspend = accesio_pci_driver_suspend,
.resume = accesio_pci_driver_resume,
.shutdown = accesio_pci_driver_shutdown,
//TODO: Look into err_handler to see if it applies
//	const struct pci_error_handlers *err_handler;
};



int accesio_pci_driver_probe (struct pci_dev *dev, const struct pci_device_id *id)
{
    int status = 0;
    struct accesio_pci_device_context *context;
    int i = 0;

    aio_driver_debug_print("<<<");

    status = pci_enable_device(dev);

    if (status)
    {
        aio_driver_err_print("pci_enable_device failed: %d", status);
        goto err_enable;
    }
        

    status = pci_request_regions(dev, "accesio_pci");

    if (status)
    {
        aio_driver_err_print("pci_request_region returned %d", status);
        goto err_request_regions;
    }

    context = kmalloc(sizeof(struct accesio_pci_device_context), GFP_KERNEL);

    if (NULL == context)
    {
        status = -ENOMEM;
        aio_driver_err_print("kmalloc failed");
        goto err_request_regions;
    }

    pci_set_drvdata(dev, context);

    context->pci_dev = dev;

    for (i = 0 ; i < NUM_ACCES_PCI_DEVICES ; i ++)
    {
        if (id->device == accesio_pci_device_descriptors[i].pid)
        {
            context->descriptor = &(accesio_pci_device_descriptors[i]);
            break;
        }
    }

    if (NULL == context->descriptor)
    {
        aio_driver_err_print("Device descriptor not found. Contact support");
        status = -EPERM;
        goto descriptor_not_found;
    }

    for (i = 0 ; i < PCI_STD_NUM_BARS ; i++)
    {
        if (pci_resource_flags(context->pci_dev, i) & 
            (IORESOURCE_IO | IORESOURCE_MEM))
        {
            context->bar_bases[i] = pci_iomap(context->pci_dev, i, 0);
            if (NULL == context->bar_bases[i])
            {
                aio_driver_err_print("Unable to map bar %d", i);
            }
        }
        if (pci_resource_flags(context->pci_dev, i) & IORESOURCE_IO)
        {
            //This should match what the Windows driver comes up with for port base
            //in StartDevice()
            context->default_bar = 1;
        }
    }

    spin_lock_init(&context->irq_lock);
    status = request_irq(context->pci_dev->irq, context->descriptor->isr, IRQF_SHARED, "accesio_pci", context);

    if (status)
    {
        aio_driver_err_print("request_irq returned %d", status);
        goto err_request_irq;
    }

    if (context->descriptor->has_dma)
    {
        context->isr_context = kmalloc(sizeof(union accesio_pci_isr_context), GFP_KERNEL);
        spin_lock_init(&(context->isr_context->dma_context.dma_data_lock));
    }

    context->char_dev = device_create(NULL, 
                                     NULL,
                                     MKDEV(accesio_char_major, accesio_char_next_minor),
                                     NULL,
                                     ACCESIO_CHAR_DEVICE_NAME);

    if (IS_ERR(context->char_dev))
    {
        aio_driver_err_print("device_create failed: %ld", PTR_ERR(context->char_dev));
        goto err_request_irq;
    }
    accesio_char_next_minor++;

err_request_irq:
descriptor_not_found:
    //kfree(context);
    //pci_set_drvdata(dev, NULL);
err_request_regions:
err_enable:
    return status;
}
void accesio_pci_driver_remove (struct pci_dev *dev)
{
    struct accesio_pci_device_context *context;
    int i;
    aio_driver_debug_print("<<<. dev = %p", dev);

    if (NULL == dev)
    {
        aio_driver_err_print("dev is NULL");
        return;
    }

    context = pci_get_drvdata(dev);

    if (NULL == context)
    {
        aio_driver_err_print("context is NULL");
        return;
    }

    free_irq(context->pci_dev->irq, context);

    aio_driver_debug_print("past free_irq");

    for (i = 0 ; i < PCI_STD_NUM_BARS ; i++)
    {
        if (NULL != context->bar_bases[i])
        {
            pci_iounmap(dev, context->bar_bases[i]);
        }
    }

    pci_release_regions(dev);

    aio_driver_debug_print("past pci_iounmap");
    //TODO: Figure out why this crashes on multiple unloads. For now know that
    //sizeof(context) is going to get leaked on unload
    kfree(context);
    pci_set_drvdata(dev, NULL);

    aio_driver_debug_print(">>>");
}
int accesio_pci_driver_suspend (struct pci_dev *dev, pm_message_t state)
{
    aio_driver_debug_print("Enter");
    return 0;
}
int accesio_pci_driver_resume (struct pci_dev *dev)
{
    aio_driver_debug_print("Enter");
    return 0;
}
void accesio_pci_driver_shutdown (struct pci_dev *dev)
{
    aio_driver_debug_print("Enter");
}
#pragma endregion pci_driver_ops



#pragma GCC diagnostic pop


static int accesio_pci_init(void)
{
    int status = 0;
    aio_driver_debug_print("Enter");
    for (int i = 0 ; i < NUM_ACCES_PCI_DEVICES ; i++)
    {
        if (accesio_pci_device_descriptors[i].pid != acces_pci_id_table[i].device)
        {
            aio_driver_err_print("descriptor table doesn't match id table. Refusing to load");
            goto err_mismatch;
        }
    }

    if (acces_pci_id_table[NUM_ACCES_PCI_DEVICES].device != 0)
    {
        aio_driver_err_print("descriptor table doesn't match id table. Refusing to load");
        goto err_mismatch;
    }

    status = pci_register_driver(&accesio_pci_driver);
    if(status)
    {
        aio_driver_err_print("registration failure %d", status);
        goto err_register;
    }

    accesio_char_major = register_chrdev(0, ACCESIO_CHAR_DEVICE_NAME, &accesio_char_driver_fops);

    if (accesio_char_major < 0)
    {
        aio_driver_err_print("unable to register_chardev");
        goto err_register;
    }

    return 0;
err_register:
err_mismatch:
    return -1;
}


static void accesio_pci_exit(void)
{
    aio_driver_debug_print("Enter");
    pci_unregister_driver(&accesio_pci_driver);
}


module_init(accesio_pci_init);
module_exit(accesio_pci_exit);
//MODULE_DEVICE_TABLE(pci, acces_pci_id_table);
