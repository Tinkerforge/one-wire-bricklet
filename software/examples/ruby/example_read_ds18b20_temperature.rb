#!/usr/bin/env ruby
# -*- ruby encoding: utf-8 -*-

require 'tinkerforge/ip_connection'
require 'tinkerforge/bricklet_one_wire'

include Tinkerforge

HOST = 'localhost'
PORT = 4223
UID = 'XYZ' # Change XYZ to the UID of your One Wire Bricklet

ipcon = IPConnection.new # Create IP connection
ow = BrickletOneWire.new UID, ipcon # Create device object

ipcon.connect HOST, PORT # Connect to brickd
# Don't use device before ipcon is connected

ow.write_command 0, 78 # WRITE SCRATCHPAD
ow.write 0 # ALARM H (unused)
ow.write 0 # ALARM L (unused)
ow.write 127 # CONFIGURATION: 12 bit mode

# Read temperature 10 times
for _ in 0..9
  ow.write_command 0, 68 # CONVERT T (start temperature conversion)
  sleep 1 # Wait for conversion to finish
  ow.write_command 0, 190 # READ SCRATCHPAD

  t_low = ow.read
  t_high = ow.read
  temperature = (t_low[0] | (t_high[0] << 8))/16.0

  puts "Temperature: #{temperature} °C"
end

puts 'Press key to exit'
$stdin.gets
ipcon.disconnect
