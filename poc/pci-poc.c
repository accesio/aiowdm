#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/cdev.h>




static const struct pci_device_id poc_pci_tbl[] = {
	{ PCI_DEVICE(0x494f, 0x0f08) },
	{ 0 }
};

static int pci_poc_probe (struct pci_dev *dev, const struct pci_device_id *id);
static void pci_poc_remove (struct pci_dev *dev);

static struct pci_driver poc_driver = {
	.name = "acces_pci_poc",
	.probe = pci_poc_probe,
	.remove = pci_poc_remove,
	.id_table = poc_pci_tbl,
};

struct poc_pci_dev_data
{
  struct pci_dev *pci_dev;
};



static int poc_driver_open (struct inode *inode, struct file *filp);
static int poc_driver_release (struct inode *inode, struct file *filp);
static int poc_driver_mmap (struct file *filp, struct vm_area_struct *vma);


static struct file_operations poc_driver_fops =
{
	.owner = THIS_MODULE,
	.open = poc_driver_open,
	.release = poc_driver_release,
	.mmap = poc_driver_mmap,
};

struct poc_device_context
{
	struct poc_pci_dev_data pci_dev_data;

	struct cdev cdev;
	int dev_major;
	struct device *dev;
	dev_t devt;

};

//TODO: There has to be a way not to need a "max cards"
static const unsigned MAX_CARDS = 32;
static const char PCI_POC_CLASS_NAME[] = "poc_class";
static dev_t poc_char_dev;
static int poc_dev_count;
static struct class *poc_class;



//TODO: Figure out why BAR[3] returns -EBUSY. For now use this mask.
#define BAR_MASK 0x7

int pci_poc_probe (struct pci_dev *dev, const struct pci_device_id *id)
{
	int status;
	struct poc_device_context *context = NULL;
	pci_info(dev, "Probing\n");

	status = pci_enable_device(dev);

	if (status)
	{
		pci_err(dev, "pci_enable_device failed with status = %d", status);
	}

	//TODO: Check if card has DMA here and perform whatever setup is needed

	status = pci_request_selected_regions (dev, BAR_MASK, "pci_poc_regions");

	if (status)
	{
		pci_err(dev, "pci_request_regions failed with status = %d\n", status);
		goto err_regions;
	}

	context = kzalloc(sizeof(struct poc_device_context), GFP_KERNEL);

	if (NULL == context)
	{
		pci_err(dev, "Could not allocate context\n");
		goto err_regions;
	}

	context->pci_dev_data.pci_dev = dev;
	pci_set_drvdata(dev, context);

	cdev_init(&context->cdev, &poc_driver_fops);
  status = cdev_add(&context->cdev, poc_char_dev, 1) ;


  if (status)
  {
     pci_err(dev, "Error calling cdev_add(): %d\n", status);
     goto err_cdev;
  }

	pci_info(dev, "poc_char_dev = %x\n", poc_char_dev);

	context -> devt = poc_char_dev + poc_dev_count;
	context->dev = device_create(poc_class,
									&context->pci_dev_data.pci_dev->dev,
									context->devt,
									context,
									"poc_device");

	if (NULL == context->dev)
	{
		pci_err(dev, "Error calling device_create()\n");
		goto err_cdev;
	}


	return 0;

err_cdev:
	cdev_del(&context->cdev);
	kfree(context);
err_regions:
	pci_disable_device(dev);
	return status;
}

void pci_poc_remove (struct pci_dev *dev)
{
	struct poc_device_context *context = NULL;

	pci_info(dev, "Removing");

	context = pci_get_drvdata(dev);

	device_destroy(poc_class, context->devt);
	cdev_del(&context->cdev);
	pci_release_selected_regions (dev, BAR_MASK);
	pci_disable_device(dev);
	kfree(context);
}


static int poc_driver_open (struct inode *inode, struct file *filp)
{
	struct poc_device_context *context = container_of(inode->i_cdev, struct poc_device_context, cdev);
	filp->private_data = context;
	return 0;
}

static int poc_driver_release (struct inode *inode, struct file *filp)
{
	return 0;
}

static int poc_driver_mmap (struct file *filp, struct vm_area_struct *vma)
{
	struct poc_device_context *context = filp->private_data;
	int status;

	//status = pci_mmap_resource_range(context->pci_dev_data.pci_dev, 1, vma, pci_mmap_mem, 0);
	pci_mmap_resource_range
	//status = pci_mmap_page_range (context->pci_dev_data.pci_dev, 2, vma, pci_mmap_mem, 0);
	//status = pci_iobar_pfn(context->pci_dev_data.pci_dev, 2, vma);
	//status = pci_mmap_resource(NULL, NULL, NULL, 0);
		return 0;
}



static int __init poc_init(void)
{
	int status;
	pr_info("Enter\n");


  status = alloc_chrdev_region(&poc_char_dev, 0, MAX_CARDS, "poc_region");

  if (status)
  {
    pr_err("Unable to allocate char dev region\n");
		goto out_driver;
  }
	else
	{
		pr_info("status = %d, poc_char_dev = 0x%x\n", status, poc_char_dev);
	}

	poc_class = class_create(THIS_MODULE, PCI_POC_CLASS_NAME);

	if (IS_ERR(poc_class))
	{
		pr_err("Unable to create class\n");
		goto out_driver;
	}

		status = pci_register_driver(&poc_driver);

	if (status)
  {
    pr_err("Unable to register driver\n");
		goto out_driver;
  }

	return 0;

out_driver:
	return -1;

}

static void __exit poc_exit(void)
{
	pr_info("Exit\n");
	class_destroy(poc_class);
	poc_class = NULL;
	pci_unregister_driver(&poc_driver);
}

module_init(poc_init);
module_exit(poc_exit);

MODULE_LICENSE("GPL");

MODULE_DEVICE_TABLE (pci, poc_pci_tbl);
