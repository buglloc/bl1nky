package cmd

import (
	"fmt"
	"os"

	"github.com/buglloc/bl1nky"
	"github.com/spf13/cobra"
)

var blinker bl1nky.Blinker

var rootArgs struct {
	serial string
}

var rootCmd = &cobra.Command{
	Use:          "bl1nky",
	Short:        "YaCTF2025 badge LED controller",
	SilenceUsage: true,
	PersistentPreRunE: func(_ *cobra.Command, _ []string) error {
		var err error
		blinker, err = bl1nky.NewHIDBl1nky(
			bl1nky.WithDeviceSerial(rootArgs.serial),
		)
		if err != nil {
			return fmt.Errorf("create bl1nky: %w", err)
		}

		return nil
	},
}

func Execute() {
	err := rootCmd.Execute()
	if err != nil {
		os.Exit(1)
	}
}

func init() {
	flags := rootCmd.PersistentFlags()
	flags.StringVar(&rootArgs.serial, "serial", "", "Serial number of the bl1nky")

	rootCmd.AddCommand(
		getCmd,
		setCmd,
		patternCmd,
	)
}
