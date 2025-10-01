package bl1nky

import (
	"errors"
	"fmt"

	"github.com/buglloc/usbhid"
)

const (
	VID          = 0x1209
	PID          = 0xF600
	HIDUsagePage = 0xFF
	HIDUsage     = 0xCF
)

type HIDDevice struct {
	dev *usbhid.Device
}

func FirstHIDDevice() (*HIDDevice, error) {
	devices, err := HIDEnumerate()
	if err != nil {
		return nil, err
	}

	if len(devices) == 0 {
		return nil, errors.New("not found")
	}

	return devices[0], nil
}

func HIDEnumerate(filters ...func(d *usbhid.Device) bool) ([]*HIDDevice, error) {
	devices, err := usbhid.Enumerate(
		usbhid.WithVidFilter(VID),
		usbhid.WithPidFilter(PID),
		usbhid.WithDeviceFilterFunc(func(d *usbhid.Device) bool {
			if d.UsagePage() != HIDUsagePage {
				return false
			}

			if d.Usage() != HIDUsage {
				return false
			}

			for _, filter := range filters {
				if !filter(d) {
					return false
				}
			}

			return true
		}),
	)
	if err != nil {
		return nil, fmt.Errorf("enumerate HID devices: %w", err)
	}

	var out []*HIDDevice
	for _, dev := range devices {
		out = append(out, &HIDDevice{
			dev: dev,
		})
	}

	return out, nil
}

func (d *HIDDevice) Path() string {
	return d.dev.Path()
}

func (d *HIDDevice) Open() error {
	return convertHIDErr(d.dev.Open(true))
}

func (d *HIDDevice) IsOpen() bool {
	return d.dev.IsOpen()
}

func (d *HIDDevice) Close() error {
	return convertHIDErr(d.dev.Close())
}

func (d *HIDDevice) Location() string {
	return d.dev.Location()
}

func (d *HIDDevice) SetOutputReport(report []byte) error {
	return d.dev.SetOutputReport(0, report)
}

func (d *HIDDevice) GetFeatureReport() ([]byte, error) {
	data, err := d.dev.GetFeatureReport(0)
	if len(data) == 0 {
		return nil, errors.New("invalid feature report: no report id")
	}

	if data[0] != 0 {
		return nil, fmt.Errorf("unexpected report id: 0 (expected) != %d", data[0])
	}

	return data[1:], err
}

func convertHIDErr(err error) error {
	if err == nil {
		return nil
	}

	switch {
	case errors.Is(err, usbhid.ErrNoDeviceFound):
		return &Error{
			Code: ErrorCodeNoDev,
			Msg:  err.Error(),
		}

	case errors.Is(err, usbhid.ErrDeviceLocked):
		return &Error{
			Code: ErrorCodeDevBusy,
			Msg:  err.Error(),
		}

	default:
		return err
	}
}
