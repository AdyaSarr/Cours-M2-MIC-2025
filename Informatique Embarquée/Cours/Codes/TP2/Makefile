MCU  := atmega328p
SRC  := main.s         
ELF  := prog.elf
HEX  := prog.hex
LST  := prog.lst
PORT := /dev/cu.usbmodem31301 # Version Mac et voici celle de linux /dev/ttyACM0

all: $(HEX)

# assemblage 
$(ELF): $(SRC)
	avr-gcc -mmcu=$(MCU) -nostartfiles -o $@ $<

# dump lisible
dump: $(LST)
$(LST): $(ELF)
	avr-objdump -d $< > $@

# conversion en .hex
$(HEX): $(ELF)
	avr-objcopy -O ihex $< $@

# flash sur la carte
flash: $(HEX)
	avrdude -p$(MCU) -carduino -P$(PORT) -D -U flash:w:$(HEX)

clean:
	rm -f $(ELF) $(HEX) $(LST)
