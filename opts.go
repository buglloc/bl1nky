package bl1nky

type Option interface {
	isOption()
}

type optDevice struct {
	Option
	dev *HIDDevice
}

func WithDevice(dev *HIDDevice) Option {
	return optDevice{
		dev: dev,
	}
}

type optDeviceSerial struct {
	Option
	serial string
}

func WithDeviceSerial(serial string) Option {
	return optDeviceSerial{
		serial: serial,
	}
}
