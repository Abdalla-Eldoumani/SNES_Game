// The functions in this file implement a basic communications system which
// allows communication between a host and the Raspberry Pi using a UART serial
// connection. Once uart_init() has been called, the Pi can transmit and receive
// characters over the UART connection using the functions uart_putc(),
// uart_puts(), uart_getc(), uart_puthex().

// This file is included since it defines the memory mapped I/O base address
#include "gpio.h"

// The addresses of the Auxilary Mini UART registers:
//
// These are defined on pages 12 - 13 of the Broadcom BCM2711 ARM Peripherals
// manual. Note that we specify the ARM physical addresses of the peripherals,
// which have the address range 0xFE000000 to 0xFEFFFFFF. These addresses are
// mapped by the VideoCore Memory Management Unit (MMU) onto the bus addresses
// in the range 0x7E000000 to 0x7EFFFFFF.
#define AUX_IRQ         ((volatile unsigned int *)(MMIO_BASE + 0x00215000))
#define AUX_ENABLE      ((volatile unsigned int *)(MMIO_BASE + 0x00215004))
#define AUX_MU_IO       ((volatile unsigned int *)(MMIO_BASE + 0x00215040))
#define AUX_MU_IER      ((volatile unsigned int *)(MMIO_BASE + 0x00215044))
#define AUX_MU_IIR      ((volatile unsigned int *)(MMIO_BASE + 0x00215048))
#define AUX_MU_LCR      ((volatile unsigned int *)(MMIO_BASE + 0x0021504C))
#define AUX_MU_MCR      ((volatile unsigned int *)(MMIO_BASE + 0x00215050))
#define AUX_MU_LSR      ((volatile unsigned int *)(MMIO_BASE + 0x00215054))
#define AUX_MU_MSR      ((volatile unsigned int *)(MMIO_BASE + 0x00215058))
#define AUX_MU_SCRATCH  ((volatile unsigned int *)(MMIO_BASE + 0x0021505C))
#define AUX_MU_CNTL     ((volatile unsigned int *)(MMIO_BASE + 0x00215060))
#define AUX_MU_STAT     ((volatile unsigned int *)(MMIO_BASE + 0x00215064))
#define AUX_MU_BAUD     ((volatile unsigned int *)(MMIO_BASE + 0x00215068))

void uart_init()
{
    register unsigned int r;
    

    // Map the Mini UART (UART1) to GPIO pins 14 and 15. The GPIO pins must be
    // set up before initializing the Mini UART.

    // Get the current contents of the GPIO Function Select Register 1
    r = *GPFSEL1;

    // Clear bits 12-14 and 15-17. These are the fields FSEL14 and FSEL15, which
    // map to GPIO pins 14 and 15. We clear the bits by ANDing with a 000 bit
    // pattern in the two fields.
    r &= ~( (0x7 << 12) | (0x7 << 15) );

    // Set the fields FSEL14 and FSEL15 to alternate function 5, which maps the
    // Mini UART peripheral to GPIO pins 14 and 15. We do so by ORing the bit
    // pattern 010 into the fields. This function treats pin 14 as a UART TXD
    // pin, and pin 15 as a UART RXD pin.
    r |= (0x2 << 12) | (0x2 << 15);

    // Write the modified bit pattern back to the GPIO Function Select
    // Register 1
    *GPFSEL1 = r;



    // Disable the pull-up/pull-down control lines for GPIO pins 14 and 15

    // Get the current bit pattern of the GPPUPPDN0 register
    r = *GPPUPPDN0;

    // Zero out bits 28-29 and 30-31 in this bit pattern, since these map to
    // GPIO pins 14 and 15. The bit pattern 00 disables pullups/pulldowns.
    r &= ~( (0x3 << 28) | (0x3 << 30) );

    // Write the modified bit pattern back to the GPPUPPDN0 register
    *GPPUPPDN0 = r;

    
    
    // Initialize the Mini UART peripheral
    
    // Enable the Mini UART by setting bit 0 in the Auxiliary Enable register
    // to a 1 value
    *AUX_ENABLE |= 0x1;
    
    // Disable all Mini UART interrupts by setting all fields in the Mini UART
    // Interrupt Enable Register to zero
    *AUX_MU_IER = 0;
    
    // Turn off flow control features by setting all fields in the Mini UART
    // Control Register to zero
    *AUX_MU_CNTL = 0;
    
    // Set the UART to work in 8-bit mode by setting bits 1:0 in the Mini UART
    // Line Control Register to 11
    *AUX_MU_LCR = 0x3;
    
    // Set the RTS line to high by setting bit 1 (and all other fields) in the
    // Mini UART Modem Control Register to zero
    *AUX_MU_MCR = 0;
    
    // Enable both the receive and transmit FIFO buffers and clear their
    // contents by setting bits 7:6 and 2:1 in the Mini UART Interrupt Status
    // Register to 1 values (the bit mask is:  1100 0110)
    *AUX_MU_IIR = 0xc6;
    
    // Set the Baud rate to 115200. We do this by putting the value 270 into
    // bits 15:0 of the Mini UART Baud Register. This value is calculated with
    // the formula:  rint((systemClockRate / (8 * 115200)) - 1)
    // where the systemClockRate is 250 MHz.
    *AUX_MU_BAUD = 270;

    // Enable the Mini UART's transmitter and receiver by setting bits 1:0 in
    // the Mini UART Control Register to the bit pattern 11
    *AUX_MU_CNTL = 0x3;
}


void uart_putc(unsigned int c)
{
    // Loop until the transmit FIFO buffer is able to accept a character for
    // transmission. This will be true when the Transmitter Empty bit (bit 5)
    // in the Mini UART Line Status Register is a 1 value.
    do {
    	// Use the NOP assembly language instruction in the loop body
      	asm volatile("nop");
    } while ( !(*AUX_MU_LSR & 0x20) );
    
    // Write the character to the mini UART I/O register
    *AUX_MU_IO = c;
}


char uart_getc()
{
    char r;
    
    // Loop until an input character is available in the receive FIFO buffer.
    // At least one character is available when the Data Ready bit (bit 0) in
    // the Mini UART Line Status Register is a 1 value.
    do {
    	// Use the NOP assembly language instruction in the loop body
        asm volatile("nop");
    } while ( !(*AUX_MU_LSR & 0x1) );

    // Read the character from the Mini UART I/O register
    r = (char)(*AUX_MU_IO);
    
    // Convert the carrige return character to a newline character, otherwise
    // return the character unchanged
    return r == '\r' ? '\n' : r;
}

void uart_puts(char *s)
{
    // Keep processing characters in the string until we reach a null
    // terminating character
    while (*s) {
        // If we encounter a newline character in the string then also send a
        // carriage return just before the newline
        if (*s == '\n')
            uart_putc('\r');

	    // Send the current character, and increment the pointer
        uart_putc(*s++);
    }
}

void uart_puthex(unsigned int value) 
{
    register unsigned int digit;
    register int i;

    // Loop 8 times, isolating each 4-bit unit in turn, starting with the
    // leftmost unit
    for (i = 28 ; i >= 0; i -= 4) {
        // Shift and mask the 4-bit unit so that it lays in the right most
        // part of the register
        digit = (value >> i) & 0xF;

        // Convert the integer value into corresponding hexadecimal digit
        if (digit > 9) {
            // Convert the value into the digits A - F
            digit += 0x37;
        } else {
            // Convert the value into the digits 0 - 9
            digit += 0x30;
        }

        // Write the digit to the console terminal
        uart_putc(digit);
    }
}
