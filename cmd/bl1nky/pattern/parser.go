package pattern

import (
	"errors"
	"fmt"
	"strconv"
	"strings"
	"time"

	"github.com/buglloc/bl1nky"
)

type CommandType int

const (
	CommandUnknown CommandType = iota
	CommandSet
	CommandWait
	CommandRepeat
	CommandEnd
)

type Command interface {
	Type() CommandType
}

type SetCommand struct {
	State bl1nky.LedSet
}

func (c *SetCommand) Type() CommandType {
	return CommandSet
}

func (c *SetCommand) String() string {
	return fmt.Sprintf("set 0b%04b", c.State)
}

type WaitCommand struct {
	Duration time.Duration
}

func (c *WaitCommand) Type() CommandType {
	return CommandWait
}

func (c *WaitCommand) String() string {
	return fmt.Sprintf("wait %s", c.Duration)
}

type RepeatCommand struct {
	Count int
}

func (c *RepeatCommand) Type() CommandType {
	return CommandRepeat
}

func (c *RepeatCommand) String() string {
	return fmt.Sprintf("repeat %d", c.Count)
}

type EndCommand struct{}

func (c *EndCommand) Type() CommandType {
	return CommandEnd
}

func (c *EndCommand) String() string {
	return "end"
}

// ParseCommand parses a command line and returns the appropriate Command type
func ParseCommand(line string) (Command, error) {
	fields := strings.Fields(line)
	if len(fields) == 0 {
		return nil, errors.New("empty command")
	}

	switch strings.ToLower(fields[0]) {
	case "set":
		return parseSetCommand(fields)

	case "wait", "delay":
		return parseWaitCommand(fields)

	case "repeat":
		return parseRepeatCommand(fields)

	case "end":
		return &EndCommand{}, nil

	default:
		return nil, fmt.Errorf("unknown command: %s", fields[0])
	}
}

func parseSetCommand(fields []string) (Command, error) {
	if len(fields) != 2 {
		return nil, fmt.Errorf("invalid set command format, expected 'set <state>' (e.g., 'set 0b1011')")
	}

	stateStr := strings.TrimPrefix(strings.ToLower(fields[1]), "0b")

	state, err := strconv.ParseUint(stateStr, 2, 8)
	if err != nil {
		return nil, fmt.Errorf("parse LED state (expected binary form like 0b1011): %w", err)
	}

	return &SetCommand{State: bl1nky.LedSet(state)}, nil
}

func parseWaitCommand(fields []string) (Command, error) {
	if len(fields) != 2 {
		return nil, fmt.Errorf("invalid wait command format, expected 'wait <duration>' (e.g., 'wait 100ms')")
	}

	duration, err := time.ParseDuration(fields[1])
	if err != nil {
		return nil, fmt.Errorf("parse duration: %w", err)
	}

	return &WaitCommand{Duration: duration}, nil
}

func parseRepeatCommand(fields []string) (Command, error) {
	if len(fields) != 2 {
		return nil, fmt.Errorf("invalid repeat command format, expected 'repeat <count>' (e.g., 'repeat 3')")
	}

	count, err := strconv.Atoi(fields[1])
	if err != nil {
		return nil, fmt.Errorf("parse repeat count: %w", err)
	}

	if count < 1 {
		return nil, fmt.Errorf("repeat count must be at least 1, got %d", count)
	}

	return &RepeatCommand{Count: count}, nil
}
