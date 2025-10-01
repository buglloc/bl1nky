#pragma once

#include <stdint.h>
#include <string.h>

#include "board.h"

/* Buffers: must be even-addressed for DMA */
__xdata __at (0x0000) uint8_t usb_ep0_buf[DEFAULT_ENDP0_SIZE];       // EP0 OUT/IN buffer
__xdata __at (0x0040) uint8_t usb_ep1_buf[DEFAULT_ENDP1_SIZE];       // EP1 IN buffer (interrupt)

/* HID class requests */
#define HID_GET_REPORT          0x01
#define HID_GET_IDLE            0x02
#define HID_GET_PROTOCOL        0x03
#define HID_SET_REPORT          0x09
#define HID_SET_IDLE            0x0A
#define HID_SET_PROTOCOL        0x0B

/* HID descriptor types */
#define HID_DESCRIPTOR_TYPE_HID         0x21
#define HID_DESCRIPTOR_TYPE_REPORT      0x22
#define HID_DESCRIPTOR_TYPE_PHYSICAL    0x23

void usb_hid_init(void);
uint8_t usb_hid_mounted(void);
void usb_hid_interrupt(void);

/* Check if a new command has been received from host */
uint8_t usb_hid_has_command(void);

/* Get the received command and data (call after usb_hid_has_command() returns true) */
void usb_hid_get_command(uint8_t *cmd, uint8_t *data);

/* Send response data back to host (for GET_REPORT) */
void usb_hid_send_response(uint8_t cmd, uint8_t data);