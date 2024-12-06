#  This is a basic Makefile for cross-compiling C language and A64 (aarch64)
#  assembly code into a kernel8.img, which can be run on the Raspberry Pi 4b or
#  on the host machine using the Qemu emulator. The Makefile also works for the
#  Compute Module 4 and its accompanying I/O board.
#
#  To compile your project, type 'make' or 'make all' at the command line. This
#  will create the kernel8.img file, plus a kernel8.dump file. The dump file is
#  a text file which shows the structure and contents of the executable file
#  (kernel8.elf), and may be useful when debugging.
#
#  To remove all the intermediate files from your directory, type 'make clean'
#  at the command line.
#
#  To remove all non-source files from your directory, type 'make deepclean'
#  at the command line. Like 'make clean', this deletes all the intermediate
#  files in the directory, plus any files ending in .img (normally the
#  kernel8.img file).
#
#  Typing 'make run' will execute your program (contained in the kernel8.img
#  file) on the host computer using the Qemu emulator. Qemu is started using
#  flags that set it to emulate a Raspberry Pi 4b device.
#
#  Typing 'make sdcard' will delete the old kernel8.img file on the SD card (if
#  it exists), copy the newly-created kernel8.img file to the SD card, and then
#  "eject" (unmount) the SD card (it will still need to be removed manually
#  from the SD card reader). Note that the SD card should already be inserted
#  into the card reader before typing 'make sdcard'.
#
#  Typing 'make flash' will boot the Compute Module from USB so that its flash
#  memory device appears as removable disk volume, then deletes the existing
#  kernel8.img file, replacing it with the newly-created kernel8.img file. After
#  listing the files on flash memory, the volume is "ejected" (unmounted).
#
#  Typing 'make gdb' will start a debugging session using the GDB debugger.
#  GDB opens a connection to the Raspberry Pi by starting an instance of an
#  OpenOCD GDB server. The server communicates with the Pi using the JTAG
#  transport via a USB-to-JTAG interface device (usually the custom Pi4 Hat V3
#  that is used CPSC 359, but one can specify other interfaces, such as the
#  Segger J-Link EDU or Olimex ARM-USB-TINY-H, in the openocd.cfg file in your
#  directory). The openocd.cfg and gdb_jtag.init files must be in the current
#  working directory in order for the debugging session to work.
#
#  Typing 'make info' will display to the screen the Makefile version number,
#  the installation type, the name of the linker script, the SD card volume
#  name, and the version numbers of the assembler, compiler, Qemu emulator, gdb,
#  and OpenOCD.
#
#  Also note that this Makefile relies on linker script file normally named
#  'link.ld'. The rules in this file tell the ld linker how to create and
#  structure the executable file (kernel8.elf). Read the comments in link.ld
#  for more information.


#  The following is the Makefile version number. It is a manually maintained
#  number that should be incremented whenever this file is modified. Its value
#  is printed out, along with other information, when 'make info' is typed at
#  the command line.
MAKEFILE_VERSION = 0.9.6



#  Determine the Operating system on which the Makefile is running by executing
#  the "uname" shell command. On a Mac OS X machine, the name will be "Darwin",
#  and on a Linux machine it will be "Linux". We use this automatically-detected
#  OS name to set the installation type to either Mac OS X or LINUX (or UNKNOWN
#  if running on another OS). This Makefile can work on either installation
#  type.
ifeq ($(shell uname -s), Darwin)
    INSTALLATION_TYPE = MAC_OS_X
else ifeq ($(shell uname -s), Linux)
    INSTALLATION_TYPE = LINUX
else
    INSTALLATION_TYPE = UNKNOWN
endif



#  Determine the machine type on which the Makefile is running by executing the
#  "uname -p" shell command. On a Mac OS Intel machine, the name will be "i386",
#  and on a Mac OS Arm machine it will be "arm". We use this automatically-
#  detected architecture to set the machine type to either ARM or INTEL (or
#  UNKNOWN on other architectures).
ifeq ($(shell uname -p), arm)
   	MACHINE_TYPE = ARM
else ifeq ($(shell uname -p), i386)
	MACHINE_TYPE = INTEL
else
	MACHINE_TYPE = UNKNOWN
endif



#  The following determines where the gcc toolchain has been installed in the
#  file hierarchy on the host machine, and what prefix is used when naming tools
#  such as 'gcc' and 'as'.
#
#  The Linux computers in the lab use the gnu.org distribution and the
#  "aarch64-elf-" prefix. The installation directory is /usr/bin.
#
#  A Mac OS X computer uses the pre-compiled toolchain downloaded using
#  Homebrew. It is also based on the gnu.org distribution, and uses the 
#  "aarch64-elf-" prefix. The installation directory on an Intel machine is
#  /usr/local/bin, while on an Arm (Apple Silicon) machine it is
#  /opt/homebrew/bin. Note that these directories actually contain symbolic
#  links to the "actual" installation directories in Homebrew's "Cellar".
#
#  If the toolchain is installed at some other location in the file hierarchy,
#  or if it uses a different prefix, then these lines will have to be changed
#  to reflect your custom installation.
ifeq ($(INSTALLATION_TYPE), LINUX)
    INSTALL_DIRECTORY = /usr/bin/
    PREFIX = aarch64-elf-
else ifeq ($(INSTALLATION_TYPE), MAC_OS_X)
	ifeq ($(MACHINE_TYPE), INTEL)
    	INSTALL_DIRECTORY = /usr/local/bin/
    else ifeq ($(MACHINE_TYPE), ARM)
    	INSTALL_DIRECTORY = /opt/homebrew/bin/
    endif
    PREFIX = aarch64-elf-
endif



#  The following is the volume name of your SD card. The name is usually
#  assigned when the SD card is initialized, but it can be changed later if
#  desired. Make sure the name of the SD card exactly matches the definition
#  below, or the 'make sdcard' target will not work.
SDCARD_VOLUME_NAME = RPI4BOOT

#  Determine the current user's login name. This is needed on Linux systems
#  that mount the SD card reader device at the following directory:
#  /run/media/<user_name>/<volume_name>.
USER_NAME = $(shell whoami)

#  The following determines the SD card volume pathname, the pathname for the
#  kernel8.img image file on the SD card, and the command to eject the SD card.
#  On the lab Linux machines, the removable SD card is located in the /media
#  directory, and the command to eject the SD card is "umount". On a Mac OS X
#  machine, the removable SD card is located in the /Volumes directory, and the
#  command to eject the SD card is "diskutil eject".
ifeq ($(INSTALLATION_TYPE), LINUX)
	SDCARD_VOLUME_PATH = /run/media/$(USER_NAME)/$(SDCARD_VOLUME_NAME)
	SDCARD_IMAGE_PATH = $(SDCARD_VOLUME_PATH)/kernel8.img
	DISK_EJECT_COMMAND = umount
else ifeq ($(INSTALLATION_TYPE), MAC_OS_X)
	SDCARD_VOLUME_PATH = /Volumes/$(SDCARD_VOLUME_NAME)
	SDCARD_IMAGE_PATH = $(SDCARD_VOLUME_PATH)/kernel8.img
	DISK_EJECT_COMMAND = diskutil eject
endif



#  The following is the volume name of the flash memory (eMMC) device on your
#  Compute Module 4. This name is usually assigned when the flash memory is
#  initialized, but it can be changed later if desired. Make sure the name of
#  flash memory device exactly matches the definition below, or the
#  'make flash' target will not work.
FLASH_VOLUME_NAME = CM4EMMC

#  The following determines the flash memory volume pathname, the pathname for
#  the kernel8.img image file on the device, and the command to eject the
#  volume. On the lab Linux machines, the volume is located in the
#  /run/media/<user_name> directory, and the command to eject the volume is
#  "umount". On a Mac OS X machine, the volume is located in the /Volumes
#  directory, and the command to eject the volume is "diskutil eject". The full
#  path to the rpiboot executable is also defined.
ifeq ($(INSTALLATION_TYPE), LINUX)
	FLASH_VOLUME_PATH = /run/media/$(USER_NAME)/$(FLASH_VOLUME_NAME)
	FLASH_IMAGE_PATH = $(FLASH_VOLUME_PATH)/kernel8.img
	DISK_EJECT_COMMAND = umount
	RPIBOOT_COMMAND = /usr/local/bin/rpiboot
else ifeq ($(INSTALLATION_TYPE), MAC_OS_X)
	FLASH_VOLUME_PATH = /Volumes/$(FLASH_VOLUME_NAME)
	FLASH_IMAGE_PATH = $(FLASH_VOLUME_PATH)/kernel8.img
	DISK_EJECT_COMMAND = diskutil eject
	RPIBOOT_COMMAND = /usr/local/bin/rpiboot
endif



#  The following are the complete paths to the gcc compiler, the as assembler,
#  the ld linker, the objcopy and objdump facilities, and the gdb debugger. They
#  depend on the install directory and prefix being correctly defined above.
GCC = $(INSTALL_DIRECTORY)$(PREFIX)gcc
AS = $(INSTALL_DIRECTORY)$(PREFIX)as
LD = $(INSTALL_DIRECTORY)$(PREFIX)ld
OBJCOPY = $(INSTALL_DIRECTORY)$(PREFIX)objcopy
OBJDUMP = $(INSTALL_DIRECTORY)$(PREFIX)objdump
GDB = $(INSTALL_DIRECTORY)gdb

#  The following are the complete paths to the openocd debugger and the Qemu
#  emulator. They depend on the install directory being correctly defined above.
OPENOCD = $(INSTALL_DIRECTORY)openocd
QEMU = $(INSTALL_DIRECTORY)qemu-system-aarch64

#  This following gives the name of the linker script file used by the ld linker
#  when linking together all the object (.o) files. This file should be in the
#  same directory as your source files and Makefile.
LINK_SCRIPT = link.ld

#  The following gives the suffixes assumed for the project's source code files
#  that will be compiled or assembled. All files ending in .asm or .s or .c will
#  be compiled or assembled into object code, and put into files ending in .o
ASM_SOURCE_FILES = $(wildcard *.asm)
S_SOURCE_FILES = $(wildcard *.s)
C_SOURCE_FILES = $(wildcard *.c)
ASM_OBJECT_FILES = $(ASM_SOURCE_FILES:.asm=.o)
S_OBJECT_FILES = $(S_SOURCE_FILES:.s=.o)
C_OBJECT_FILES = $(C_SOURCE_FILES:.c=.o)

#  These C flags are used when invoking gcc, and tell the compiler to show all
#  warnings, to do level 2 optimization, and to create freestanding code that
#  does not include the usual libraries and startup code.
C_FLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles

#  These link flags tell the ld linker not to include the usual libraries
LD_FLAGS = -nostdlib

#  These flags tell the objdump facility to disassemble code sections in the
#  executable (.elf file), to display source code intermixed with disassembly
#  (if possible), to display the full contents of any sections requested, and 
#  to display section header summaries.
OBJDUMP_FLAGS = -d -S -s -h



#  This is the Makefile's main target
all: kernel8.img

#  The following is a suffix rule that indicates how a file ending in .asm
#  should be processed to create a corresponding file ending in .o (i.e. a file
#  that contains object code). The .asm file is assumed to contain m4 macros
#  plus A64 assembly code. The .asm file is first run through the m4
#  preprocessor, and produces a corresponding .S file that contains pure
#  assembly code. Secondly, the .S file is assembled using the 'as' assembler,
#  producing a corresponding .o file.
%.o: %.asm
	m4 $< > $*.S
	$(AS) $*.S -o $@

#  The following suffix rule indicates how a file ending in .s should be
#  processed to create a corresponding file ending in .o (i.e. a file that
#  contains object code). The .s file should contain pure A64 assembly code
#  (i.e. no macros!).
%.o: %.s
	$(AS) $< -o $@

#  The following rule indicates how a file ending in .c should be processed to
#  create a corresponding file ending in .o (i.e. a file that contains object
#  code). The .c file should contain pure C code.
%.o: %.c
	$(GCC) $(C_FLAGS) -c $< -o $@

#  The following target indicates how to create the kernel8.elf file. This
#  target depends on all of the .o files created from .asm or .s or .c source
#  code files. The 'ld' linker links all these .o files together to create a
#  temporary kernel8.elf file.
kernel8.elf: $(ASM_OBJECT_FILES) $(S_OBJECT_FILES) $(C_OBJECT_FILES)
	$(LD) $(LD_FLAGS) $(ASM_OBJECT_FILES) $(S_OBJECT_FILES) $(C_OBJECT_FILES) \
	    -T $(LINK_SCRIPT) -o kernel8.elf
	    
#  The following target shows how to create the kernel8.img file. The 'objcopy'
#  facility creates a kernel8.img file from the .elf file, and then 'objdump' is
#  invoked to create a kernel8.dump text file, which shows the structure and
#  contents of the .elf file.
kernel8.img: kernel8.elf
	$(OBJCOPY) -O binary kernel8.elf kernel8.img
	$(OBJDUMP) $(OBJDUMP_FLAGS) kernel8.elf > kernel8.dump

#  This target removes all intermediate files with the .elf, .o, .S, .dump, and
#  .log suffixes. Any warning or error messages are thrown away (redirected to
#  /dev/null), and if errors occur, processing will still continue.
.PHONY: clean
clean:
	rm *.elf *.o *.S *.dump *.log >/dev/null 2>/dev/null || true

#  This target removes all files with the .img, .elf, .o, .S, .dump, and .log
#  suffixes. Any warning or error messages are thrown away (redirected to
# /dev/null), and if errors occur, processing will still continue.
.PHONY: deepclean
deepclean:
	rm *.img *.elf *.o *.S *.dump *.log >/dev/null 2>/dev/null || true
	
#  The following target runs the kernel8.img file in the Qemu emulator while
#  emulating a Raspberry Pi 4b device. Any serial I/O is handled using standard
#  input and output.
.PHONY: run
run: kernel8.img
	$(QEMU) -M raspi4b -kernel kernel8.img -serial null -serial stdio
	
#  The following target deletes the existing kernel8.img file (if it exists)
#  from the SD card, and then copies the newly-created kernel8.img file to the
#  SD card. Next, the files installed on the SD card are listed, after which the
#  SD card is "ejected" (unmounted) so that it can be removed manually from the
#  SD card reader.
.PHONY: sdcard
sdcard: kernel8.img
	rm -f $(SDCARD_IMAGE_PATH)
	cp kernel8.img $(SDCARD_IMAGE_PATH)
	ls -l $(SDCARD_VOLUME_PATH)
	$(DISK_EJECT_COMMAND) $(SDCARD_VOLUME_PATH)
	@echo "You may now remove the SD card from the SD card reader"
	
#  The following target first boots the Compute Module from USB so that its
#  flash memory device appears as removable disk volume. Next, it deletes the
#  existing kernel8.img file (if it exists) from the volume, and then copies
#  the newly-created kernel8.img file to the device. Next, the files installed
#  on the device are listed, after which the flash memory device (volume) is
#  "ejected" (unmounted) so that power can be removed from the Compute Module.
.PHONY: flash
flash: kernel8.img
	@echo " "
	@echo "Before writing to the flash memory device, make sure you do"
	@echo "the following steps:"
	@echo " "
	@echo "  1. Remove power to the Compute Module."
	@echo "  2. Place a shorting block on Pins 1 & 2 of the J2 Header."
	@echo "  3. Connect a USB cable from the host to the J11 USB slave port."
	@echo " "
	@echo "Apply power to the Compute Module after the 'rpiboot' command"
	@echo "starts below."
	@echo " "
	@echo "Press any key when you are ready to proceed:"
	@read line
	$(RPIBOOT_COMMAND)
	@echo " "
	@echo "Waiting for the volume to be mounted..."
	@sleep 3
	@echo " "
	rm -f $(FLASH_IMAGE_PATH)
	@sleep 1
	cp kernel8.img $(FLASH_IMAGE_PATH)
	@sleep 1
	ls -l $(FLASH_VOLUME_PATH)
	@sleep 1
	$(DISK_EJECT_COMMAND) $(FLASH_VOLUME_PATH)
	@echo " "
	@echo "Writing to the flash memory is completed. To boot from flash"
	@echo "memory on the Compute Module, remove power, disconnect the USB"
	@echo "cable, and remove the shorting block. Then reapply power."
	@echo " "
	
#  The follow target starts a debugging session using the GDB debugger. The
#  commands in the gdb_jtag.init file tell GDB to open a connection to the
#  Raspberry Pi by starting an instance of an OpenOCD GDB server. The server
#  communicates with the Pi using the JTAG transport via a USB-to-JTAG interface
#  device (usually the Pi4 Hat V3). The openocd.cfg and gdb_jtag.init files
#  must be in the current working directory in order for this to work.
.PHONY: gdb
gdb: kernel8.elf
	$(GDB) -q -x gdb_jtag.init kernel8.elf
    
#  The following target prints out the Makefile version number, the installation
#  and machine type, the name of the link script, the SD card volume name, the
#  flash volume name, the current user's login name, and the version numbers of
#  the assembler, compiler, debugger, binary utilities, OpenOCD, and the Qemu
#  emulator.
.PHONY: info
info:
	@echo "Makefile version:     " $(MAKEFILE_VERSION)
	@echo "Installation type:    " $(INSTALLATION_TYPE)
	@echo "Machine type:         " $(MACHINE_TYPE)
	@echo "Link script:          " $(LINK_SCRIPT)
	@echo "SD card volume name:  " $(SDCARD_VOLUME_NAME)
	@echo "Flash volume name:    " $(FLASH_VOLUME_NAME)
	@echo "User name:            " $(USER_NAME)
	@echo " "
	@$(AS) --version
	@echo " "
	@$(GCC) --version
	@$(GDB) --version
	@echo " "
	@$(LD) --version
	@echo " "
	@$(OBJCOPY) --version
	@echo " "
	@$(OBJDUMP) --version
	@echo " "
	@$(OPENOCD) --version
	@echo " "
	@$(QEMU) --version
	@echo " "
