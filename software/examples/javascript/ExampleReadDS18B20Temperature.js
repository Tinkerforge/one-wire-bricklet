var Tinkerforge = require('tinkerforge');

// FIXME: This example is incomplete

var HOST = 'localhost';
var PORT = 4223;
var UID = 'XYZ'; // Change XYZ to the UID of your One Wire Bricklet

var ipcon = new Tinkerforge.IPConnection(); // Create IP connection
var ow = new Tinkerforge.BrickletOneWire(UID, ipcon); // Create device object

ipcon.connect(HOST, PORT,
    function (error) {
        console.log('Error: ' + error);
    }
); // Connect to brickd
// Don't use device before ipcon is connected

ipcon.on(Tinkerforge.IPConnection.CALLBACK_CONNECTED,
    function (connectReason) {
        ow.writeCommand(0, 78); // WRITE SCRATCHPAD
        ow.write(0); // ALARM H (unused)
        ow.write(0); // ALARM L (unused)
        ow.write(127); // CONFIGURATION: 12 bit mode

        // Read temperature 10 times
        for(var i = 0; i < 10; ++i) {
            ow.writeCommand(0, 68); // CONVERT T (start temperature conversion)

            setTimeout(function () {
                ow.writeCommand(0, 190); // READ SCRATCHPAD
            }, 1000 * i + 1000); // Wait for conversion to finish
        }
    }
);

console.log('Press key to exit');
process.stdin.on('data',
    function (data) {
        ipcon.disconnect();
        process.exit(0);
    }
);
