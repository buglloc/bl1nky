package cmd

import (
	"fmt"
	"io"
	"os"

	"github.com/spf13/cobra"

	"github.com/buglloc/bl1nky/cmd/bl1nky/pattern"
	"github.com/buglloc/bl1nky/cmd/bl1nky/patterns"
)

var patternArgs struct {
	quiet bool
}

var patternCmd = &cobra.Command{
	Use:   "pattern [target]",
	Short: "Perform LED pattern animations",
	Long: `Perform LED pattern animations.
Each line can contain:
  - Set LED state: set 0b101 (turns on LEDs C and F)
  - Wait command: wait 100ms, wait 1s
  - Repeat block: repeat 3 ... end (repeats commands 3 times)

The target argument can be:
  - "-" for stdin (default if no argument provided)
  - A path to a pattern file
  - A predefined pattern name (blink, loop, wave, bounce, chase, pulse, binary)

Examples:
  bl1nky pattern blink          # Use predefined pattern
  bl1nky pattern loop           # Use predefined pattern
  bl1nky pattern pattern.txt    # Use custom file
  echo -e "set 0b111\nwait 500ms" | bl1nky pattern
  echo -e "set 0b101\nwait 500ms" | bl1nky pattern -`,
	RunE: func(_ *cobra.Command, args []string) error {
		if err := blinker.Open(); err != nil {
			return fmt.Errorf("open blinker: %w", err)
		}
		defer func() { _ = blinker.Close() }()

		patternName := "-"
		if len(args) > 0 {
			patternName = args[0]
		}

		reader, closer, err := choosePattern(patternName)
		if err != nil {
			return fmt.Errorf("choose pattern: %w", err)
		}

		if closer != nil {
			defer func() { _ = closer() }()
		}

		opts := []pattern.ExecutorOption{
			pattern.WithBlinker(blinker),
		}

		if !patternArgs.quiet {
			opts = append(opts, pattern.WithTracer(func(line int, cmd pattern.Command) {
				fmt.Printf("#%d %s\n", line, cmd)
			}))
		}

		executor := pattern.NewExecutor(opts...)
		return executor.Execute(reader)
	},
}

func choosePattern(in string) (io.Reader, func() error, error) {
	if in == "-" {
		return os.Stdin, nil, nil
	}

	if _, err := os.Stat(in); err == nil {
		f, err := os.Open(in)
		if err != nil {
			return nil, nil, fmt.Errorf("open pattern file: %w", err)
		}

		return f, f.Close, nil
	}

	if f, err := patterns.Open(in); err == nil {
		return f, f.Close, nil
	}

	return nil, nil, fmt.Errorf("pattern not found: %q (not a file, and no embedded pattern)", in)
}

func init() {
	patternCmd.Flags().BoolVar(&patternArgs.quiet, "quiet", false, "disable command logging")
}
