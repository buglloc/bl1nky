#pragma once

#include <stdint.h>

#include <ch554.h>

void leds_init(void);
void leds_set(uint8_t idx, uint8_t value);
void leds_on(void);
void leds_off(void);
