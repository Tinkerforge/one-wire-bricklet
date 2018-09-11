<?php

require_once('Tinkerforge/IPConnection.php');
require_once('Tinkerforge/BrickletOneWire.php');

use Tinkerforge\IPConnection;
use Tinkerforge\BrickletOneWire;

const HOST = 'localhost';
const PORT = 4223;
const UID = 'XYZ'; // Change XYZ to the UID of your One Wire Bricklet

$ipcon = new IPConnection(); // Create IP connection
$ow = new BrickletOneWire(UID, $ipcon); // Create device object

$ipcon->connect(HOST, PORT); // Connect to brickd
// Don't use device before ipcon is connected

$ow->writeCommand(0, 78); // WRITE SCRATCHPAD
$ow->write(0); // ALARM H (unused)
$ow->write(0); // ALARM L (unused)
$ow->write(127); // CONFIGURATION: 12 bit mode

// Read temperature 10 times
for($i = 0; $i < 10; $i++) {
    $ow->writeCommand(0, 68); // CONVERT T (start temperature conversion)
    sleep(1); // Wait for conversion to finish
    $ow->writeCommand(0, 190); // READ SCRATCHPAD

    $t_low = $ow->read();
    $t_high = $ow->read();
	$temperature = ($t_low['data'] | ($t_high['data'] << 8)) / 16.0;

	echo "Temperature: " . $temperature . " °C\n";
}

echo "Press key to exit\n";
fgetc(fopen('php://stdin', 'r'));
$ipcon->disconnect();

?>
