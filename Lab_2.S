.include "m328Pdef.inc"
.cseg
.org 0

rjmp start ; jump to the start label

; Lookup table for 7-segment display characters (0-9, A-F)
seg_hex_table:
    ;   0    1    2    3    4    5    6    7
    .db 0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07
    ;   8    9    A    B    C    D    E    F
    .db 0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71

; Shift byte to register and generate pulses for SRCLK and RCLK
display:
    ; backup used registers on stack
    push R16
    push R17
    in R17, SREG
    push R17

    ldi R17, 8 ; send 8 bits serially to shift register

loop:
    rol R16 ; rotate left through Carry
    BRCS set_ser_in_1 ; branch if Carry is set

    ; put code here to set SER to 0
    cbi PORTB, 0 ; Clear SER pin (assuming SER is connected to PB0)
    
    rjmp end

set_ser_in_1:
    ; put code here to set SER to 1
    sbi PORTB, 0 ; Set SER pin (assuming SER is connected to PB0)

end:
    ; put code here to generate SRCLK pulse
    
    dec R17
    brne loop ; branch to loop if R17 is not zero

    ; put code here to generate RCLK pulse
    cbi PORTB, 1 ; Clear RCLK pin (assuming RCLK is connected to PB1)
    sbi PORTB, 1 ; Set RCLK pin (assuming RCLK is connected to PB1)
    
    ; restore registers from stack
    pop R17
    out SREG, R17
    pop R17
    pop R16
    ret

input:
    ; Implement input function to read user input (e.g., from buttons or serial)

    ; TODO: implement logic to modify R16 based on user input (e.g., increment/decrement character index)

    ldi R16, 0 ; Initially, load index for character '0'
    add ZL, R16 ; Move to the correct position in the table
    ld R16, Z ; Load the character pattern into R16

    ret

main:
    rcall input ; Call the input function to get user input
    rcall display ; Call the display function to show the character
    rjmp main ; jump to main loop

; Start of main program (initialize registers, call display function, etc.)
start:
    ; Initialize registers and ports here
    ldi ZH, high(seg_hex_table)
    ldi ZL, low(seg_hex_table)
    ldi R16, 0 ; Initially, load index for character '0'
    add ZL, R16 ; Move to the correct position in the table
    ld R16, Z ; Load the character pattern into R16

    rjmp main ; jump to main loop