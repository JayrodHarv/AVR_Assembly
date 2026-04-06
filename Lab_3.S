.include "m328Pdef.inc"

.dseg
.org 0x100 ; Start of data segment for variables
seg_hex_table: .byte 17 ; Lookup table for 7-segment display characters (-, 0-9, A-F)
passcode: .byte 5 ; hardcoded passcode
userInput: .byte 5
.cseg
.org 0

#define SOFTWARE_DEBOUNCE

; Constants
.equ SERL = 0 ; Assuming SER is connected to PB0
.equ RCLK = 1 ; Assuming RCLK is connected to PB1
.equ SRCLK = 2 ; Assuming SRCLK is connected to PB2
.equ BTN = 3 ; Assuming button is connected to PB3
.equ ENC_A = 2
.equ ENC_B = 3
.equ LED = 5

.equ ONE_SECOND = 200 ; Interations for approximately 1 second delay (assuming 16 MHz clock)
.equ TWO_SECONDS = 400 ; Interations for approximately 2 seconds delay (assuming 16 MHz clock)

.def stop_timer = r25
.def start_timer = r24
.def enc_curr = r14
.def enc_prev = r15
.def enc_trans = r21
.def enc_step = r19
.def num_inputs = r13 

; Configure PB0, PB1, PB2 as output pins.
sbi DDRB, SERL ; Set PB0 to output (Serial Input to Shift Register)
sbi DDRB, RCLK ; Set PB1 to output (Right Shift Clock)
sbi DDRB, SRCLK ; Set PB2 to output (Shift Register Clock)
sbi DDRB, LED

cbi DDRB, BTN ; Set PB3 to input (Button)
#ifdef SOFTWARE_DEBOUNCE
	sbi PORTB, BTN ; enable internal pull-up resistor
#endif

cbi DDRD, ENC_A
cbi DDRD, ENC_B
sbi PORTD, ENC_A
sbi PORTD, ENC_B ;pull up resistors

; R25:R24 -> 16-bit counter for waiting for button release
; R31:R30 -> Variable timer counter
; R18 -> index of character in lookup table
; R16 -> Character value to send to display subroutine
; R17 -> General use register
; R0 -> Contents are always zero

cbi PORTB, LED ; turns off the yellow LED on board
rjmp start ; jump to the start label

; variable delay subroutine
delay_ms:
    push R16

delay_outer:
wait_ovf:
	in r16, TIFR0
	sbrs r16, TOV0
	rjmp wait_ovf

    ; Clear overflow flag
	ldi r16, (1<<TOV0)
	out TIFR0, r16

	; decrement 16-bit counter
    sbiw r30, 1
    brne delay_outer

	pop r16
	ret

delay_5ms:
    ldi r30, 5
    clr r31
    rjmp delay_ms

delay_4s:
    ldi r30, low(4000)
    ldi r31, high(4000)
    rjmp delay_ms

delay_7s:
    ldi r30, low(7000)
    ldi r31, high(7000)
    rjmp delay_ms

; Shift byte to register and generate pulses for SRCLK and RCLK
display:
    ; backup used registers on stack
    push R16
    push R17
    in R17, SREG ; backup status register
    push R17

    ldi R17, 8 ; send 8 bits serially to shift register

loop:
    rol R16 ; rotate left through Carry (Least Significant Bit goes into Carry)
    BRCS set_ser_in_1 ; branch if Carry is set
    cbi PORTB, SERL ; Clear SER pin (assuming SER is connected to PB0)
    rjmp end

set_ser_in_1:
    sbi PORTB, SERL ; Set SER pin (assuming SER is connected to PB0)

end:
    ; generate SRCLK pulse triggers right shift
    sbi PORTB, SRCLK ; Set SRCLK pin 
    cbi PORTB, SRCLK ; Clear SRCLK pin 
    
    dec R17 ; decrement bit counter
    brne loop ; branch to loop if R17 is not zero

	; generate RCLK pulse to parallel store
    sbi PORTB, RCLK ; Set RCLK pin 
    cbi PORTB, RCLK ; Clear RCLK pin 
    
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

	#ifdef SOFTWARE_DEBOUNCE
		rcall delay_5ms ; Debounce delay

		; Check if button is still pressed after debounce delay
		sbic PINB, BTN
		ret ; returns if button is not pressed (false trigger)
	#endif

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

load_char:
    push r26
    push r27

    clr r26
    add r26, r18
    adc r27, r1

    ld r16, X

    pop r27
    pop r26
    ret

check_hold_time:
    push r16
    push R22
    push R23
    
    ; Check if button is held for less than 1 second
    ldi R22, low(ONE_SECOND)
    ldi R23, high(ONE_SECOND)

    ; Compare hold time counter to 1 second
    cp R24, R22
    cpc R25, R23
    brlo short_press ; If counter is less than 1 second, it's a short press

long_press:
    ; If the button is held for 2 seconds or more, it's a long press
    clr R18 ; reset character offset
	clr num_inputs
	ldi YL, low(userInput)
	ldi YH, high(userInput)
    rjmp exit_check

; Input digit of passcode
short_press:
	st Y+, r18
	inc num_inputs
	clr r18

	ldi r16, 5
	cp num_inputs, r16
	brlo exit_check
	rjmp check_passcode

exit_check:
    pop R23
    pop R22
    pop r16
	ret
	
check_passcode:
    ; Z -> passcode, Y -> userInput
    ldi ZL, low(passcode)
    ldi ZH, high(passcode)

    ldi YL, low(userInput)
    ldi YH, high(userInput)

    ldi r22, 5              ; loop counter

compare_loop:
    ld  r19, Z+             ; passcode[i]
    ld  r20, Y+             ; userInput[i]
    cp  r19, r20
    brne wrong              ; mismatch -> fail

    dec r22
    brne compare_loop

    rjmp correct            ; all 5 matched

wrong:
    ldi r16, 0b00001000 ; enable only the D segment for error indication
    rcall display ; update display
    rcall delay_7s ; delay for 7 seconds
    rjmp compare_exit

correct:
    sbi PORTB, LED ; turn on yellow LED
    ldi r16, 0b10000000 ; enable only the DP segment
    rcall display ; update display
    rcall delay_4s ; delay for 4 seconds
    cbi PORTB, LED ; turn off yellow LED

compare_exit:
    ; reset for next attempt
    clr num_inputs
    clr r18
    ldi YL, low(userInput)
    ldi YH, high(userInput)
    ret

enc_read_state:
	in enc_curr, PIND
	lsr enc_curr
	lsr enc_curr
	ldi r30, 0x03        
    and enc_curr, r30 
	ret

enc_update:
    rcall enc_read_state

    ; return if no change
	cp enc_curr, enc_prev
	breq enc_update_done

	mov enc_trans, enc_prev
	lsl enc_trans
	lsl enc_trans
	or enc_trans, enc_curr

	; ---- CW cases: 1, 7, 14, 8 ----
    cpi enc_trans, 1
    breq enc_cw
    cpi enc_trans, 7
    breq enc_cw
    cpi enc_trans, 14
    breq enc_cw
    cpi enc_trans, 8
    breq enc_cw

	 ; ---- CCW cases: 2, 4, 13, 11 ----
    cpi enc_trans, 2
    breq enc_ccw
    cpi enc_trans, 4
    breq enc_ccw
    cpi enc_trans, 13
    breq enc_ccw
    cpi enc_trans, 11
    breq enc_ccw

	rjmp enc_update_done

enc_cw:
    inc enc_step              ; +1 per CW edge
    cpi enc_step, 4
    brne enc_update_done      ; not a full detent yet

    clr enc_step              ; reached +4 edges -> one step
    cpi r18, 16
	brge enc_update_done
	inc R18
    rjmp enc_update_done

enc_ccw:
    dec enc_step              ; -1 per CCW edge
    cpi enc_step, 0xFC        ; -4 (two's complement)
    brne enc_update_done      ; not a full detent yet

    clr enc_step              ; reached -4 edges -> one step
	cpi r18, 2
	brlo enc_update_done
    dec R18
    rjmp enc_update_done

enc_update_done: 
	mov enc_prev, enc_curr
	ret

; main loop to continuously check for button presses and update display
main:
	rcall input
    rcall enc_update   ; update R18 from encoder direction
    rcall load_char    ; R18 -> segment pattern in R16
    rcall display
    rjmp main

; Start of main program (initialize registers, call display function, etc.)
start:
	; Initialize the stack
	ldi r16, high(RAMEND)
	out SPH, r16
	ldi r16, low(RAMEND)
	out SPL, r16

    ; Initialize lookup table
    ; X -> seg_hex_table in SRAM
    ldi XL, low(seg_hex_table)
    ldi XH, high(seg_hex_table)
	ldi YL, low(passcode)
	ldi YH, high(passcode)

    ; Now store values one-by-one
	;bits go from dp to A
	ldi r16, 0b01000000 ; -
	st X+, r16
    ldi r16, 0b00111111 ; 0
    st  X+, r16
    ldi r16, 0b00000110 ; 1
    st  X+, r16
    ldi r16, 0b01011011 ; 2
    st  X+, r16
    ldi r16, 0b01001111 ; 3
    st  X+, r16
    ldi r16, 0b01100110 ; 4
    st  X+, r16
    ldi r16, 0b01101101 ; 5
    st  X+, r16
    ldi r16, 0b01111101 ; 6
    st  X+, r16
    ldi r16, 0b00000111 ; 7
    st  X+, r16
    ldi r16, 0b01111111 ; 8
    st  X+, r16
    ldi r16, 0b01101111 ; 9
    st  X+, r16
    ldi r16, 0b01110111 ; A
    st  X+, r16
    ldi r16, 0b01111100 ; B
    st  X+, r16
    ldi r16, 0b00111001 ; C
    st  X+, r16
    ldi r16, 0b01011110 ; D
    st  X+, r16
    ldi r16, 0b01111001 ; E
    st  X+, r16
    ldi r16, 0b01110001 ; F
    st  X+, r16

    ; Initialize passcode
	ldi r16, 0x0B
	st Y+, r16
	ldi r16, 0x10
	st Y+, r16
	ldi r16, 0x02
	st Y+, r16
	ldi r16, 0x01
	st Y+, r16
	ldi r16, 0x07
	st Y+, r16

    ; reset X pointer to start of table for later use
    ldi XL, low(seg_hex_table)
    ldi XH, high(seg_hex_table)

	ldi YL, low(userInput)
	ldi YH, high(userInput)
	clr num_inputs

    ; Initialize timer0 for delay functions
    ldi r16, 0
	out TCCR0A, r16
	ldi r16, (1<<CS01)|(1<<CS00)
	out TCCR0B, r16
	ldi r16, (1<<TOV0)
	out TIFR0, r16

    ; Initialize registers and ports here
    ldi R16, 0 ; Initially, load index for character '0'
    add XL, R16 ; Move to the correct position in the table
    ld R16, X ; Load the character pattern into R16

	clr R18
	clr enc_step
	rcall enc_read_state
	mov enc_prev, enc_curr

    rjmp main ; jump to main loop

.exit