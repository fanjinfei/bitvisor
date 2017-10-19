
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
/* BAR0: GTTMMIO, BAR1: GMADR, BAR2: IO */

/* physical GPU split into two virtual GPUs:
Resources:	I/O space -- hook | no split
		Memory    -- hook | split
		Intr      -- Interrupt pass/share

Hooks:	Memory -- shadow
	I/O    -- display switch
	Cmd    -- scheduler
	Guc    -- only VM0 firmware upload
	Compositor - two

Memory:	low memory -- 
	high memory - 

step1: shrink VM0 memory with hooks working @bitvisor
step2: two vms with display switch(F12), vm1 seaBIOS boot without vbios, only os start GPU @tinyvisor branch
step3: vm1 with vga vbios
*/

#ifdef VTD_TRANS
#include "passthrough/vtd.h"
int add_remap() ;
u32 vmm_start_inf() ;
u32 vmm_term_inf() ;
#endif // of VTD_TRANS

enum hooktype {
	HOOK_ANY,
	HOOK_IOSPACE,
	HOOK_MEMSPACE,
};

struct i915_hook {
	int i;
	int e;
	int io;
	int hd;
	void *h;
	void *map;
	uint maplen;
	phys_t mapaddr;
	u32 iobase;
	struct i915_data *gpud;
};

struct i915_command_list {
	LIST2_DEFINE (struct i915_command_list, list);
	u64 start_time;
	int vGpu_id;
};

struct i915_guest_data {
	phys_t *dev_ctx_array;
} __attribute__ ((packed));

struct i915_regs {
	phys_t iobase;

	u8  cap_offset; /* Capability registers offset, always zero) */
	u8  opr_offset;	/* Operational registers offset */
	u32 db_offset;	/* Doorbell registers offset	*/
	u32 rts_offset;	/* Runtime registers offset	*/
	u16 ext_offset;	/* Extended capabilities offset */

	phys_t cap_start;
	phys_t cap_end;

	phys_t opr_start;
	phys_t opr_end;

	phys_t rts_start;
	phys_t rts_end;

	phys_t db_start;
	phys_t db_end;

	u8 *reg_map;
	u32 map_len;

	u8 *cap_reg;
	u8 *opr_reg;
	u8 *rts_reg;
	u8 *db_reg;

};

struct i915_data {
	struct i915_regs *regs;

	spinlock_t locked_lock;
	bool locked;
	int waiting;
	int host_id;
	struct i915_hook i915_io, i915_mem;
	bool enabled;
	struct pci_device *pci;
	unsigned int nvGpu;	/* Number of vGPU, 2 */
	spinlock_t i915_cmd_lock;
	LIST2_DEFINE_HEAD (i915_cmd_list, struct i915_command_list, list);
	bool i915_cmd_thread;
	int guestOS_started;

	/* Guest part */
	struct i915_guest_data vGpu[2];
};

static int
mmhandler (void *data, phys_t gphys, bool wr, void *buf, uint len, u32 flags)
{
	struct i915_hook *d;
	struct i915_data *ad;

	d = data;
	ad = d->gpud;
	//ahci_lock (ad);
	//mmhandler2 (ad, gphys - ad->ahci_mem.mapaddr, wr, buf, len, flags);
	//ahci_unlock (ad);
	return 1;
}

static int
iohandler (core_io_t io, union mem *data, void *arg)
{
	struct i915_hook *d;
	struct i915_data *ad;

	d = arg;
	ad = d->gpud;
	ASSERT (io.size == 1 || io.size == 2 || io.size == 4);
	//idphandler (ad, io.port - d->iobase, io.dir == CORE_IO_DIR_OUT,
	//	    &data->dword, io.size);
	return CORE_IO_RET_DONE;
}

static void
unreghook (struct i915_hook *d)
{
	if (d->e) {
		if (d->io) {
			core_io_unregister_handler (d->hd);
		} else {
			mmio_unregister (d->h);
			unmapmem (d->map, d->maplen);
		}
		d->e = 0;
	}
}

static void
reghook (struct i915_hook *d, int i, int off, struct pci_bar_info *bar,
	 enum hooktype ht)
{
	if (bar->type == PCI_BAR_INFO_TYPE_NONE)
		return;
	unreghook (d);
	d->i = i;
	d->e = 0;
	if (bar->type == PCI_BAR_INFO_TYPE_IO) {
		if (ht == HOOK_MEMSPACE)
			return;
		if (bar->len < off + 8)
			return;
		d->io = 1;
		d->iobase = bar->base + off;
		d->hd = core_io_register_handler (bar->base + off, 8,
						  iohandler, d,
						  CORE_IO_PRIO_EXCLUSIVE,
						  driver_name);
	} else {
		if (ht == HOOK_IOSPACE)
			return;
		if (bar->len < 0x180)
			/* The memory space is too small for AHCI */
			return;
		d->mapaddr = bar->base;
		d->maplen = bar->len;
		d->map = mapmem_gphys (bar->base, bar->len, MAPMEM_WRITE);
		if (!d->map)
			panic ("mapmem failed");
		d->io = 0;
		d->h = mmio_register (bar->base, bar->len, mmhandler, d);
		if (!d->h)
			panic ("mmio_register failed");
	}
	d->e = 1;
}

static void
i915_new (struct pci_device *pci_device)
{
	void *tmp;
	int i;
	struct i915_data *gd;
	struct pci_bar_info bar_info;

	printf ("i915 found.\n");
	gd = alloc(sizeof *gd);
	memset(gd, 0, sizeof *gd);

	for (i=0; i<3; i++) {
		pci_get_bar_info (pci_device, i, &bar_info);
		printf("i915: %d - base 0x%08llX, len 0x%04x\n", i, bar_info.base, bar_info.len);
	}

	pci_device->driver->options.use_base_address_mask_emulation = 1;
	pci_device->host = gd;

	reghook (&gd->i915_mem, 0, 0, &bar_info, HOOK_MEMSPACE);
	reghook (&gd->i915_mem, 0, 0, &bar_info, HOOK_IOSPACE);

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
