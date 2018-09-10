#!/usr/bin/perl

# FIXME: This example is incomplete

use strict;
use Tinkerforge::IPConnection;
use Tinkerforge::BrickletOneWire;

use constant HOST => 'localhost';
use constant PORT => 4223;
use constant UID => 'XYZ'; # Change XYZ to the UID of your One Wire Bricklet

my $ipcon = Tinkerforge::IPConnection->new(); # Create IP connection
my $ow = Tinkerforge::BrickletOneWire->new(&UID, $ipcon); # Create device object

$ipcon->connect(&HOST, &PORT); # Connect to brickd
# Don't use device before ipcon is connected

$ow->write_command(0, 78); # WRITE SCRATCHPAD
$ow->write(0); # ALARM H (unused)
$ow->write(0); # ALARM L (unused)
$ow->write(127); # CONFIGURATION: 12 bit mode

# Read temperature 10 times
for (my $i = 0; $i < 10; $i++)
{
    $ow->write_command(0, 68); # CONVERT T (start temperature conversion)
    sleep(1); # Wait for conversion to finish
    $ow->write_command(0, 190); # READ SCRATCHPAD
}

print "Press key to exit\n";
<STDIN>;
$ipcon->disconnect();
