.include "m328Pdef.inc"
.cseg
.org 0

; Constants
.equ SERL = 0 ; Assuming SER is connected to PB0
.equ RCLK = 1 ; Assuming RCLK is connected to PB1
.equ SRCLK = 2 ; Assuming SRCLK is connected to PB2
.equ BTN = 3 ; Assuming button is connected to PB3
.equ ONE_SECOND = 200 ; Interations for approximately 1 second delay (assuming 16 MHz clock)
.equ TWO_SECONDS = 400 ; Interations for approximately 2 seconds delay (assuming 16 MHz clock)

; Configure PB0, PB1, PB2 as output pins.
sbi DDRB, SERL ; Set PB0 to output (Serial Input to Shift Register)
sbi DDRB, RCLK ; Set PB1 to output (Right Shift Clock)
sbi DDRB, SRCLK ; Set PB2 to output (Shift Register Clock)

cbi DDRB, BTN ; Set PB3 to input (Button)
sbi PORTB, BTN ; enable internal pull-up resistor

; R25:R24 -> 16-bit counter for waiting for button release
; R20 -> mode (increment/decrement)
; R19 -> Button hold time counter
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
    cbi PORTB, SERL ; Clear SER pin (assuming SER is connected to PB0)
    rjmp end

set_ser_in_1:
    sbi PORTB, SERL ; Set SER pin (assuming SER is connected to PB0)

end:
    ; generate SRCLK pulse
    sbi PORTB, SRCLK ; Set SRCLK pin (assuming SRCLK is connected to PB2)
    cbi PORTB, SRCLK ; Clear SRCLK pin (assuming SRCLK is connected to PB2)
    
    dec R17 ; decrement bit counter
    brne loop ; branch to loop if R17 is not zero

	; generate RCLK pulse (triggers right shift)
    sbi PORTB, RCLK ; Set RCLK pin (assuming RCLK is connected to PB1)
    cbi PORTB, RCLK ; Clear RCLK pin (assuming RCLK is connected to PB1)
    
    ; restore registers from stack
    pop R17
    out SREG, R17 ; restore status register
    pop R17
    pop R16
    ret

; Implement input function to read user input
input:
    ; Check if button is pressed (active low)
    sbic PINB, BTN ; skip if bit clear (pressed)
    ret ; return to main loop if button is not pressed

    rcall delay_5ms ; Debounce delay

    ; Check if button is still pressed after debounce delay
    sbic PINB, BTN
    ret ; returns if button is not pressed (false trigger)

    ; Clear hold time counter
    clr r24
    clr r25

    rcall wait_release ; Wait for button release and count hold time
    rcall check_hold_time ; Check if the button is held down for 1 or 2 seconds and update index accordingly
    ret ; return to main loop after handling button press

; Wait for button release (active low)
wait_release:
    sbis PINB, BTN ; skip if button is released (not pressed)
    rjmp button_held ; if button is still pressed, jump to button_held to continue counting hold time
    
    rcall delay_5ms ; release debounce
    ret ; return to input label

; If the button is held down, increment the hold time counter until the button is released
button_held:
    rcall delay_5ms ; increment every 5ms
    ADIW R24,1 ; Increment the 16-bit counter in R25:R24

no_carry:
    rjmp wait_release

; delay function for approximately 5 milliseconds (assuming 16 MHz clock)
delay_5ms:
    ; save registers that will be used in this function
    push R24
    push R25

    ldi r24, 60
d1:
    ldi r25, 200
d2:
    dec r25
    brne d2
    dec r24
    brne d1

    ; restore registers before returning
    pop R25
    pop R24
    ret

load_char:
    ; Reset Z pointer to the start of the lookup table
	ldi ZH, high(seg_hex_table<<1)
    ldi ZL, low(seg_hex_table<<1)

	add ZL, R18 ; Add R18 index to the Z pointer
	clr R0 ; make sure that R0 is zero
	adc ZH, R0 ; Handle 16-bit carry

	lpm R16, Z ; Load char value into R16

    or R16, R20 ; XOR with mode to toggle between increment and decrement character sets if needed

    ret 

; Increment or decrement depending on mode
short_press:
    sbrc R20,7 ; Check mode bit (bit 7 of R20)
    rjmp dec_mode ; if mode bit is set, jump to decrement mode

; increment
    inc R18
    andi R18, 0x0F ; wrap 0–>15 automatically
    ret

dec_mode:
    dec R18
    andi R18, 0x0F ; wrap 15->0 automatically
    ret

medium_press:
    ; Toggle mode between increment and decrement
    ldi R17, 0x80 ; Load 1 into R17 for toggling (bit 7)
    eor R20, R17 ; Toggle the mode bit in R20 (0 for increment, 1 for decrement)
    ret ; return to main loop after handling medium press

check_hold_time:
    ; store registers that will be used in this function
    push R25
    push R24
    push R23
    push R22

    ; Check if button is held for less than 1 second
    ldi R22, low(ONE_SECOND)  ; Load low byte of ONE_SECOND into R22
    ldi R23, high(ONE_SECOND) ; Load high byte of ONE_SECOND into R23

    cp R24, R22 ; Compare low byte of counter to low byte of ONE_SECOND
    cpc R25, R23 ; Compare high byte of counter to high byte of ONE_SECOND
    brlo short_case ; If counter is less than ONE_SECOND, it's a short press

    ; Check if button is held for 1 second or more but less than 2 seconds
    ldi R22, low(TWO_SECONDS)  ; Load low byte of TWO_SECONDS into R22
    ldi R23, high(TWO_SECONDS) ; Load high byte of TWO_SECONDS into R23
    cp R24, R22 ; Compare low byte of counter to low byte of TWO_SECONDS
    cpc R25, R23 ; Compare high byte of counter to high byte of TWO_SECONDS
    brlo medium_case ; If counter is less than TWO_SECONDS, it's a medium press

long_press:
    ; If the button is held for 2 seconds or more, it's a long press
    clr R18 ; Clear R18 for use in long press logic
    rjmp exit_check ; jump to exit after handling long press (or implement long press logic here)

short_case:
    rcall short_press
    rjmp exit_check

medium_case:
    rcall medium_press

    ; restore registers before returning to main loop
exit_check:
    pop R22
    pop R23
    pop R24
    pop R25
    ret

main:
    rcall input ; Call the input function to get user input
	rcall load_char ; Load char value from lookup table at R18 into R16 to send to display
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