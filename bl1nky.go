package bl1nky

import (
	"fmt"
	"strings"
)

type LedSet byte

const (
	LedC = 1 << (3 - iota)
	LedT
	LedF
)

func (l LedSet) String() string {
	states := make([]string, 3)
	for i := 0; i < 3; i++ {
		if l&(1<<(2-i)) != 0 {
			states[i] = "on"
		} else {
			states[i] = "off"
		}
	}

	return fmt.Sprintf("LedSet{%s}", strings.Join(states, ", "))
}

type Blinker interface {
	Open() error
	Close() error
	SetLEDs(LedSet) error
	GetLEDs() (LedSet, error)
}
