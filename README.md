# bl1nky

A LED controller for the YaCTF2025 badge, featuring a Go library, CLI tool, and CH552 firmware.

## Overview

`bl1nky` provides USB HID-based control for the 4 LEDs on the YaCTF2025 badge. The project includes:

- **Go Library**: Full-featured library for LED control via USB HID
- **CLI Tool**: Command-line interface for LED operations and pattern animations
- **Firmware**: CH552 microcontroller firmware for the YaCTF2025 badge

## Installation

### CLI Tool

```bash
go install github.com/buglloc/bl1nky/cmd/bl1nky@latest
```

Or build from source:

```bash
cd cmd/bl1nky
go build
```

### Go Library

```bash
go get github.com/buglloc/bl1nky
```

### Linux udev Rules

For non-root access on Linux, install the udev rules:

```bash
sudo cp udev/70-bl1nky.rules /etc/udev/rules.d/
sudo udevadm control --reload-rules
sudo udevadm trigger
```

## CLI Usage

The `bl1nky` CLI provides three main commands: `get`, `set`, and `pattern`.

### Get LED State

Get the current state of all LEDs:

```bash
# Get current LED state
bl1nky get

# Get current LED state in binary format
bl1nky get --binary
```

The LED state is represented in left-to-right order: `0b[LED1][LED2][LED3][LED4]`
- `1` = LED is on
- `0` = LED is off

### Set LED State

Set the state of all LEDs using binary notation:

```bash
# Turn on all LEDs
bl1nky set --state 0b1111

# Turn on only LED 1 and LED 3
bl1nky set --state 0b1010

# Turn off all LEDs
bl1nky set --state 0b0000

# Turn on only LED 1 and LED 4 (0b prefix is optional)
bl1nky set --state 1001
```

### Pattern Animations

Execute LED patterns with timing control:

```bash
# Use predefined patterns
bl1nky pattern blink
bl1nky pattern wave
bl1nky pattern chase
bl1nky pattern pulse
bl1nky pattern bounce
bl1nky pattern loop

# Use custom pattern file
bl1nky pattern mypattern.txt

# Use pattern from stdin
echo -e "set 0b1111\nwait 500ms\nset 0b0000\nwait 500ms" | bl1nky pattern
```

### Pattern Language

Pattern files support the following commands:

- **Set LED state**: `set 0b1011` (turns on LEDs 1, 2, and 4)
- **Wait commands**: `wait 100ms`
- **Repeat blocks**: `repeat 3 ... end` (repeats commands 3 times)
- **Comments**: Lines starting with `#` are ignored

Example pattern file:

```txt
# Blink pattern
repeat 5
    set 0b1111
    wait 500ms
    set 0b0000
    wait 500ms
end
```

## Go Library Usage

### Basic Example

```go
package main

import (
    "fmt"
    "log"
    "time"
    
    "github.com/buglloc/bl1nky"
)

func main() {
    // Create a new blinker instance
    blinker, err := bl1nky.NewHIDBl1nky()
    if err != nil {
        log.Fatalf("Failed to create blinker: %v", err)
    }
    
    // Open connection to the device
    if err := blinker.Open(); err != nil {
        log.Fatalf("Failed to open blinker: %v", err)
    }
    defer blinker.Close()
    
    // Turn on all LEDs
    if err := blinker.SetLEDs(bl1nky.Led1 | bl1nky.Led2 | bl1nky.Led3 | bl1nky.Led4); err != nil {
        log.Fatalf("Failed to set LEDs: %v", err)
    }
    time.Sleep(1 * time.Second)
    
    // Turn off all LEDs
    if err := blinker.SetLEDs(0); err != nil {
        log.Fatalf("Failed to set LEDs: %v", err)
    }
    
    // Get current LED state
    state, err := blinker.GetLEDs()
    if err != nil {
        log.Fatalf("Failed to get LEDs: %v", err)
    }
    fmt.Printf("Current LED state: %s\n", state)
}
```

## Firmware

The firmware is designed for the CH552 microcontroller used in the YaCTF2025 badge.

### Building Firmware

Requirements:
- SDCC (Small Device C Compiler)
- [WCH ISP Tool in Rust](https://github.com/ch32-rs/wchisp)

Build&&Flash the firmware:

```bash
cd firmware
make flash
```

### Firmware Features

- USB HID interface (VID: 0x1209, PID: 0xF600)
- Supports 4 LEDs on the YaCTF2025 badge
- Commands:
  - `0x01`: Set LED state
  - `0x02`: Get LED state
- No external dependencies beyond the CH552 SDK

### Hardware Configuration

The firmware controls 4 LEDs:
- LED 1: Controlled via bit 3 (leftmost)
- LED 2: Controlled via bit 2
- LED 3: Controlled via bit 1
- LED 4: Controlled via bit 0

## Technical Details

### USB HID Descriptors

- **Vendor ID**: 0x1209
- **Product ID**: 0xF600
- **Usage Page**: 0xFF (Vendor-defined)
- **Usage**: 0xCF

### Protocol

The device uses USB HID output reports for commands and feature reports for responses.

#### Set LED State

Send output report:
```
[0x01, LED_STATE]
```

Where `LED_STATE` is a byte with bits representing LED states (1 = on, 0 = off).

#### Get LED State

Send output report:
```
[0x02, 0x00]
```

Receive feature report:
```
[0x02, LED_STATE]
```

## Predefined Patterns

The CLI includes several built-in patterns:

- **blink**: Simple on/off blinking of all LEDs
- **loop**: Sequential LED loop
- **wave**: Smooth wave effect across LEDs
- **bounce**: Bouncing LED effect
- **chase**: Chasing LED pattern
- **pulse**: Pulsing effect

## License

This project is licensed under the GNU Lesser General Public License v2.1. See the [LICENSE](LICENSE) file for details.

## Contributing

Contributions are welcome! Feel free to open issues or submit pull requests.

## Troubleshooting

### Device Not Found

1. Check if the device is connected: `lsusb | grep 1209:f600`
2. Ensure udev rules are installed (Linux)
3. Try running with `sudo` (if udev rules aren't set up)

### Permission Denied

On Linux, install the udev rules (see Installation section) and replug the device.

### Multiple Devices

If you have multiple bl1nky devices, use the `--serial` flag to specify which device to control.

## See Also

- [CH552 Datasheet](firmware/docs/ch552ds1.pdf)

