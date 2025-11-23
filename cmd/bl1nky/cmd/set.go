package cmd

import (
	"fmt"
	"strconv"
	"strings"

	"github.com/spf13/cobra"

	"github.com/buglloc/bl1nky"
)

var setArgs = struct {
	state string
}{
	state: "0b111",
}

var setCmd = &cobra.Command{
	Use:   "set",
	Short: "Set LEDs state",
	Long: `Set the state of all LEDs on the bl1nky device using binary notation.

The state is represented as a binary number in left-to-right order: 0b[LED_C][LED_T][LED_F]
  - First bit (leftmost): LED C
  - Second bit: LED T
  - Third bit: LED F

A bit value of 1 turns the LED on, and 0 turns it off.

Examples:
  # Turn on all LEDs
  bl1nky set --state 0b111

  # Turn on only LED C and LED F
  bl1nky set --state 0b101

  # Turn off all LEDs
  bl1nky set --state 0b0000

  # Turn on only LED C and LED T
  bl1nky set --state 0b110

Note: The '0b' prefix is optional. You can also use just '111' or '101'.`,
	RunE: func(_ *cobra.Command, _ []string) error {
		if err := blinker.Open(); err != nil {
			return fmt.Errorf("open blinker: %w", err)
		}
		defer func() { _ = blinker.Close() }()

		if setArgs.state == "" {
			return fmt.Errorf("state is required")
		}

		stateStr := strings.TrimPrefix(strings.ToLower(setArgs.state), "0b")

		state, err := strconv.ParseUint(stateStr, 2, 8)
		if err != nil {
			return fmt.Errorf("parse state: %w", err)
		}

		if err := blinker.SetLEDs(bl1nky.LedSet(state)); err != nil {
			return fmt.Errorf("set LEDs: %w", err)
		}

		return nil
	},
}

func init() {
	flags := setCmd.Flags()
	flags.StringVar(&setArgs.state, "state", setArgs.state, "LEDs state in binary form, for example 0b101 to turn on first and third LED")
}
