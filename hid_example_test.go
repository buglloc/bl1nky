package bl1nky_test

import (
	"log"

	"github.com/buglloc/bl1nky"
)

func ExampleNewHIDBl1nky() {
	h, err := bl1nky.NewHIDBl1nky()
	if err != nil {
		log.Fatalf("create bl1nky: %v\n", err)
	}

	if err := h.Open(); err != nil {
		log.Fatalf("open device: %v\n", err)
	}
	defer h.Close()

	// Turn on LED 1 and LED 2
	err = h.SetLEDs(bl1nky.Led1 | bl1nky.Led2)
	if err != nil {
		log.Fatalf("set LEDs: %v\n", err)
	}

	// Read current LED state
	leds, err := h.GetLEDs()
	if err != nil {
		log.Fatalf("get LEDs: %v\n", err)
	}
	log.Printf("Current LED state: 0b%08b\n", leds)
}
