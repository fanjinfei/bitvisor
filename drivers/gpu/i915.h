
/* port from Linux
HD 615: kaby lake GT2, according to wiki page

static const struct intel_device_info intel_kabylake_info = {
	.is_kabylake = 1,
	.gen = 9,
	.num_pipes = 3,
	.need_gfx_hws = 1, .has_hotplug = 1,
	.ring_mask = RENDER_RING | BSD_RING | BLT_RING | VEBOX_RING,
	.has_llc = 1,
	.has_ddi = 1,
	.has_fpga_dbg = 1,
	.has_fbc = 1,
	GEN_DEFAULT_PIPEOFFSETS,
	IVB_CURSOR_OFFSETS,
};

*/


#define VGT_REG_CFG_VENDOR_ID			0x00
#define VGT_REG_CFG_COMMAND			0x04
#define _REGBIT_CFG_COMMAND_IO			(1 << 0)
#define _REGBIT_CFG_COMMAND_MEMORY		(1 << 1)
#define _REGBIT_CFG_COMMAND_MASTER		(1 << 2)
#define VGT_REG_CFG_CLASS_PROG_IF		0x09
#define VGT_REG_CFG_SUB_CLASS_CODE		0x0A
#define VGT_REG_CFG_CLASS_CODE			0x0B
#define VGT_REG_CFG_SPACE_BAR0			0x10
#define VGT_REG_CFG_SPACE_BAR1			0x18
#define VGT_REG_CFG_SPACE_BAR2			0x20
#define VGT_REG_CFG_SPACE_BAR_ROM		0x30
#define VGT_REG_CFG_SPACE_MSAC			0x62
#define VGT_REG_CFG_SWSCI_TRIGGER		0xE8
#define	_REGBIT_CFG_SWSCI_SCI_SELECT		(1 << 15)
#define	_REGBIT_CFG_SWSCI_SCI_TRIGGER		1
#define VGT_REG_CFG_OPREGION			0xFC

/*
struct opregion_header;
struct opregion_acpi;
struct opregion_swsci;
struct opregion_asle;

struct intel_opregion {
	struct opregion_header *header;
	struct opregion_acpi *acpi;
	struct opregion_swsci *swsci;
	u32 swsci_gbda_sub_functions;
	u32 swsci_sbcb_sub_functions;
	struct opregion_asle *asle;
	void *vbt;
	u32 *lid_state;
	struct work_struct asle_work;
};
#define OPREGION_SIZE            (8*1024)

enum csr_state {
	FW_UNINITIALIZED = 0,
	FW_LOADED,
	FW_FAILED
};

struct intel_csr {
	const char *fw_path;
	uint32_t *dmc_payload;
	uint32_t dmc_fw_size;
	uint32_t mmio_count;
	uint32_t mmioaddr[8];
	uint32_t mmiodata[8];
	enum csr_state state;
};
*/

#if 0
struct intel_device_info {
	u32 display_mmio_offset;
	u16 device_id;
	u8 num_pipes:3;
	u8 num_sprites[I915_MAX_PIPES];
	u8 gen;
	u8 ring_mask; /* Rings supported by the HW */
	DEV_INFO_FOR_EACH_FLAG(DEFINE_FLAG, SEP_SEMICOLON);
	/* Register offsets for the various display pipes and transcoders */
	int pipe_offsets[I915_MAX_TRANSCODERS];
	int trans_offsets[I915_MAX_TRANSCODERS];
	int palette_offsets[I915_MAX_PIPES];
	int cursor_offsets[I915_MAX_PIPES];

	/* Slice/subslice/EU info */
	u8 slice_total;
	u8 subslice_total;
	u8 subslice_per_slice;
	u8 eu_total;
	u8 eu_per_subslice;
	/* For each slice, which subslice(s) has(have) 7 EUs (bitfield)? */
	u8 subslice_7eu[3];
	u8 has_slice_pg:1;
	u8 has_subslice_pg:1;
	u8 has_eu_pg:1;
};


struct ddi_vbt_port_info {
	/*
	 * This is an index in the HDMI/DVI DDI buffer translation table.
	 * The special value HDMI_LEVEL_SHIFT_UNKNOWN means the VBT didn't
	 * populate this field.
	 */
#define HDMI_LEVEL_SHIFT_UNKNOWN	0xff
	uint8_t hdmi_level_shift;

	uint8_t supports_dvi:1;
	uint8_t supports_hdmi:1;
	uint8_t supports_dp:1;

	uint8_t alternate_aux_channel;
	uint8_t alternate_ddc_pin;

	uint8_t dp_boost_level;
	uint8_t hdmi_boost_level;
};
#endif
