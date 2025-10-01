package pattern

import (
	"bufio"
	"fmt"
	"io"
	"strings"
	"time"

	"github.com/buglloc/bl1nky"
)

type execCommand struct {
	Command
	lineNum int
}

func Execute(blinker bl1nky.Blinker, reader io.Reader) error {
	scanner := bufio.NewScanner(reader)
	lineNum := 0

	var commands []execCommand
	for scanner.Scan() {
		lineNum++
		line := strings.TrimSpace(scanner.Text())

		// Skip empty lines and comments
		if line == "" || strings.HasPrefix(line, "#") {
			continue
		}

		cmd, err := ParseCommand(line)
		if err != nil {
			return fmt.Errorf("line %d: %w", lineNum, err)
		}

		commands = append(commands, execCommand{
			Command: cmd,
			lineNum: lineNum,
		})
	}

	if err := scanner.Err(); err != nil {
		return fmt.Errorf("read input: %w", err)
	}

	return execute(blinker, commands)
}

func execute(blinker bl1nky.Blinker, commands []execCommand) error {
	for i := 0; i < len(commands); i++ {
		parsed := commands[i]

		switch c := parsed.Command.(type) {
		case *SetCommand:
			if err := blinker.SetLEDs(c.State); err != nil {
				return fmt.Errorf("line %d: set LEDs: %w", parsed.lineNum, err)
			}

		case *WaitCommand:
			time.Sleep(c.Duration)

		case *RepeatCommand:
			endIdx := findMatchingEnd(commands, i)
			if endIdx == -1 {
				return fmt.Errorf("line %d: repeat without matching 'end'", parsed.lineNum)
			}

			repeatBlock := commands[i+1 : endIdx]
			for j := 0; j < c.Count; j++ {
				if err := execute(blinker, repeatBlock); err != nil {
					return err
				}
			}

			// Skip to after the end command
			i = endIdx

		case *EndCommand:
			return fmt.Errorf("line %d: 'end' without matching 'repeat'", parsed.lineNum)

		default:
			return fmt.Errorf("line %d: unknown command: %T", parsed.lineNum, c)
		}
	}

	return nil
}

func findMatchingEnd(commands []execCommand, startIdx int) int {
	depth := 1
	for i := startIdx + 1; i < len(commands); i++ {
		switch commands[i].Command.(type) {
		case *RepeatCommand:
			depth++

		case *EndCommand:
			depth--
			if depth == 0 {
				return i
			}

		}
	}

	return -1
}
