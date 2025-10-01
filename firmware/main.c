#include <stdint.h>

#include "config.h"
#include "board.h"
#include "leds.h"
#include "usb_hid.h"

/* Command definitions */
#define CMD_SET_LED_STATE  0x01
#define CMD_GET_LED_STATE  0x02

void usb_hid_interrupt(void);
void usb_isr(void) __interrupt(INT_NO_USB) {
  usb_hid_interrupt();
}

int main(void)
{
  uint8_t led_state = 0;
  uint8_t cmd, data;

  board_init();
  leds_init();
  leds_off();
  usb_hid_init();

  while (1) {
    if (!usb_hid_mounted()) {
      continue;
    }

    // Check for new commands from host
    if (usb_hid_has_command()) {
      usb_hid_get_command(&cmd, &data);

      switch (cmd) {
        case CMD_SET_LED_STATE:
          // Host wants to set LED state
          led_state = data;

          // Map each bit to an LED
          // Bit 0 -> LED 0
          // Bit 1 -> LED 1
          // Bit 2 -> LED 2
          // Bit 3 -> unused (only 3 LEDs available)
          leds_set(0, led_state & 0x01);
          leds_set(1, (led_state >> 1) & 0x01);
          leds_set(2, (led_state >> 2) & 0x01);
          break;

        case CMD_GET_LED_STATE:
          // Host requests current LED state - send response
          usb_hid_send_response(CMD_GET_LED_STATE, led_state);
          break;

        default:
          // ???
          break;
      }
    }
  }
}