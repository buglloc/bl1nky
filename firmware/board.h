#pragma once

#include <stdint.h>

#include <ch554.h>
#include <ch554_usb.h>

void delay_us(uint16_t n);
void delay_ms(uint16_t n);
void board_init(void);
void board_reset(void);
