.include "m328Pdef.inc"
.cseg
.org 0

; Configure PB0, PB1, PB2 as output pins.
sbi DDRB, 0 ; Set PB0 to output (Serial Input to Shift Register)
sbi DDRB, 1 ; Set PB1 to output (Right Shift Clock)
sbi DDRB, 2 ; Set PB2 to output (Shift Register Clock)

cbi DDRB, 3 ; Set PB3 to input (Button)

sbi PORTB, 3    ; enable pull-up resistor

; R18 -> index of character in lookup table
; R16 -> Character value to send to display subroutine
; R17 -> General use register
; R0 -> Contents are always zero

; Lookup table for 7-segment display characters (0-9, A-F)
seg_hex_table:
	; DP most significant -> A least significant
    ;   0    1    2    3    4    5    6    7
    .db 0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07
    ;   8    9    A    B    C    D    E    F
    .db 0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71

rjmp start ; jump to the start label

; Shift byte to register and generate pulses for SRCLK and RCLK
display:
    ; backup used registers on stack
    push R16
    push R17
    in R17, SREG ; backup status register
    push R17

    ldi R17, 8 ; send 8 bits serially to shift register

loop:
    ror R16 ; rotate right through Carry (Least Significant Bit goes into Carry)
    BRCS set_ser_in_1 ; branch if Carry is set

    ; put code here to set SER to 0
    cbi PORTB, 0 ; Clear SER pin (assuming SER is connected to PB0)
    
    rjmp end

set_ser_in_1:
    ; put code here to set SER to 1
    sbi PORTB, 0 ; Set SER pin (assuming SER is connected to PB0)

end:
    ; put code here to generate SRCLK pulse
    sbi PORTB, 2 ; Set SRCLK pin (assuming SRCLK is connected to PB2)
    cbi PORTB, 2 ; Clear SRCLK pin (assuming SRCLK is connected to PB2)
    
    dec R17 ; decrement bit counter
    brne loop ; branch to loop if R17 is not zero

	; put code here to generate RCLK pulse (triggers right shift)
    sbi PORTB, 1 ; Set RCLK pin (assuming RCLK is connected to PB1)
    cbi PORTB, 1 ; Clear RCLK pin (assuming RCLK is connected to PB1)
    
    ; restore registers from stack
    pop R17
    out SREG, R17 ; restore status register
    pop R17
    pop R16
    ret

button_pressed:
	inc r18 ; increment r18
	ret

	; Implement input function to read user input (e.g., from buttons or serial)
input:
	; Save contents of registers
	push R17

	clr R17 ; clear register for debugging purposes
    
	in r17, PINB ; read in input from button
	sbrs r17, 3  ; skip next if bit 3 is set (not pressed)
	rcall button_pressed ; jump to button press subroutine

	cpi  r18,16 ; compare r18 & 16 (r18 - 16)
	brlo ok ; bypass clear if index in bounds
	clr  r18 ; reset to zero if overflow
ok:

	; restore contents of registers
	pop R17
    ret

load_char:
	; Save contents of Z pointer
	push R30
	push R31

	; Z = start of lookup table
	ldi ZH, high(seg_hex_table<<1)
    ldi ZL, low(seg_hex_table<<1)

	add ZL, R18 ; Add R18 index to the Z pointer

	adc ZH, R0 ; Handle 16-bit carry

	lpm R16, Z ; Load char value into R16

	; restore contents of Z pointer to lookup table
	pop r31
    pop r30
    ret

main:
    rcall input ; Call the input function to get user input
	rcall load_char ; Load char value from lookup table at R18 into R16 to send to display
    rcall display ; Call the display function to show the character
    rjmp main ; jump to main loop

delay:
    ldi r21, 85        ; outer loop
L1:
    ldi r19, 255
L2:
    ldi r20, 255
L3:
    dec r20
    brne L3

    dec r19
    brne L2

    dec r21
    brne L1

    ret

test:
	ldi ZH, high(seg_hex_table<<1)
	ldi ZL, low(seg_hex_table<<1)

	add ZL, R18 ; Move to the correct position in the table
    lpm R16, Z ; Load the character pattern into R16
	rcall display ; show digit
    rcall delay ; wait

    inc r18 ; next hex digit
    cpi r18, 16 ; compare r18 value to 16
    brlo test ; branch to test if value less than 16

    clr r18 ; restart at 0
    rjmp test

test_char:
	ldi R16, 15 ; Initially, load index for character '0'
    add ZL, R16 ; Move to the correct position in the table
    lpm R16, Z ; Load the character pattern into R16

	rcall display

forever:
	rjmp forever

; Start of main program (initialize registers, call display function, etc.)
start:
	; Initialize the stack
	ldi r16, high(RAMEND)
	out SPH, r16
	ldi r16, low(RAMEND)
	out SPL, r16

    ; Initialize registers and ports here
    ldi ZH, high(seg_hex_table<<1)
	ldi ZL, low(seg_hex_table<<1)
    ldi R16, 0 ; Initially, load index for character '0'
    add ZL, R16 ; Move to the correct position in the table
    lpm R16, Z ; Load the character pattern into R16

    rjmp main ; jump to main loop

.exit