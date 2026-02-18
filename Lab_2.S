.include "m328Pdef.inc"
.cseg
.org 0

; Configure PB0, PB1, PB2 as output pins.
sbi DDRB, 0 ; Set PB0 to output (Serial Input to Shift Register)
sbi DDRB, 1 ; Set PB1 to output (Right Shift Clock)
sbi DDRB, 2 ; Set PB2 to output (Shift Register Clock)

cbi DDRB, 3 ; Set PB3 to input (Button)

sbi PORTB, 3    ; enable pull-up resistor

; R20 -> mode (increment/decrement)
; R19 -> Button hold time counter
; R18 -> index of character in lookup table
; R16 -> Character value to send to display subroutine
; R17 -> General use register
; R0 -> Contents are always zero
; R21:R20 -> 16-bit counter for waiting for button release

; Constants
.equ ONE_SECOND = 200 ; Interations for approximately 1 second delay (assuming 16 MHz clock)
.equ TWO_SECONDS = 400 ; Interations for approximately 2 seconds delay (assuming 16 MHz clock)

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

; Implement input function to read user input
input:
    ; Check if button is pressed (active low)
    sbic PINB, 3 ; skip if bit clear (pressed)
    ret ; return to main loop if button is not pressed

    ; Debounce delay
    rcall delay_5ms

    ; Check if button is still pressed after debounce delay
    sbic PINB, 3
    ret ; returns if button is not pressed (false trigger)

    ; If we reach here, the button is confirmed as pressed
    inc r18 ; increment index

    ; Wrap around if index exceeds 15 (for hex digits 0-F)
    cpi r18, 16 ; compare r18 value to 16
    clr r19 ; reset button hold time counter
    brlo wait_release ; branch to wait_release if value less than 16
    clr r18 ; reset index to 0 if it exceeds 15

; Wait for button release (active low)
wait_release:
    sbis PINB, 3 ; skip if released
    rjmp button_held ; wait until button is released
    
    rcall delay_5ms ; release debounce
    ret ; return to main loop once button is released

; If the button is held down, increment the hold time counter until the button is released
button_held:
    rcall delay_5ms     ; time base = 5 ms

    ; increment 16-bit counter
    inc r20
    brne no_carry
    inc r21

no_carry:
    rjmp wait_release

; delay function for approximately 5 milliseconds (assuming 16 MHz clock)
delay_5ms:
    ldi r20, 60
d1:
    ldi r21, 200
d2:
    dec r21
    brne d2
    dec r20
    brne d1
    ret

load_char:
	; Save contents of Z pointer
	push R30
	push R31

	add ZL, R18 ; Add R18 index to the Z pointer
	clr R0 ; make sure that R0 is zero
	adc ZH, R0 ; Handle 16-bit carry

	lpm R16, Z ; Load char value into R16

	; restore contents of Z pointer to lookup table
	pop r31
    pop r30
    ret

check_hold_time:
    ; store registers that will be used in this function
    push R20
    push R21
    push R19
    push R18

    ldi R19, high(ONE_SECOND) ; Load high byte of ONE_SECOND into R19
    ldi R20, low(ONE_SECOND)  ; Load low byte of ONE_SECOND into R20

    ; Check if button is held for 1 second
    cp R21, R19 ; Compare high byte of counter to high byte of ONE_SECOND
    cpc R20, R20 ; Compare low byte of counter to low byte of ONE_SECOND

    ; restore registers before returning to main loop
    pop R19
    pop R21
    pop R20
    ret

main:
    rcall input ; Call the input function to get user input

    rcall check_hold_time ; Check if the button is held down for 1 or 2 seconds and update index accordingly

	rcall load_char ; Load char value from lookup table at R18 into R16 to send to display

    ; ori R16, 0x80 ; enable decimal point

    rcall display ; Call the display function to show the character
    rjmp main ; jump to main loop

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