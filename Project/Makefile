# Target MCU - atmega328p is the chip used on most Arduino Unos
MCU    = atmega328p

# CPU frequency in Hz - must match your hardware (16MHz for Arduino Uno)
# This is passed as a preprocessor definition so that _delay_ms() and
# other time-sensitive macros work correctly in your C code
F_CPU  = 16000000UL

# The AVR GCC cross-compiler - compiles C code targeting AVR architecture
# instead of your host machine's architecture (x86/ARM/etc.)
CC     = avr-gcc

# Compiler flags:
#   -Wall        : enable all warnings
#   -Os          : optimize for size (important on a chip with 32KB flash)
#   -mmcu        : tell the compiler which AVR chip to target
#   -DF_CPU      : pass F_CPU as a preprocessor #define to all source files
CFLAGS = -Wall -Os -mmcu=$(MCU) -DF_CPU=$(F_CPU)

# The name used for output files (.elf, .hex) - usually just main
TARGET = main

# The serial port the Arduino is connected to.
# Linux:   typically /dev/ttyACM0 or /dev/ttyUSB0
# macOS:   typically /dev/cu.usbmodem*
# Windows: typically COM3, COM4, etc.
PORT   = /dev/ttyACM0

# Baud rate used by avrdude to talk to the Arduino's bootloader
BAUD   = 115200

# Automatically find every .c file in the current directory
SRCS = $(wildcard *.c)

# Generate a list of .o object files by replacing .c extension with .o
# e.g. main.c lcd.c -> main.o lcd.o
OBJS = $(SRCS:.c=.o)

# .PHONY tells Make that these targets are not real files.
# Without this, Make would get confused if a file named 'all',
# 'flash', or 'clean' ever existed in the directory.
.PHONY: all flash clean

# Default target - builds the final .hex file ready for flashing
all: $(TARGET).hex

# Pattern rule: compiles any .c file into a .o object file.
#   -c  : compile only, do not link (produces a .o, not an executable)
#   $<  : the first prerequisite (the .c file)
#   $@  : the target (the .o file)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Links all .o object files together into a single ELF binary.
# ELF is a standard executable format that contains code, debug info, etc.
#   $^  : all prerequisites (all .o files)
#   $@  : the target (the .elf file)
$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Converts the ELF binary into Intel HEX format, which is what
# avrdude actually flashes onto the chip.
#   -O ihex    : output format is Intel HEX
#   -R .eeprom : exclude the EEPROM section (flashed separately if needed)
#   $<         : the first prerequisite (the .elf file)
#   $@         : the target (the .hex file)
$(TARGET).hex: $(TARGET).elf
	avr-objcopy -O ihex -R .eeprom $< $@
	rm -f $(OBJS) $(TARGET).elf

# Flashes the .hex file onto the chip via avrdude.
# Depends on $(TARGET).hex so it will rebuild first if source has changed.
#   -c arduino : use the Arduino bootloader protocol (no external programmer needed)
#   -p m328p   : target chip is the ATmega328P
#   -P         : serial port the Arduino is connected to
#   -b         : baud rate for communication with the bootloader
#   -U flash:w : write to flash memory
flash: $(TARGET).hex
	avrdude -c arduino -p m328p -P $(PORT) -b $(BAUD) \
	        -U flash:w:$(TARGET).hex

# Removes all generated files so you can start a clean build.
# Deletes all .o object files, the .elf binary, and the .hex file.
clean:
	rm -f $(OBJS) *.elf *.hex