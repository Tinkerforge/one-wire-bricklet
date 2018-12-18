package main

import (
	"fmt"
	"time"
    "github.com/tinkerforge/go-api-bindings/ipconnection"
    "github.com/tinkerforge/go-api-bindings/one_wire_bricklet"
)

const ADDR string = "localhost:4223"
const UID string = "XYZ" // Change XYZ to the UID of your One Wire Bricklet.

func main() {
	ipcon := ipconnection.New()
    defer ipcon.Close()
	ow, _ := one_wire_bricklet.New(UID, &ipcon) // Create device object.

	ipcon.Connect(ADDR) // Connect to brickd.
    defer ipcon.Disconnect()
	// Don't use device before ipcon is connected.

	ow.WriteCommand(0, 78) // WRITE SCRATCHPAD
	ow.Write(0)            // ALARM H (unused)
	ow.Write(0)            // ALARM L (unused)
	ow.Write(127)          // CONFIGURATION: 12 bit mode

	// Read temperature 10 times
	for i := 0; i < 10; i++ {
		ow.WriteCommand(0, 68)              // CONVERT T (start temperature conversion)
		time.Sleep(1000 * time.Millisecond) // Wait for conversion to finish
		ow.WriteCommand(0, 190)             // READ SCRATCHPAD
        
        tLow, _, _ := ow.Read()
        tHigh, _, _ := ow.Read()
        fmt.Printf("Temperature %f°C\n", float32(uint16(tLow) | uint16(tHigh) << 8) / 16.0)
	}

	fmt.Print("Press enter to exit.")
	fmt.Scanln()

	ipcon.Disconnect()
}
