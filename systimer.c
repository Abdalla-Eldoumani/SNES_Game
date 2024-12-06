// The addresses of the BCM System Timer registers:
//
// These are defined on page 175 of the Broadcom BCM2711 ARM Peripherals Manual.
// Note that we specify the ARM physical addresses of the peripherals, which
// have the address range 0xFE000000 to 0xFEFFFFFF on the Pi 4.
//
// These addresses are mapped by the VideoCore Memory Management Unit (MMU) onto
// the bus addresses in the range 0x7E000000 to 0x7EFFFFFF.

// This file is included since it defines the memory mapped I/O base address
#include "gpio.h"

#define SYSTEM_TIMER_CS	    ((volatile unsigned int *)(MMIO_BASE + 0x00003000))
#define SYSTEM_TIMER_CLO    ((volatile unsigned int *)(MMIO_BASE + 0x00003004))
#define SYSTEM_TIMER_CHI    ((volatile unsigned int *)(MMIO_BASE + 0x00003008))
#define SYSTEM_TIMER_C0     ((volatile unsigned int *)(MMIO_BASE + 0x0000300C))
#define SYSTEM_TIMER_C1     ((volatile unsigned int *)(MMIO_BASE + 0x00003010))
#define SYSTEM_TIMER_C2     ((volatile unsigned int *)(MMIO_BASE + 0x00003014))
#define SYSTEM_TIMER_C3     ((volatile unsigned int *)(MMIO_BASE + 0x00003018))




////////////////////////////////////////////////////////////////////////////////
//
//  Function:       get_timer_counter
//
//  Arguments:      none
//
//  Returns:        The current value of the BCM system timer counter.
//
//  Description:    This function reads the current value of the BCM system
//                  timer, and returns it as a 64-bit unsigned integer.
//
////////////////////////////////////////////////////////////////////////////////

unsigned long get_timer_counter()
{
    unsigned int high, low;
    
    
    // Read the system timer counter, by reading its higher and lower 32 bits
    high = *SYSTEM_TIMER_CHI;
    low = *SYSTEM_TIMER_CLO;
    
    // We repeat the read if the high 32 bits changed when reading the low
    // 32 bits. This may happen when the low order bits roll over.
    if (high != *SYSTEM_TIMER_CHI) {
        high = *SYSTEM_TIMER_CHI;
        low = *SYSTEM_TIMER_CLO;
    }
    
    // Form the complete 64-bit value, and return it to calling code
    return ( ((unsigned long)high << 32) | low );
}



 
////////////////////////////////////////////////////////////////////////////////
//
//  Function:       microsecond_delay
//
//  Arguments:      interval:     The time to delay in microseconds
//
//  Returns:        void
//
//  Description:    This function uses the BCM System Timer peripheral device
//                  to delay the specified number of microseconds. This timer
//                  is not emulated in Qemu, so this function returns
//                  immediately (without delay) if this code is run under Qemu.
//
////////////////////////////////////////////////////////////////////////////////

void microsecond_delay(unsigned int interval)
{
    unsigned long current_counter, target_counter;
	
	
    // Get the current value of the system timer counter
    current_counter = get_timer_counter();
	
    // Because Qemu does not emulate the system counter, the timer counter will
    // always be 0 and we cannot use it to do timing (it will result in an
    // infinite loop). In this case, we return immediately (without any delay).
    if (current_counter == 0) {
        return;
    }
	
    // Calculate the target value of the system timer counter. This will be the
    // specified number of microseconds into the future.
    target_counter = current_counter + interval;
	    
    // Keep polling the system timer counter until we reach the target value
    while (get_timer_counter() < target_counter)
        ;
    	
    // Once we have reached this point, we have delayed the specified number of
    // microseconds, so return
    return;
}
