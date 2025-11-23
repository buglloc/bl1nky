package cmd

import (
	"fmt"

	"github.com/spf13/cobra"
)

var getArgs struct {
	binary bool
}

var getCmd = &cobra.Command{
	Use:   "get",
	Short: "Get LEDs state",
	Long: `Get the current state of all LEDs on the bl1nky device.

By default, the command outputs the LED state as a decimal number (0-15).
Use the --binary flag to display the state in binary notation (0bXXXX).

The state is represented in left-to-right order: 0b[LED_C][LED_T][LED_F]
  - First bit (leftmost): LED C
  - Second bit: LED T
  - Third bit: LED F

A bit value of 1 means the LED is on, and 0 means it's off.

Examples:
  # Get current LED state (decimal output)
  bl1nky get
  # Output: 15 (means all LEDs are on)

  # Get current LED state in binary format
  bl1nky get --binary
  # Output: 0b111 (means all LEDs are on)

  # Binary output examples:
  # 0b101 = LED C and LED F are on
  # 0b110 = LED C and LED T are on
  # 0b000 = All LEDs are off`,
	RunE: func(_ *cobra.Command, _ []string) error {
		if err := blinker.Open(); err != nil {
			return fmt.Errorf("open blinker: %w", err)
		}
		defer func() { _ = blinker.Close() }()

		state, err := blinker.GetLEDs()
		if err != nil {
			return fmt.Errorf("get LEDs: %w", err)
		}

		if getArgs.binary {
			fmt.Printf("0b%03b\n", state)
			return nil
		}

		fmt.Println(state)
		return nil
	},
}

func init() {
	flags := getCmd.Flags()
	flags.BoolVar(&getArgs.binary, "binary", false, "Output in binary form")
}
