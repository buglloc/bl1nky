package bl1nky

import (
	"fmt"

	"github.com/buglloc/usbhid"
)

const (
	cmdSetLEDState = 0x01
	cmdGetLEDState = 0x02
)

var _ Bl1nky = (*HIDBl1nky)(nil)

type HIDBl1nky struct {
	dev *HIDDevice
}

func NewHIDBl1nky(opts ...Option) (*HIDBl1nky, error) {
	h := &HIDBl1nky{}
	for _, opt := range opts {
		switch v := opt.(type) {
		case optDevice:
			h.dev = v.dev

		case optDeviceSerial:
			devs, err := HIDEnumerate(func(d *usbhid.Device) bool {
				return d.SerialNumber() == v.serial
			})
			if err != nil {
				return nil, fmt.Errorf("device enumeration: %w", err)
			}

			if len(devs) == 0 {
				return nil, fmt.Errorf("device with serial %q not found", v.serial)
			}

			if len(devs) > 1 {
				return nil, fmt.Errorf("more than one device with serial %q was found", v.serial)
			}

			h.dev = devs[0]

		default:
			return nil, fmt.Errorf("unsupported option: %T", opt)
		}
	}

	if h.dev == nil {
		dev, err := FirstHIDDevice()
		if err != nil {
			return nil, fmt.Errorf("unable to find device: %w", err)
		}
		h.dev = dev
	}

	return h, nil
}

func (h *HIDBl1nky) Open() error {
	return h.dev.Open()
}

func (h *HIDBl1nky) Close() error {
	return h.dev.Close()
}

func (h *HIDBl1nky) Location() string {
	return h.dev.Location()
}

func (h *HIDBl1nky) SetLEDs(leds LedSet) error {
	return h.dev.SetOutputReport([]byte{
		cmdSetLEDState,
		byte(leds)},
	)
}

func (h *HIDBl1nky) GetLEDs() (LedSet, error) {
	err := h.dev.SetOutputReport([]byte{
		cmdGetLEDState,
		0x00,
	})
	if err != nil {
		return 0, fmt.Errorf("send get command: %w", err)
	}

	data, err := h.dev.GetFeatureReport()
	if err != nil {
		return 0, fmt.Errorf("get report: %w", err)
	}

	if len(data) < 2 {
		return 0, fmt.Errorf("invalid response length: %d (expected at least 2)", len(data))
	}

	if data[0] != cmdGetLEDState {
		return 0, fmt.Errorf("unexpected command in response: 0x%02X (expected 0x%02X)", data[0], cmdGetLEDState)
	}

	return LedSet(data[1]), nil
}
