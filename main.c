#include "stm32l476xx.h"
#include "SysClock.h"
#include "LED.h"
#include "UART.h"
#include "keypad.h"
#include "OLED_I2C.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"

#include <stdint.h>
#include <stddef.h>

extern void waitms(unsigned int ms);

void execute_key(char key) {
    switch (key) {
        case '1':
            LED_On();
            waitms(100);
            LED_Off();
            break;
        case '2':
            LED_On();
            waitms(300);
            LED_Off();
            break;
        case '3':
            LED_On();
            waitms(500);
            LED_Off();
            break;
        default:
            LED_On();
            waitms(150);
            LED_Off();
            break;
    }
}

/* Draws the full buffer text on the OLED screen */
static void OLED_ShowBuffer(const char *text)
{
    const uint8_t chars_per_line = 18U;
    const uint8_t line_height = 10U;
    const uint8_t max_lines = 6U;
    uint8_t line = 0U;
    uint8_t col = 0U;
    size_t i = 0U;
    char one_char[2];

    ssd1306_Fill(Black);

    /* Header */
    ssd1306_SetCursor(0U, 0U);
    ssd1306_WriteString("Hello!!", Font_11x18, White);

    line = 2U;
    col = 0U;
    ssd1306_SetCursor(0U, (uint8_t)(line * line_height));

    while ((text[i] != '\0') && (line < max_lines)) {
        if (col >= chars_per_line) {
            line++;
            col = 0U;

            if (line >= max_lines) {
                break;
            }

            ssd1306_SetCursor(0U, (uint8_t)(line * line_height));
        }

        one_char[0] = text[i];
        one_char[1] = '\0';
        ssd1306_WriteString(one_char, Font_7x10, White);

        col++;
        i++;
    }

    ssd1306_UpdateScreen();
}

/* Displays current replaying key in large font on the OLED screen */
static void OLED_ShowReplayStep(char key, uint32_t current_step, uint32_t total_steps)
{
    char active_str[2] = { key, '\0' };

    ssd1306_Fill(Black);
    
    // Header
    ssd1306_SetCursor(0U, 0U);
    ssd1306_WriteString("REPLAYING...", Font_7x10, White);

    // Active Key Display
    ssd1306_SetCursor(40U, 20U);
    ssd1306_WriteString(active_str, Font_16x26, White);

    ssd1306_UpdateScreen();
}

int main(void)
{
    char chr;
    char str[64] = {0};
    uint32_t n = 0U;

    System_Clock_Init();
    LED_Init();
    UART2_Init();

    Keypad_Pin_Init();

    OLED_I2C_Init();
    ssd1306_Init();

    if (OLED_I2C_GetLastStatus() != OLED_I2C_OK) {
        LED_On();
        while (1) { ; }
    }

    OLED_ShowBuffer(str);

    while (1) {
        chr = (char)keypad_scan();

        // '*' = Delete latest entry
        if (chr == '*') {
            if (n > 0U) {
                n--;
                str[n] = '\0';
            }

            OLED_ShowBuffer(str);
            USART_Write(USART2, (uint8_t *)str, n);
            LED_Toggle();
        }

        // '#' = Replay sequence on OLED and LEDs
        else if (chr == '#') {
            USART_Write(USART2, (uint8_t *)str, n);

            // Loop through each step in the stored sequence
            for (uint32_t i = 0; i < n; i++) {
                // 1. Update OLED with current key step
                OLED_ShowReplayStep(str[i], i + 1, n);

                // 2. Execute physical hardware output
                execute_key(str[i]);

                // 3. Pause so each key remains visible on OLED
                waitms(400);
            }

            // Restore complete string display when replay finishes
            OLED_ShowBuffer(str);
        }

        // Normal key pressed
        else if ((unsigned char)chr != 0xFFU) {

            if (n < (sizeof(str) - 1U)) {
                str[n] = chr;
                n++;
                str[n] = '\0';
            }

            execute_key(chr);

            OLED_ShowBuffer(str);
            USART_Write(USART2, (uint8_t *)str, n);
            LED_Toggle();
        }
    }
}