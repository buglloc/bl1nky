#include "leds.h"

#define LED0_BIT 0x00
#define LED1_BIT 0x01
#define LED2_BIT 0x05
#define LED_COUNT 3

// P3.0
SBIT(led_0, 0xB0, LED0_BIT);
// P3.1
SBIT(led_1, 0xB0, LED1_BIT);
// P1.5
SBIT(led_2, 0x90, LED2_BIT);


void _set_led(uint8_t idx, uint8_t value)
{
  switch (idx) {
    case 0:
      led_0 = value;
      break;

    case 1:
      led_1 = value;
      break;

    case 2:
      led_2 = value;
      break;
  }
}

void leds_init(void)
{
  P3_MOD_OC &= ~(1 << LED0_BIT); // push-pull
  P3_DIR_PU |= (1 << LED0_BIT);  // output

  P3_MOD_OC &= ~(1 << LED1_BIT);
  P3_DIR_PU |= (1 << LED1_BIT);

  P1_MOD_OC &= ~(1 << LED2_BIT);
  P1_DIR_PU |= (1 << LED2_BIT);
}

void leds_set(uint8_t idx, uint8_t value)
{
  if (idx >= LED_COUNT) {
    return;
  }

  // LEDs are active low (0 = on, 1 = off)
  _set_led(idx, value ? 0 : 1);
}

void leds_on(void)
{
  for (uint8_t i = 0; i < LED_COUNT; ++i) {
    _set_led(i, 0);
  }
}

void leds_off(void)
{
  for (uint8_t i = 0; i < LED_COUNT; ++i) {
    _set_led(i, 1);
  }
}
