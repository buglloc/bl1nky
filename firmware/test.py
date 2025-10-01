#!/usr/bin/env python3
"""
Simple test script for the USB HID LED controller.
Requires: pip install hidapi
"""

import hid
import time
import sys

# USB VID/PID from the descriptors
VENDOR_ID = 0x2341
PRODUCT_ID = 0x8037

# Commands
CMD_SET_LED_STATE = 0x01
CMD_GET_LED_STATE = 0x02

def find_device():
    """Find the HID LED device."""
    devices = hid.enumerate(VENDOR_ID, PRODUCT_ID)
    if not devices:
        print(f"Device not found (VID: 0x{VENDOR_ID:04X}, PID: 0x{PRODUCT_ID:04X})")
        print("\nAvailable HID devices:")
        for dev in hid.enumerate():
            print(f"  VID: 0x{dev['vendor_id']:04X}, PID: 0x{dev['product_id']:04X} - {dev['product_string']}")
        return None
    
    print(f"Found device: {devices[0]['product_string']}")
    return devices[0]['path']

def set_leds(device, led_state):
    """
    Set LED state.
    led_state: 8-bit value where each bit controls an LED
               Bit 0 = LED 0 (led_c)
               Bit 1 = LED 1 (led_t)
               Bit 2 = LED 2 (led_f)
               Bits 3-7 = unused
    """
    # HID Output Report: 2 bytes [command, data]
    device.write([0x00, CMD_SET_LED_STATE, led_state & 0xFF])  # Report ID (0) + command + data

def get_leds(device):
    """Get current LED state from device."""
    try:
        # First, send GET_LED_STATE command
        device.write([0x00, CMD_GET_LED_STATE, 0x00])  # Report ID (0) + command + dummy data
        
        # Now request the Input Report
        data = device.get_feature_report(0, 3)  # Report ID 0, 3 bytes (ID + command + data)
        if data and len(data) >= 3:
            # data[0] is the Report ID, data[1] is the command, data[2] is the LED state
            if data[1] == CMD_GET_LED_STATE:
                return data[2] & 0xFF
    except Exception as e:
        print(f"Failed to get LED state: {e}")
    return None

def main():
    path = find_device()
    if not path:
        sys.exit(1)
    
    try:
        device = hid.device()
        device.open_path(path)
        print("Device opened successfully!\n")
        
        # Test pattern: cycle through all LEDs
        patterns = [
            (0b0000, "All LEDs OFF"),
            (0b0001, "LED 0 ON"),
            (0b0010, "LED 1 ON"),
            (0b0100, "LED 2 ON"),
            (0b0111, "All LEDs ON"),
            (0b0101, "LED 0 and 2 ON"),
            (0b0011, "LED 0 and 1 ON"),
            (0b0110, "LED 1 and 2 ON"),
        ]
        
        print("Testing LED patterns...")
        for pattern, description in patterns:
            print(f"Setting: {description} (0b{pattern:04b})")
            set_leds(device, pattern)
            time.sleep(0.5)
            
            # Read back current state
            current = get_leds(device)
            if current is not None:
                print(f"  Current state: 0b{current:04b}\n")
        
        # Turn off all LEDs at the end
        print("Turning off all LEDs...")
        set_leds(device, 0b0000)
        
        device.close()
        print("Done!")
        
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()