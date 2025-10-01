package bl1nky

var _ Bl1nky = (*NopBl1nky)(nil)

type NopBl1nky struct{}

func NewNopBl1nky() *NopBl1nky {
	return &NopBl1nky{}
}

func (*NopBl1nky) SetLEDs(LedSet) error {
	return nil
}

func (*NopBl1nky) GetLEDs() (LedSet, error) {
	return 0, nil
}

func (*NopBl1nky) Open() error {
	return nil
}

func (*NopBl1nky) Close() error {
	return nil
}
