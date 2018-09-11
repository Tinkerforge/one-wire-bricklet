#!/bin/sh
# Connects to localhost:4223 by default, use --host and --port to change this

uid=XYZ # Change XYZ to the UID of your One Wire Bricklet

tinkerforge call one-wire-bricklet $uid write-command 0 78 >/dev/null 2>&1 # WRITE SCRATCHPAD
tinkerforge call one-wire-bricklet $uid write 0 >/dev/null 2>&1 # ALARM H (unused)
tinkerforge call one-wire-bricklet $uid write 0 >/dev/null 2>&1 # ALARM L (unused)
tinkerforge call one-wire-bricklet $uid write 127 >/dev/null 2>&1 # CONFIGURATION: 12 bit mode

# Read temperature 10 times
for i in 0 1 2 3 4 5 6 7 8 9; do
	tinkerforge call one-wire-bricklet $uid write-command 0 68 >/dev/null 2>&1 # CONVERT T (start temperature conversion)
	sleep 1 # Wait for conversion to finish
	tinkerforge call one-wire-bricklet $uid write-command 0 190 >/dev/null 2>&1 # READ SCRATCHPAD

	tinkerforge call one-wire-bricklet $uid read
	tinkerforge call one-wire-bricklet $uid read
done
