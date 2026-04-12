#include "lcd.h"

// ─── Low-level helpers ────────────────────────────────────────────────────────

static void lcd_pulse_enable(void) {
    // The E pin must be pulsed high then low to latch data.
    // Datasheet specifies PWEH (enable pulse width) >= 450ns at 3V,
    // >= 230ns at 5V. _delay_us(1) is safe at both voltages.
    LCD_E_PORT |=  (1 << LCD_E_PIN);
    _delay_us(1);
    LCD_E_PORT &= ~(1 << LCD_E_PIN);
    _delay_us(1);
}

static void lcd_write_nibble(uint8_t nibble) {
    // Write the upper 4 bits of 'nibble' to DB4–DB7.
    // First, clear the data pins, then set them from the nibble.
    LCD_DATA_PORT &= ~((1 << LCD_D4_PIN) | (1 << LCD_D5_PIN) |
                       (1 << LCD_D6_PIN) | (1 << LCD_D7_PIN));

    if (nibble & 0x10) LCD_DATA_PORT |= (1 << LCD_D4_PIN);
    if (nibble & 0x20) LCD_DATA_PORT |= (1 << LCD_D5_PIN);
    if (nibble & 0x40) LCD_DATA_PORT |= (1 << LCD_D6_PIN);
    if (nibble & 0x80) LCD_DATA_PORT |= (1 << LCD_D7_PIN);

    lcd_pulse_enable();
}

static void lcd_send_byte(uint8_t byte, uint8_t is_data) {
    // RS = 1 for character data, RS = 0 for commands (Table 1 in datasheet)
    if (is_data)
        LCD_RS_PORT |=  (1 << LCD_RS_PIN);
    else
        LCD_RS_PORT &= ~(1 << LCD_RS_PIN);

    // Per datasheet section on 4-bit mode: send high nibble first, then low.
    lcd_write_nibble(byte & 0xF0);        // High nibble
    lcd_write_nibble((byte << 4) & 0xF0); // Low nibble

    // Most instructions take 37µs to execute; CLEAR and HOME take up to 1.52ms.
    // We use a safe 50µs delay here and handle the slow commands separately.
    _delay_us(50);
}

// ─── Public API ───────────────────────────────────────────────────────────────

void lcd_send_command(uint8_t cmd) {
    lcd_send_byte(cmd, 0);

    // Clear display and Return home are slow — datasheet says up to 1.52ms
    if (cmd == LCD_CLEAR_DISPLAY || cmd == LCD_RETURN_HOME)
        _delay_ms(2);
}

void lcd_send_data(uint8_t data) {
    lcd_send_byte(data, 1);
}

void lcd_clear(void) {
    lcd_send_command(LCD_CLEAR_DISPLAY);
}

void lcd_set_cursor(uint8_t row, uint8_t col) {
    // DDRAM addresses: row 0 starts at 0x00, row 1 starts at 0x40
    // (from the datasheet DDRAM address map, Figures 4–5)
    uint8_t row_offsets[] = {0x00, 0x40};
    lcd_send_command(LCD_SET_DDRAM_ADDR | (row_offsets[row] + col));
}

void lcd_print(const char *str) {
    while (*str)
        lcd_send_data((uint8_t)(*str++));
}

void lcd_init(void) {
    // Set control and data pins as outputs
    LCD_CTRL_DDR |=  (1 << LCD_RS_PIN) | (1 << LCD_E_PIN);
    LCD_DATA_DDR |=  (1 << LCD_D4_PIN) | (1 << LCD_D5_PIN) |
                     (1 << LCD_D6_PIN) | (1 << LCD_D7_PIN);

    // Ensure all pins start low
    LCD_RS_PORT  &= ~(1 << LCD_RS_PIN);
    LCD_E_PORT   &= ~(1 << LCD_E_PIN);
    LCD_DATA_PORT &= ~((1 << LCD_D4_PIN) | (1 << LCD_D5_PIN) |
                       (1 << LCD_D6_PIN) | (1 << LCD_D7_PIN));

    // ── Initialization sequence (Figure 24 in datasheet) ──────────────────
    // Wait > 40ms after VCC rises to 2.7V before sending any commands.
    _delay_ms(50);

    // Step 1: Send 0x30 three times as 8-bit writes (the LCD starts in 8-bit
    // mode after power-on; these writes coax it into a known state).
    // RS=0, send high nibble only each time.
    LCD_RS_PORT &= ~(1 << LCD_RS_PIN);

    lcd_write_nibble(0x30); // 8-bit code (0011 0000); D3-D0 = 0000
    _delay_ms(5);    // Wait > 4.1ms (datasheet requirement)

    lcd_write_nibble(0x30);
    _delay_us(150);  // Wait > 100µs

    lcd_write_nibble(0x30);
    _delay_us(50);

    // Step 2: Now switch to 4-bit mode by sending 0x20
    lcd_write_nibble(0x20);
    _delay_us(50);

    // ── From here we can send full bytes as two nibbles ────────────────────

    // Function Set: 4-bit, 2 lines, 5×8 dots  (N=1, F=0, DL=0)
    lcd_send_command(LCD_FUNCTION_SET_4BIT);

    // Display Off (required step in init sequence)
    lcd_send_command(0x08);

    // Clear Display
    lcd_send_command(LCD_CLEAR_DISPLAY);

    // Entry Mode Set: increment cursor, no display shift  (I/D=1, S=0)
    lcd_send_command(LCD_ENTRY_MODE);

    // Display On, cursor off, blink off  (D=1, C=0, B=0)
    lcd_send_command(LCD_DISPLAY_ON);
}