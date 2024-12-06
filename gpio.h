// The addresses of GPIO registers on the Raspberry Pi 4 (BCM2711):
//
// These are defined on pages 83 - 84 of the Broadcom BCM2711 ARM Peripherals
// manual. The BCM2711 is the SoC used for the Raspberry Pi 4. The base address
// for memory-mapped registers on the BCM2711 is 0xFE00000. Note that this has
// changed from the 0x3F00000 address used on the Raspberry Pi 3 (BCM2837).
//
// When we write code to access these registers, we specify the ARM physical
// addresses of the peripherals, which have the address range 0xFE000000 to
// 0xFEFFFFFF. These addresses are mapped by the VideoCore Memory Management
// Unit (MMU) onto bus addresses in the range 0x7E000000 to 0x7EFFFFFF.


// BCM2711 (Raspberry Pi 4) Memory Mapped I/O base address
#define MMIO_BASE       0xFE000000

// GPIO register addresses
#define GPFSEL0         ((volatile unsigned int *)(MMIO_BASE + 0x00200000))
#define GPFSEL1         ((volatile unsigned int *)(MMIO_BASE + 0x00200004))
#define GPFSEL2         ((volatile unsigned int *)(MMIO_BASE + 0x00200008))
#define GPFSEL3         ((volatile unsigned int *)(MMIO_BASE + 0x0020000C))
#define GPFSEL4         ((volatile unsigned int *)(MMIO_BASE + 0x00200010))
#define GPFSEL5         ((volatile unsigned int *)(MMIO_BASE + 0x00200014))
#define GPSET0          ((volatile unsigned int *)(MMIO_BASE + 0x0020001C))
#define GPSET1          ((volatile unsigned int *)(MMIO_BASE + 0x00200020))
#define GPCLR0          ((volatile unsigned int *)(MMIO_BASE + 0x00200028))
#define GPCLR1          ((volatile unsigned int *)(MMIO_BASE + 0x0020002C))
#define GPLEV0          ((volatile unsigned int *)(MMIO_BASE + 0x00200034))
#define GPLEV1          ((volatile unsigned int *)(MMIO_BASE + 0x00200038))
#define GPEDS0          ((volatile unsigned int *)(MMIO_BASE + 0x00200040))
#define GPEDS1          ((volatile unsigned int *)(MMIO_BASE + 0x00200044))
#define GPREN0          ((volatile unsigned int *)(MMIO_BASE + 0x0020004C))
#define GPREN1          ((volatile unsigned int *)(MMIO_BASE + 0x00200050))
#define GPFEN0          ((volatile unsigned int *)(MMIO_BASE + 0x00200058))
#define GPFEN1          ((volatile unsigned int *)(MMIO_BASE + 0x0020005C))
#define GPHEN0          ((volatile unsigned int *)(MMIO_BASE + 0x00200064))
#define GPHEN1          ((volatile unsigned int *)(MMIO_BASE + 0x00200068))
#define GPLEN0          ((volatile unsigned int *)(MMIO_BASE + 0x00200070))
#define GPLEN1          ((volatile unsigned int *)(MMIO_BASE + 0x00200074))
#define GPAREN0         ((volatile unsigned int *)(MMIO_BASE + 0x0020007C))
#define GPAREN1         ((volatile unsigned int *)(MMIO_BASE + 0x00200080))
#define GPAFEN0         ((volatile unsigned int *)(MMIO_BASE + 0x00200088))
#define GPAFEN1         ((volatile unsigned int *)(MMIO_BASE + 0x0020008C))
#define GPPUPPDN0       ((volatile unsigned int *)(MMIO_BASE + 0x002000E4))
#define GPPUPPDN1       ((volatile unsigned int *)(MMIO_BASE + 0x002000E8))
#define GPPUPPDN2       ((volatile unsigned int *)(MMIO_BASE + 0x002000EC))
#define GPPUPPDN3       ((volatile unsigned int *)(MMIO_BASE + 0x002000F0))