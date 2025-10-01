package bl1nky

type LedSet byte

const (
	Led0 = 1 << iota
	Led1
	Led2
	Led3
)

type Bl1nky interface {
	SetLEDs(LedSet) error
	GetLEDs() (LedSet, error)
}
