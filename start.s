// This routine is used to establish an environment in which a C program can
// run. We create this environment only on CPU Core 0. The other cores simply
// run an infinite loop.
//
// The stack pointer register is initialized to point just below the text
// section of the program. It grows backwards (toward 0), so it uses memory
// addresses below that of the _start routine.
//
// We also zero out all bytes in the .bss section, and then branch to the main()
// routine. The main() routine should never return to this code (it should be in
// an infinite loop), but if it does, we then put the CPU Core 0 into an
// infinite loop.


        // Put the machine code for this routine into the .text.boot section
        .section ".text.boot"

        // The _start symbol needs to be visible to the linker since this is
        // where execution starts for bare metal code
        .global _start
_start:
        // Copy the contents of the multiprocessor affinity register into the x1
        // register. The rightmost 2 bits gives us the CPU Core number that this
        // code is running on. We will only continue running the rest of the
        // program if we are on CPU Core 0. We will put all other cores into an
        // infinite loop.
        mrs     x1, mpidr_el1   // Read the MP affinity system register
        tst     x1, 0x3	        // Bitwise AND the rightmost 2 bits
        b.eq    core_zero       // Skip forward if both bits are 0

        //  If here, the CPU Core number is not 0, so loop forever
loop:  	wfe                     // Wait for event puts the core to sleep
        b       loop            // Infinite loop

        // If here, the CPU Core is 0, and we run the rest of the program
core_zero:

        // Set the stack pointer to point to where the _start routine begins.
        // The stack grows backwards (towards 0), so it uses memory that has
        // lower addresses than the _start routine. We need to set this properly
        // so that C functions and assembly routines can allocate stack frames.
        adrp    x1, _start              // Put the _start address into x1
        add     x1, x1, :lo12:_start
        mov     sp, x1                  // Copy the address into the sp register

        // Clear the .bss section using a loop. The __bss_start symbol indicates
        // where in RAM the .bss section starts. The __bss_size symbol is also
        // provided by the linker, and gives the size (in doublewords) of the
        // .bss section.
        adrp	x1, __bss_start         // Put address of .bss into x1
        add     x1, x1, :lo12:__bss_start
        ldr     w2, =__bss_size         // Put the size of the .bss section
                                        // into w2, using a literal pool.
                                        // w2 is our counter.


top:    cbz     w2, endloop             // Exit loop if counter == 0
        str     xzr, [x1], 8            // Write zeroes to RAM, x1 += 8
        sub     w2, w2, 1               // Decrement counter (w2)
        cbnz    w2, top                 // Keep looping while counter != 0
endloop:

        // Branch to the main() routine, which should never return
        bl      main

        // We should never arrive here, but if we do we branch to the infinite
        // loop above
        b       loop
