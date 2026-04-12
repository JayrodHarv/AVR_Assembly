MCU = atmega328p
F_CPU = 16000000UL
CC = avr-gcc

CFLAGS = -Wall -Os -mmcu=$(MCU) -DF_CPU=$(F_CPU)

TARGET = main
PORT = /dev/ttyACM0   # <-- change if needed
BAUD = 115200

all:
	$(CC) $(CFLAGS) -o $(TARGET).elf $(TARGET).c
	avr-objcopy -O ihex -R .eeprom $(TARGET).elf $(TARGET).hex

flash:
	avrdude -c arduino -p m328p -P $(PORT) -b $(BAUD) -U flash:w:$(TARGET).hex

clean:
	rm -f *.elf *.hex