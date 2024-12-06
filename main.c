// This program demonstrates how to retrieve button-press information from the
// SNES Controller. It polls the controller 30 times a second, and prints out
// the button presses as a binary-encoded 16-bit integer. 0 means unpressed,
// and 1 means pressed.


// Include files
#include "uart.h"
#include "gpio.h"
#include "systimer.h"

// Function prototypes
unsigned short get_SNES();
void init_GPIO9_to_output();
void set_GPIO9();
void clear_GPIO9();
void init_GPIO11_to_output();
void set_GPIO11();
void clear_GPIO11();
void init_GPIO10_to_input();
unsigned int get_GPIO10();
void int_to_hex_string(unsigned short value, char *buffer);

////////////////////////////////////////////////////////////////////////////////
//
//  Function:       main
//
//  Arguments:      none
//
//  Returns:        void
//
//  Description:    This function first initializes GPIO pins 9 and 11 as LATCH
//                  and CLOCK output lines, and GPIO pin 10 as a DATA input
//                  line. It then polls the SNES controller 30 times a second,
//                  and prints out the button presses as an encoded 16-bit
//                  integer (in hexadecimal) whenever a button changes state.
//
////////////////////////////////////////////////////////////////////////////////

void main()
{
    unsigned short data, currentState = 0xFFFF;

    // Set up the UART serial port
    uart_init();

    // Set up GPIO pins
    init_GPIO9_to_output();
    init_GPIO11_to_output();
    init_GPIO10_to_input();

    // Clear the LATCH line (GPIO 9) to low
    clear_GPIO9();

    // Set CLOCK line (GPIO 11) to high
    set_GPIO11();

    // Print out a message to the console
    uart_puts("SNES Controller Program starting.\n");

    // Loop forever, reading from the SNES controller 30 times per second
    while (1) {
        // Read data from the SNES controller
        data = get_SNES();

        // If the state of the controller has changed
        if (data != currentState) {

            // Send the button state
            uart_puthex(data);

            // Update the current state
            currentState = data;
        }

        // Delay 1/30th of a second
        microsecond_delay(33333);
    }
}



////////////////////////////////////////////////////////////////////////////////
//
//  Function:       get_SNES
//
//  Arguments:      none
//
//  Returns:        A short integer with the button presses encoded with 16
//                  bits. 1 means pressed, and 0 means unpressed. Bit 0 is
//                  button B, Bit 1 is button Y, etc. up to Bit 11, which is
//                  button R. Bits 12-15 are always 0.
//
//  Description:    This function samples the button presses on the SNES
//                  controller, and returns an encoding of these in a 16-bit
//                  integer. We assume that the CLOCK output is already high,
//                  and set the LATCH output to high for 12 microseconds. This
//                  causes the controller to latch the values of the button
//                  presses into its internal register. We then clock this data
//                  to the CPU over the DATA line in a serial fashion, by
//                  pulsing the CLOCK line low 16 times. We read the data on
//                  the falling edge of the clock. The rising edge of the clock
//                  causes the controller to output the next bit of serial data
//                  to be place on the DATA line. The clock cycle is 12
//                  microseconds long, so the clock is low for 6 microseconds,
//                  and then high for 6 microseconds. 
//
////////////////////////////////////////////////////////////////////////////////

unsigned short get_SNES()
{
    int i;
    unsigned short data = 0;
    unsigned int value;
	
	
    // Set LATCH to high for 12 microseconds. This causes the controller to
    // latch the values of button presses into its internal register. The first
    // serial bit also becomes available on the DATA line.
    set_GPIO9();
    microsecond_delay(12);
    clear_GPIO9();
	
    // Output 16 clock pulses, and read 16 bits of serial data
	for (i = 0; i < 16; i++) {
		// Delay 6 microseconds (half a cycle)
		microsecond_delay(6);
		
		// Clear the CLOCK line (creates a falling edge)
		clear_GPIO11();
		
		// Read the value on the input DATA line
		value = get_GPIO10();
		
		// Store the bit read. Note we convert a 0 (which indicates a button
		// press) to a 1 in the returned 16-bit integer. Unpressed buttons will
		// be encoded as a 0.
		if (value == 0) {
	    	data |= (0x1 << i);
		}
		
		// Delay 6 microseconds (half a cycle)
		microsecond_delay(6);
		
		// Set the CLOCK to 1 (creates a rising edge). This causes the
		// controller to output the next bit, which we read half a cycle later.
		set_GPIO11();
    }
	
    // Return the encoded data
    return data;
}



////////////////////////////////////////////////////////////////////////////////
//
//  Function:       init_GPIO9_to_output
//
//  Arguments:      none
//
//  Returns:        void
//
//  Description:    This function sets GPIO pin 9 to an output pin without any
//                  pull-up or pull-down resistors.
//
////////////////////////////////////////////////////////////////////////////////

void init_GPIO9_to_output()
{
    register unsigned int r;
    
    
    // Get the current contents of the GPIO Function Select Register 0
    r = *GPFSEL0;

    // Clear bits 27 - 29. This is the field FSEL9, which maps to GPIO pin 9.
    // We clear the bits by ANDing with a 000 bit pattern in the field.
    r &= ~(0x7 << 27);

    // Set the field FSEL9 to 001, which sets pin 9 to an output pin. We do so
    // by ORing the bit pattern 001 into the field.
    r |= (0x1 << 27);

    // Write the modified bit pattern back to the GPIO Function Select
    // Register 0
    *GPFSEL0 = r;


    // Disable the pull-up/pull-down control line for GPIO pin 9:

    // Get the current bit pattern of the GPPUPPDN0 register
    r = *GPPUPPDN0;

    // Zero out bits 18-19 in this bit pattern, since this maps to GPIO pin 9.
    // The bit pattern 00 disables pullups/pulldowns.
    r &= ~(0x3 << 18);

    // Write the modified bit pattern back to the GPPUPPDN0 register
    *GPPUPPDN0 = r;
}



////////////////////////////////////////////////////////////////////////////////
//
//  Function:       set_GPIO9
//
//  Arguments:      none
//
//  Returns:        void
//
//  Description:    This function sets the GPIO output pin 9 to a 1 (high)
//                  level.
//
////////////////////////////////////////////////////////////////////////////////

void set_GPIO9()
{
    register unsigned int r;
	  
    // Put a 1 into the SET9 field of the GPIO Pin Output Set Register 0
    r = (0x1 << 9);
    *GPSET0 = r;
}



////////////////////////////////////////////////////////////////////////////////
//
//  Function:       clear_GPIO9
//
//  Arguments:      none
//
//  Returns:        void
//
//  Description:    This function clears the GPIO output pin 9 to a 0 (low)
//                  level.
//
////////////////////////////////////////////////////////////////////////////////

void clear_GPIO9()
{
    register unsigned int r;
	  
    // Put a 1 into the CLR9 field of the GPIO Pin Output Clear Register 0
    r = (0x1 << 9);
    *GPCLR0 = r;
}



////////////////////////////////////////////////////////////////////////////////
//
//  Function:       init_GPIO11_to_output
//
//  Arguments:      none
//
//  Returns:        void
//
//  Description:    This function sets GPIO pin 11 to an output pin without
//                  any pull-up or pull-down resistors.
//
////////////////////////////////////////////////////////////////////////////////

void init_GPIO11_to_output()
{
    register unsigned int r;
    
    
    // Get the current contents of the GPIO Function Select Register 1
    r = *GPFSEL1;

    // Clear bits 3 - 5. This is the field FSEL11, which maps to GPIO pin 11.
    // We clear the bits by ANDing with a 000 bit pattern in the field.
    r &= ~(0x7 << 3);

    // Set the field FSEL11 to 001, which sets pin 9 to an output pin. We do so
    // by ORing the bit pattern 001 into the field.
    r |= (0x1 << 3);

    // Write the modified bit pattern back to the GPIO Function Select
    // Register 1
    *GPFSEL1 = r;

    
    // Disable the pull-up/pull-down control line for GPIO pin 11:

    // Get the current bit pattern of the GPPUPPDN0 register
    r = *GPPUPPDN0;

    // Zero out bits 22-23 in this bit pattern, since this maps to GPIO pin 11.
    // The bit pattern 00 disables pullups/pulldowns.
    r &= ~(0x3 << 22);

    // Write the modified bit pattern back to the GPPUPPDN0 register
    *GPPUPPDN0 = r;
}



////////////////////////////////////////////////////////////////////////////////
//
//  Function:       set_GPIO11
//
//  Arguments:      none
//
//  Returns:        void
//
//  Description:    This function sets the GPIO output pin 11 to a 1 (high)
//                  level.
//
////////////////////////////////////////////////////////////////////////////////

void set_GPIO11()
{
    register unsigned int r;
	  
    // Put a 1 into the SET11 field of the GPIO Pin Output Set Register 0
    r = (0x1 << 11);
    *GPSET0 = r;
}



////////////////////////////////////////////////////////////////////////////////
//
//  Function:       clear_GPIO11
//
//  Arguments:      none
//
//  Returns:        void
//
//  Description:    This function clears the GPIO output pin 11 to a 0 (low)
//                  level.
//
////////////////////////////////////////////////////////////////////////////////

void clear_GPIO11()
{
    register unsigned int r;
	  
    // Put a 1 into the CLR11 field of the GPIO Pin Output Clear Register 0
    r = (0x1 << 11);
    *GPCLR0 = r;
}



////////////////////////////////////////////////////////////////////////////////
//
//  Function:       init_GPIO10_to_input
//
//  Arguments:      none
//
//  Returns:        void
//
//  Description:    This function sets GPIO pin 10 to an input pin without any
//                  internal pull-up or pull-down resistors.
//
////////////////////////////////////////////////////////////////////////////////

void init_GPIO10_to_input()
{
    register unsigned int r;
    
    
    // Get the current contents of the GPIO Function Select Register 1
    r = *GPFSEL1;

    // Clear bits 0 - 2. This is the field FSEL10, which maps to GPIO pin 10.
    // We clear the bits by ANDing with a 000 bit pattern in the field. This
    // sets the pin to be an input pin.
    r &= ~(0x7 << 0);

    // Write the modified bit pattern back to the GPIO Function Select
    // Register 1
    *GPFSEL1 = r;


    // Disable the pull-up/pull-down control line for GPIO pin 10:

    // Get the current bit pattern of the GPPUPPDN0 register
    r = *GPPUPPDN0;

    // Zero out bits 20-21 in this bit pattern, since this maps to GPIO pin 10.
    // The bit pattern 00 disables pullups/pulldowns.
    r &= ~(0x3 << 20);

    // Write the modified bit pattern back to the GPPUPPDN0 register
    *GPPUPPDN0 = r;
}



////////////////////////////////////////////////////////////////////////////////
//
//  Function:       get_GPIO10
//
//  Arguments:      none
//
//  Returns:        1 if the pin level is high, and 0 if the pin level is low.
//
//  Description:    This function gets the current value of pin 10.
//
////////////////////////////////////////////////////////////////////////////////

unsigned int get_GPIO10()
{
    register unsigned int r;
	  
	  
    // Get the current contents of the GPIO Pin Level Register 0
    r = *GPLEV0;
	  
    // Isolate pin 10, and return its value (a 0 if low, or a 1 if high)
    return ((r >> 10) & 0x1);
}