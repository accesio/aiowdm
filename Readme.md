# Design
* devices header file will be two tables. One that is the deviceID and a struct describing the device. The other is the static device table required for Linux to load our device driver when it's a loadable module.
  * The integrity of the two tables above will be verified on module load and a mismatch is to be treated as a fatal error.
  * The primary goal for this portion of the design is that it be less error prone when adding new devices that are software compatible with existing devices.
* WaitForIRQ will return a mask that varies by card (stay compatible with non-zero on success)
* mmap BARs? Need POC. https://unix.stackexchange.com/questions/643077/how-can-my-pci-device-driver-remap-pci-memory-to-userspace
  * Not possible with port I/O
  * Possibly implement port I/O fread() and fwrite() handlers.
    * Would need way to set BAR (ioctl probably)
* https://sysprog21.github.io/lkmpg/

# From pci.rst in Linux kernel Docs

```
Once the driver knows about a PCI device and takes ownership, the
driver generally needs to perform the following initialization:

  - Enable the device
  - Request MMIO/IOP resources
  - Set the DMA mask size (for both coherent and streaming DMA)
  - Allocate and initialize shared control data (pci_allocate_coherent())
  - Access device configuration space (if needed)
  - Register IRQ handler (request_irq())
  - Initialize non-PCI (i.e. LAN/SCSI/etc parts of the chip)
  - Enable DMA/processing engines

When done using the device, and perhaps the module needs to be unloaded,
the driver needs to take the follow steps:

  - Disable the device from generating IRQs
  - Release the IRQ (free_irq())
  - Stop all DMA activity
  - Release DMA buffers (both streaming and coherent)
  - Unregister from other subsystems (e.g. scsi or netdev)
  - Release MMIO/IOP resources
  - Disable the device
```

# mmap helper functions
```c
/* These two functions provide almost identical functionality. Depending
 * on the architecture, one will be implemented as a wrapper around the
 * other (in drivers/pci/mmap.c).
 *
 * pci_mmap_resource_range() maps a specific BAR, and vm->vm_pgoff
 * is expected to be an offset within that region.
 *
 * pci_mmap_page_range() is the legacy architecture-specific interface,
 * which accepts a "user visible" resource address converted by
 * pci_resource_to_user(), as used in the legacy mmap() interface in
 * /proc/bus/pci/.
 */
int pci_mmap_resource_range(struct pci_dev *dev, int bar,
			    struct vm_area_struct *vma,
			    enum pci_mmap_state mmap_state, int write_combine);
int pci_mmap_page_range(struct pci_dev *pdev, int bar,
			struct vm_area_struct *vma,
			enum pci_mmap_state mmap_state, int write_combine);
```

#cmake3 on RHEL/CentOS
Enable epel:
https://docs.fedoraproject.org/en-US/epel/
Install cmake3:
sudo yum -y install cmake3



https://itecnote.com/tecnote/python-cmake-on-linux-centos-7-how-to-force-the-system-to-use-cmake3/
