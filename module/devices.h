#include "register-actions.h"

#define AIO_PCI_VEN_ID 0x494f

#define isLatched_PCI   {.action=REGISTER_ACTION_TESTBITS_8, 	.bar=1, .offset=0x4C, .value=0xFF, .mask=0x04 }
#define isLatched_PCIe  {.action=REGISTER_ACTION_TESTBITS_8, 	.bar=1, .offset=0x69, .value=0xFF, .mask=0x80 }
#define init_PCIe       {.action=REGISTER_ACTION_WRITE_8,	.bar=1, .offset=0x69, .value=0x09 }

/* IRQ Configuration struct type */
struct aio_irq_descriptor
{
struct register_operation init; /* things like PCI Express cards needing BAR[0]+0x69 << 0x09 */
struct register_operation test_latched; /* does this card have an IRQ pending? */
struct register_operation disable; /* enable all IRQ sources, or "primary" */
struct register_operation Clear;
struct register_operation Enable;
void * Generate; /*  diagnostic: force card to generate an IRQ, [generally: if enabled] */
/*  TODO: add support for PCIe-IDIO-24-style IRQs
*  TODO: add support for mPCIe-DIO-24A-style IRQs
*/
};

/* IRQ Configuration data instances, one per different IRQ handling "style" used by supported devices. */
 struct aio_irq_descriptor PCI_1_STYLE = /* S4C&04|13<00|13>|13<02	 */
{
.init = { 0 },
.test_latched = isLatched_PCI,
.disable =
	{
	.action=REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x13, .value = 0x00
	},
.Clear =
	{
	REGISTER_ACTION_READ_8, .bar = 2, .offset = 0x13
	},
.Enable =
	{
	REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x13, .value = 0x02
	},
.Generate = NULL,
};

struct aio_irq_descriptor PCI_2_STYLE = /* S4C&04|1E<||1F<	 */
{
.init = { 0 }, 
.test_latched = isLatched_PCI,
.disable =
	{
	REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x1E
	}, 
.Clear = { 0 }, 
.Enable =
	{
	REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x1F
	}, .Generate = NULL,
};

 struct aio_irq_descriptor PCI_3_STYLE = /* S4C&04|2<|1<|2>	 */
{
.init = { 0 },
.test_latched = isLatched_PCI,
.disable =
	{
	REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x02
	},
.Clear =
	{
	REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x02
	},
.Enable =
	{
	REGISTER_ACTION_READ_8, .bar = 2, .offset = 0x01
	},
.Generate = NULL,
};

struct aio_irq_descriptor PCI_4_STYLE = /* S4C&04|4>|C>|3>	 */
{
.init = { 0 },
.test_latched = isLatched_PCI,
.disable =
	{
	REGISTER_ACTION_READ_8, .bar = 2, .offset = 0x04
	},
.Clear =
	{
	REGISTER_ACTION_READ_8, .bar = 2, .offset = 0x0C
	},
.Enable =
	{
	REGISTER_ACTION_READ_8, .bar = 2, .offset = 0x03
	},
.Generate = NULL,
};

 struct aio_irq_descriptor PCI_5_STYLE = /* S4C&04|7<00|7>|7<80	 */
{
.init = { 0 },
.test_latched = isLatched_PCI,
.disable =
	{
	REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x07, .value = 0x00
	},
.Clear =
	{
	REGISTER_ACTION_READ_8, .bar = 2, .offset = 0x07
	},
.Enable =
	{
	REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x07, .value = 0x80
	},
.Generate = NULL,
};

struct aio_irq_descriptor PCI_6_STYLE = /* S4C&04|9>||4<81	 */
{
.init = { 0 },
.test_latched = isLatched_PCI,
.disable =
	{
	REGISTER_ACTION_READ_8, .bar = 2, .offset = 0x09
	}, 
.Clear = { 0 }, 
.Enable =
	{
	REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x04, .value = 0x81
	},
.Generate = NULL,
};

struct aio_irq_descriptor PCI_7_STYLE = /* S4C&04|9>||4>	 */
{
.init = { 0 },
.test_latched = isLatched_PCI,
.disable =
{
	REGISTER_ACTION_READ_8, .bar = 2, .offset = 0x09
}, 
.Clear = { 0 },
.Enable =
	{
	REGISTER_ACTION_READ_8, .bar = 2, .offset = 0x04
	},
.Generate = NULL,
};

struct aio_irq_descriptor PCI_8_STYLE = /* S4C&04|B<FF|F<|B<00	 */
{
.init = { 0 }, 
.test_latched = isLatched_PCI,
.disable =
	{
	REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x0B, .value = 0xFF
	},
.Clear =
	{
	REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x0F
	},
.Enable =
	{
	REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x0B, .value = 0x00
	},
.Generate = NULL,
};

struct aio_irq_descriptor PCI_9_STYLE = /* S4C&04|C<00||C<10	 */
{
.init = { 0 },
.test_latched = isLatched_PCI,
.disable =
	{
	REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x0C, .value = 0x00
	},
.Clear = { 0 	},
.Enable =
	{
	REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x0C, .value = 0x10
	},
.Generate = NULL,
};

 struct aio_irq_descriptor PCI_10_STYLE = /* S4C&04|D<|F<|E<	 */
{
.init ={ 0 },
.test_latched = isLatched_PCI,
.disable =
	{
	REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x0D
	},
.Clear =
	{
	REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x0F
	},
.Enable =
	{
	REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x0E
	},
.Generate = NULL,
};

//TODO: find out about NOT_YET_IMPLEMENTED
 struct aio_irq_descriptor PCI_11_STYLE = { 0 };
// struct aio_irq_descriptor PCI_11_STYLE = /* S4C&04|E<00|8#6|E<FF	 */
// {
// .init = { 0 },
// .test_latched = isLatched_PCI,
// .disable =
// 	{
// 	REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x0E, .value = 0x00
// 	},
// .Clear =
// 	{
// 	NOT_YET_IMPLEMENTED
// 	}, .Enable =
// 	{
// 	REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x0E, .value = 0xFF
// 	}, .Generate = NULL,
//     };

struct aio_irq_descriptor PCI_12_STYLE = /* S4C&04||4>|	 	 */
{
.init = { 0 },
.test_latched = isLatched_PCI,
.disable = { 0 },
.Clear =
	{
	REGISTER_ACTION_READ_8, .bar = 2, .offset = 0x04
	},
.Enable = { 0 },
.Generate = NULL,
};

struct aio_irq_descriptor PCI_13_STYLE = /* S4C&04||7F<00|	 */
{
.init = { 0 },
.test_latched = isLatched_PCI,
.disable = { 0 },
.Clear =
	{
	REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x7F, .value = 0x00
	},
.Enable = { 0 },
.Generate = NULL,
};

struct aio_irq_descriptor PCI_NO_IRQ_STYLE = /* S4C&04|||	 	 */
{
.init = { 0 },
.test_latched = isLatched_PCI,
.disable = { 0 },
.Clear = { 0 },
.Enable = { 0 },
.Generate = NULL,
};

 struct aio_irq_descriptor PCIE_1_STYLE = /* S69&80|1E<||1F<	 */
{
.init = init_PCIe,
.test_latched = isLatched_PCIe,
.disable =
	{
	REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x1E
	},
.Clear = { 0 },
.Enable = 
	{
	REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x1F
	},
.Generate = NULL,
};

struct aio_irq_descriptor PCIE_2_STYLE = /* S69&80|28<00|29<FF|28<FF	 */
{
.init = init_PCIe,
.test_latched = isLatched_PCIe,
.disable =
	{
	REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x28, .value = 0x00
	},
.Clear =
	{
	REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x29, .value = 0xFF
	},
.Enable =
	{
	REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x28, .value = 0xFF
	},
.Generate = NULL,
};

 struct aio_irq_descriptor PCIE_3_STYLE = /* S69&80|2<|1<|2>	 */
{
.init = init_PCIe,
.test_latched = isLatched_PCIe,
.disable =
	{
	REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x02
	},
.Clear =
	{
	REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x01
	},
.Enable =
	{
	REGISTER_ACTION_READ_8, .bar = 2, .offset = 0x02
	},
.Generate = NULL,
};

struct aio_irq_descriptor PCIE_4_STYLE = /* S69&80|B<FF|F<|B<00	 */
{
.init = init_PCIe,
.test_latched = isLatched_PCIe,
.disable =
	{
	REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x0B, .value = 0xFF
	},
.Clear =
	{
	REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x0F
	},
.Enable =
	{
	REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x0B, .value = 0x00
	},
.Generate = NULL,
};

struct aio_irq_descriptor PCIE_5_STYLE = /* S69&80|B>|F<|B<00	 */
{
.init = init_PCIe,
.test_latched = isLatched_PCIe,
.disable =
	{
	REGISTER_ACTION_READ_8, .bar = 2, .offset = 0x0B
	},
.Clear =
	{
	REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x0F
	},
.Enable =
	{
	REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x0B, .value = 0x00
	},
.Generate = NULL,
};

struct aio_irq_descriptor PCIE_6_STYLE = /* S69&80|E<00|8#4|E<FF	 */
{
.init = init_PCIe,
.test_latched = isLatched_PCIe,
.disable =
	{
	REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x0E, .value = 0x00
	},
.Clear =
	{
	REGISTER_ACTION_WRITE_32, .bar = 2, .offset = 0x08, .value = 0xFFFFFFFF
	},
.Enable =
	{
	REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x0E, .value = 0xFF
	},
.Generate = NULL,
};

struct aio_irq_descriptor PCIE_7_STYLE = /* S69&80|E<00|F<|E<03	 */
{
.init = init_PCIe,
.test_latched = isLatched_PCIe,
.disable =
	{
	REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x0E, .value = 0x00
	},
.Clear =
	{
	REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x0F
	},
.Enable =
	{
	REGISTER_ACTION_WRITE_8, .bar = 2, .offset = 0x0E, .value = 0x03
	},
.Generate = NULL,
};

struct aio_irq_descriptor PCIE_8_STYLE = /* S4C&04|4>|C>|3>	 */
{
.init = init_PCIe,
.test_latched = isLatched_PCIe,
.disable =
	{
	REGISTER_ACTION_READ_8, .bar = 2, .offset = 0x04
	},
.Clear =
	{
	REGISTER_ACTION_READ_8, .bar = 2, .offset = 0x0C
	},
.Enable =
	{
	REGISTER_ACTION_READ_8, .bar = 2, .offset = 0x03
	},
.Generate = NULL,
};

struct aio_irq_descriptor NYI =
{
.init = { 0 },
.test_latched = { 0 },
.disable = { 0 },
.Clear = { 0 },
.Enable = { 0 },
.Generate = NULL,
};

struct aio_pci_dev_cfg 
{
	uint16_t pciDevId; /* PCI Device ID */
	char * Model; /* Model name */
	struct aio_irq_descriptor * IRQ; /* IRQ control/configuration data, not an IRQ Number */
	uint32_t flags; /* bitmasked flag storage */
};

/* Match Table */
 const struct aio_pci_dev_cfg aio_pci_dev_table[] =
		{
	{
	.pciDevId = 0x0100, .Model = "mPCIe-DIO-24S", .IRQ = &PCIE_2_STYLE,
	},
	{
	.pciDevId = 0x0101, .Model = "mPCIe-IDIO-8", .IRQ = &PCIE_2_STYLE,
	},
	{
	.pciDevId = 0x0102, .Model = "mPCIe-IIRO-8", .IRQ = &PCIE_2_STYLE,
	},
	{
	.pciDevId = 0x0103, .Model = "mPCIe-IDIO-4", .IRQ = &PCIE_2_STYLE,
	},
	{
	.pciDevId = 0x0104, .Model = "mPCIe-IIRO-4", .IRQ = &PCIE_2_STYLE,
	},
	{
	.pciDevId = 0x0105, .Model = "mPCIe-IDO-8", .IRQ = &PCIE_2_STYLE,
	},
	{
	.pciDevId = 0x0106, .Model = "mPCIe-RO-8", .IRQ = &PCIE_2_STYLE,
	},
	{
	.pciDevId = 0x0107, .Model = "mPCIe-II-16", .IRQ = &PCIE_2_STYLE,
	},
	{
	.pciDevId = 0x0108, .Model = "mPCIe-II-8", .IRQ = &PCIE_2_STYLE,
	},
	{
	.pciDevId = 0x0109, .Model = "mPCIe-II-4", .IRQ = &PCIE_2_STYLE,
	},
	{
	.pciDevId = 0x010A, .Model = "mPCIe-QUAD-4", .IRQ = &PCIE_2_STYLE,
	},
	{
	.pciDevId = 0x010B, .Model = "mPCIe-QUAD-8", .IRQ = &PCIE_2_STYLE,
	},
	{
	.pciDevId = 0x010C, .Model = "mod-TBD", .IRQ = &NYI,
	},
	{
	.pciDevId = 0x010D, .Model = "mod-TBD", .IRQ = &NYI,
	},
	{
	.pciDevId = 0x010E, .Model = "mod-TBD", .IRQ = &NYI,
	},
	{
	.pciDevId = 0x010F, .Model = "mod-TBD", .IRQ = &NYI,
	},
	{
	.pciDevId = 0x0508, .Model = "PCI-IDO-16A", .IRQ = &PCI_NO_IRQ_STYLE,
	},
	{
	.pciDevId = 0x0518, .Model = "PCI-IDO-32A", .IRQ = &PCI_NO_IRQ_STYLE,
	},
	{
	.pciDevId = 0x0520, .Model = "PCI-IDO-48", .IRQ = &PCI_NO_IRQ_STYLE,
	},
	{
	.pciDevId = 0x0521, .Model = "PCI-IDO-48A", .IRQ = &PCI_NO_IRQ_STYLE,
	},
	{
	.pciDevId = 0x0703, .Model = "PCIe-RO-4", .IRQ = &PCIE_3_STYLE,
	},
	{
	.pciDevId = 0x07C0, .Model = "PCIe-IDO-12", .IRQ = &NYI,
	},
	{
	.pciDevId = 0x07D0, .Model = "PCIe-IDO-24", .IRQ = &PCIE_6_STYLE,
	},
	{
	.pciDevId = 0x0920, .Model = "PCI-IDI-48", .IRQ = &PCI_5_STYLE,
	},
	{
	.pciDevId = 0x0BC0, .Model = "PCIe-IDI-12A", .IRQ = &NYI,
	},
	{
	.pciDevId = 0x0BC1, .Model = "PCIe-IDI-12B", .IRQ = &NYI,
	},
	{
	.pciDevId = 0x0BD0, .Model = "PCIe-IDI-24", .IRQ = &PCIE_6_STYLE,
	},
	{
	.pciDevId = 0x0BD1, .Model = "PCIe-IDI-24B", .IRQ = &NYI,
	},
	{
	.pciDevId = 0x0C40, .Model = "PCIe-DIO-144", .IRQ = &PCIE_1_STYLE,
	},
	{
	.pciDevId = 0x0C50, .Model = "PCI-DIO-24H", .IRQ = &PCI_10_STYLE,
	},
	{
	.pciDevId = 0x0C51, .Model = "PCI-DIO-24D", .IRQ = &PCI_10_STYLE,
	},
	{
	.pciDevId = 0x0C52, .Model = "PCIe-DIO-24", .IRQ = &PCIE_7_STYLE,
	},
	{
	.pciDevId = 0x0C53, .Model = "PCIe-DIO-24D/H", .IRQ = &PCIE_7_STYLE,
	},
	{
	.pciDevId = 0x0C54, .Model = "PCIe-DIO-24HC", .IRQ = &PCIE_4_STYLE,
	},
	{
	.pciDevId = 0x0C57, .Model = "mPCIe-DIO-24", .IRQ = &PCIE_4_STYLE,
	},
	{
	.pciDevId = 0x0C60, .Model = "PCI-DIO-48(H)", .IRQ = &PCI_8_STYLE,
	},
	{
	.pciDevId = 0x0C61, .Model = "PCIe-DIO-48", .IRQ = &PCIE_4_STYLE,
	},
	{
	.pciDevId = 0x0C62, .Model = "P104-DIO-48", .IRQ = &PCI_NO_IRQ_STYLE,
	},
	{
	.pciDevId = 0x0C68, .Model = "PCI-DIO-72", .IRQ = &PCI_2_STYLE,
	},
	{
	.pciDevId = 0x0C69, .Model = "P104-DIO-96", .IRQ = &PCI_NO_IRQ_STYLE,
	},
	{
	.pciDevId = 0x0C6A, .Model = "PCIe-DIO-72", .IRQ = &PCIE_1_STYLE,
	},
	{
	.pciDevId = 0x0C70, .Model = "PCI-DIO-96", .IRQ = &PCI_2_STYLE,
	},
	{
	.pciDevId = 0x0C71, .Model = "PCIe-DIO-96", .IRQ = &PCIE_1_STYLE,
	},
	{
	.pciDevId = 0x0C78, .Model = "PCI-DIO-120", .IRQ = &PCI_2_STYLE,
	},
	{
	.pciDevId = 0x0C79, .Model = "PCIe-DIO-120", .IRQ = &PCIE_1_STYLE,
	},
	{
	.pciDevId = 0x0DC8, .Model = "PCI-IDIO-16", .IRQ = &PCI_3_STYLE,
	},
	{
	.pciDevId = 0x0E50, .Model = "PCI-DIO-24S", .IRQ = &PCI_8_STYLE,
	},
	{
	.pciDevId = 0x0E51, .Model = "PCI-DIO-24H(C)", .IRQ = &PCI_10_STYLE,
	},
	{
	.pciDevId = 0x0E52, .Model = "PCI-DIO-24D(C)", .IRQ = &PCI_10_STYLE,
	},
	{
	.pciDevId = 0x0E53, .Model = "PCIe-DIO-24S", .IRQ = &PCIE_4_STYLE,
	},
	{
	.pciDevId = 0x0E54, .Model = "PCIe-DIO-24DS/HS", .IRQ = &PCIE_4_STYLE,
	},
	{
	.pciDevId = 0x0E55, .Model = "PCIe-DIO-24DC", .IRQ = &PCIE_7_STYLE,
	},
	{
	.pciDevId = 0x0E56, .Model = "PCIe-DIO-24DCS", .IRQ = &PCIE_4_STYLE,
	},
	{
	.pciDevId = 0x0E57, .Model = "PCI-DIO-48HS", .IRQ = &PCIE_4_STYLE,
	},
	{
	.pciDevId = 0x0E60, .Model = "PCI-DIO-48S", .IRQ = &PCI_8_STYLE,
	},
	{
	.pciDevId = 0x0E61, .Model = "PCIe-DIO-48S", .IRQ = &PCIE_4_STYLE,
	},
	{
	.pciDevId = 0x0E62, .Model = "P104-DIO-48S", .IRQ = &PCI_11_STYLE,
	},
	{
	.pciDevId = 0x0E63, .Model = "PCIe-EZ-BLEND", .IRQ = &NYI,
	},
	{
	.pciDevId = 0x0E6A, .Model = "PCIe-DIO-72S", .IRQ = &PCIE_1_STYLE,
	},
	{
	.pciDevId = 0x0E71, .Model = "PCIe-DIO-96S", .IRQ = &PCIE_1_STYLE,
	},
	{
	.pciDevId = 0x0E79, .Model = "PCIe-DIO-120S", .IRQ = &PCIE_1_STYLE,
	},
	{
	.pciDevId = 0x0F00, .Model = "PCI-IIRO-8", .IRQ = &PCI_3_STYLE,
	},
	{
	.pciDevId = 0x0F01, .Model = "LPCI-IIRO-8", .IRQ = &PCI_3_STYLE,
	},
	{
	.pciDevId = 0x0F02, .Model = "PCIe-IIRO-8", .IRQ = &PCIE_3_STYLE,
	},
	{
	.pciDevId = 0x0F08, .Model = "PCI-IIRO-16", .IRQ = &PCI_3_STYLE,
	},
	{
	.pciDevId = 0x0F09, .Model = "PCIe-IIRO-16", .IRQ = &PCIE_3_STYLE,
	},
	{
	.pciDevId = 0x0FC0, .Model = "PCIe-IDIO-12", .IRQ = &PCIE_6_STYLE,
	},
	{
	.pciDevId = 0x0FC1, .Model = "PCIe-IDI-12", .IRQ = &PCIE_6_STYLE,
	},
	{
	.pciDevId = 0x0FC2, .Model = "PCIe-IDO-12", .IRQ = &PCIE_6_STYLE,
	},
	{
	.pciDevId = 0x0FD0, .Model = "PCIe-IDIO-24", .IRQ = &PCIE_6_STYLE,
	},
	{
	.pciDevId = 0x0FD1, .Model = "PCIe-IDIO-24B", .IRQ = &NYI,
	},
	{
	.pciDevId = 0x2230, .Model = "PCI-QUAD-8", .IRQ = &PCI_13_STYLE,
	},
	{
	.pciDevId = 0x2231, .Model = "PCI-QUAD-4", .IRQ = &PCI_13_STYLE,
	},
	{
	.pciDevId = 0x22C0, .Model = "PCI-WDG-CSM", .IRQ = &PCI_7_STYLE,
	},
	{
	.pciDevId = 0x25C0, .Model = "P104-WDG-E", .IRQ = &PCI_6_STYLE,
	},
	{
	.pciDevId = 0x2C50, .Model = "PCI-DIO-96CT", .IRQ = &PCI_2_STYLE,
	},
	{
	.pciDevId = 0x2C58, .Model = "PCI-DIO-96C3", .IRQ = &PCI_2_STYLE,
	},
	{
	.pciDevId = 0x2C59, .Model = "PCIe-DIO-96C3", .IRQ = &PCIE_1_STYLE,
	},
	{
	.pciDevId = 0x2C70, .Model = "PCI-DIO-96CT", .IRQ = &NYI,
	},
	{
	.pciDevId = 0x2E50, .Model = "mPCIe-DIO-24A", .IRQ = &PCIE_4_STYLE,
	},
	{
	.pciDevId = 0x2EE0, .Model = "PCIe-DIO24S-CTR12", .IRQ = &PCIE_5_STYLE,
	},
	{
	.pciDevId = 0x2FC0, .Model = "P104-WDG-CSM", .IRQ = &PCI_6_STYLE,
	},
	{
	.pciDevId = 0x2FC1, .Model = "P104-WDG-CSMA", .IRQ = &PCI_6_STYLE,
	},
	{
	.pciDevId = 0x4890, .Model = "PCIe-DA12-2", .IRQ = &PCIE_8_STYLE,
	},
	{
	.pciDevId = 0x4898, .Model = "PCIe-DA12-4", .IRQ = &PCIE_8_STYLE,
	},
	{
	.pciDevId = 0x48a0, .Model = "PCIe-DA12-6", .IRQ = &PCIE_8_STYLE,
	},
	{
	.pciDevId = 0x48a8, .Model = "PCIe-DA12-8", .IRQ = &PCIE_8_STYLE,
	},
	{
	.pciDevId = 0x48b0, .Model = "PCIe-DA12-16", .IRQ = &PCIE_8_STYLE,
	},
	{
	.pciDevId = 0x48d0, .Model = "PCIe-DA16-2", .IRQ = &PCIE_8_STYLE,
	},
	{
	.pciDevId = 0x48d8, .Model = "PCIe-DA16-4", .IRQ = &PCIE_8_STYLE,
	},
	{
	.pciDevId = 0x48e0, .Model = "PCIe-DA16-6", .IRQ = &PCIE_8_STYLE,
	},
	{
	.pciDevId = 0x48e8, .Model = "PCIe-DA16-8", .IRQ = &PCIE_8_STYLE,
	},
	{
	.pciDevId = 0x48f0, .Model = "PCIe-DA16-16", .IRQ = &PCIE_8_STYLE,
	},
	{
	.pciDevId = 0x5ED0, .Model = "PCI-DAC", .IRQ = &NYI,
	},
	{
	.pciDevId = 0x6C90, .Model = "PCI-DA12-2", .IRQ = &PCI_4_STYLE,
	},
	{
	.pciDevId = 0x6C98, .Model = "PCI-DA12-4", .IRQ = &PCI_4_STYLE,
	},
	{
	.pciDevId = 0x6CA0, .Model = "PCI-DA12-6", .IRQ = &PCI_4_STYLE,
	},
	{
	.pciDevId = 0x6CA8, .Model = "PCI-DA12-8", .IRQ = &PCI_4_STYLE,
	},
	{
	.pciDevId = 0x6CA9, .Model = "PCI-DA12-8V", .IRQ = &PCI_4_STYLE,
	},
	{
	.pciDevId = 0x6CB0, .Model = "PCI-DA12-16", .IRQ = &PCI_4_STYLE,
	},
	{
	.pciDevId = 0x6CB1, .Model = "PCI-DA12-16V", .IRQ = &PCI_4_STYLE,
	},
	{
	.pciDevId = 0xACA8, .Model = "PCI-AI12-16", .IRQ = &PCI_12_STYLE,
	},
	{
	.pciDevId = 0xACA9, .Model = "PCI-AI12-16A", .IRQ = &PCI_12_STYLE,
	},
	{
	.pciDevId = 0xECA8, .Model = "PCI-AIO12-16", .IRQ = &PCI_12_STYLE,
	},
	{
	.pciDevId = 0xECA9, .Model = "PCI-A12-16", .IRQ = &PCI_12_STYLE,
	},
	{
	.pciDevId = 0xECAA, .Model = "PCI-A12-16A", .IRQ = &PCI_12_STYLE,
	},
	{
	.pciDevId = 0xECE8, .Model = "LPCI-A16-16A", .IRQ = &PCI_9_STYLE,
	},
	{
	.pciDevId = 0xECE9, .Model = "LPCI-AIO16A", .IRQ = &PCI_1_STYLE,
	},
};

 #define NUM_ACCES_PCI_DEVICES sizeof(aio_pci_dev_table)/sizeof(aio_pci_dev_table[0])