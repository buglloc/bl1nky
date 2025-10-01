package bl1nky

import (
	"fmt"
	"strings"
)

type LedSet byte

const (
	Led1 = 1 << (3 - iota)
	Led2
	Led3
	Led4
)

func (l LedSet) String() string {
	states := make([]string, 4)
	for i := 0; i < 4; i++ {
		if l&(1<<(3-i)) != 0 {
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
