
#include <core.h>
#include <core/initfunc.h>
#include <core/list.h>
#include <core/mmio.h>
#include <net/netapi.h>
#include "pci.h"
#include "i915.h"

typedef unsigned int UINT;

static const char driver_name[] = "i915";
static const char driver_longname[] = "Intel GVT driver";

#ifdef VTD_TRANS
#include "passthrough/vtd.h"
int add_remap() ;
u32 vmm_start_inf() ;
u32 vmm_term_inf() ;
#endif // of VTD_TRANS

static void
i915_new (struct pci_device *pci_device)
{
	void *tmp;
	struct pci_bar_info bar_info;

	printf ("i915 found.\n");

#ifdef VTD_TRANS
        if (iommu_detected && 0) {
                add_remap(pci_device->address.bus_no ,pci_device->address.device_no ,pci_device->address.func_no,
                          vmm_start_inf() >> 12, (vmm_term_inf()-vmm_start_inf()) >> 12, PERM_DMA_RW) ;
        }
#endif // of VTD_TRANS

}

static int
i915_config_read (struct pci_device *pci_device, u8 iosize,
		     u16 offset, union mem *data)
{

return CORE_IO_RET_DEFAULT; //pass

	/* provide fake values
	   for reading the PCI configration space. */
	memset (data, 0, iosize);
	return CORE_IO_RET_DONE;
}

static int
i915_config_write (struct pci_device *pci_device, u8 iosize,
		      u16 offset, union mem *data)
{
return CORE_IO_RET_DEFAULT; //pass

	/* do nothing, ignore any writing. */
	return CORE_IO_RET_DONE;

}
static struct pci_driver i915_driver = {
	.name		= driver_name,
	.longname	= driver_longname,
//	.driver_options	= "tty,net,virtio",
	.device		= "class_code=030000,id="
			  "8086:591e",
	.new		= i915_new,
	.config_read	= i915_config_read,
	.config_write	= i915_config_write,
};

static void
i915_init (void)
{
	pci_register_driver (&i915_driver);
	return;
}

PCI_DRIVER_INIT (i915_init);
