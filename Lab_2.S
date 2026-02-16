.include "m328Pdef.inc"
.cseg
.org 0

; Configure PB0, PB1, PB2 as output pins.
sbi   DDRB,0      ; PB0 is now output
sbi   DDRB,1      ; PB1 is now output
sbi   DDRB,2      ; PB2 is now output

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
    in R17, SREG
    push R17

    ldi R17, 8 ; send 8 bits serially to shift register

loop:
    ror R16 ; rotate right through Carry
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
    
    dec R17
    brne loop ; branch to loop if R17 is not zero

	; put code here to generate RCLK pulse (triggers right shift)
    sbi PORTB, 1 ; Set RCLK pin (assuming RCLK is connected to PB1)
    cbi PORTB, 1 ; Clear RCLK pin (assuming RCLK is connected to PB1)
    
    ; restore registers from stack
    pop R17
    out SREG, R17
    pop R17
    pop R16
    ret

input:
    ; Implement input function to read user input (e.g., from buttons or serial)

    ; TODO: implement logic to modify R16 based on user input (e.g., increment/decrement character index)

	ldi ZH, high(seg_hex_table<<1)
    ldi ZL, low(seg_hex_table<<1)

    ldi R16, 0        ; index
    add ZL, R16
    adc ZH, R1

    lpm R16, Z
    ret

main:
    rcall input ; Call the input function to get user input
    rcall display ; Call the display function to show the character
    rjmp main ; jump to main loop

delay:
    ldi r18, 85        ; outer loop
L1:
    ldi r19, 255
L2:
    ldi r20, 255
L3:
    dec r20
    brne L3

    dec r19
    brne L2

    dec r18
    brne L1

    ret

test:
    ldi ZH, high(seg_hex_table<<1)
    ldi ZL, low(seg_hex_table<<1)

    inc R16 ; increment index to get next character

	add ZL, R16 ; Move to the correct position in the table
    lpm R16, Z ; Load the character pattern into R16
	rcall display ; show digit
    rcall delay ; wait

    inc r18 ; next hex digit
    cpi r18, 16 ; compare r18 value to 16
    brlo test ; branch to test if value to low

    clr r18            ; restart at 0
    rjmp test

; Start of main program (initialize registers, call display function, etc.)
start:
	; PB0 = SER
    ; PB1 = RCLK
    ; PB2 = SRCLK

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

    rjmp test ; jump to main loop

.exit