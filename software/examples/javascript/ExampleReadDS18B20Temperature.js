var Tinkerforge = require('tinkerforge');

var HOST = 'localhost';
var PORT = 4223;
var UID = 'XYZ'; // Change XYZ to the UID of your One Wire Bricklet

var ipcon = new Tinkerforge.IPConnection(); // Create IP connection
var ow = new Tinkerforge.BrickletOneWire(UID, ipcon); // Create device object

var writeCommandPromise = function(ow, identifier, command) {
    var promise = new Promise(function(resolve, reject) {
        ow.writeCommand(identifier, command, function(status) {
            resolve(status);
        });
    });

    return promise;
}

var writePromise = function(ow, data) {
    var promise = new Promise(function(resolve, reject) {
        ow.write(data, function(status) {
            resolve(status);
        });
    });

    return promise;
}

var readPromise = function(ow) {
    var promise = new Promise(function(resolve, reject) {
        ow.read(function(data, status) {
            resolve(data, status);
        });
    });

    return promise;
}

var timeoutPromise = function(millis) {
    var promise = new Promise(function(resolve, reject) {
        setTimeout(function () {
            resolve();
        }, millis);
    });

    return promise;
}

ipcon.connect(HOST, PORT,
    function (error) {
        console.log('Error: ' + error);
    }
); // Connect to brickd
// Don't use device before ipcon is connected

ipcon.on(Tinkerforge.IPConnection.CALLBACK_CONNECTED,
    async function (connectReason) {
        await writeCommandPromise(ow, 0, 78); // WRITE SCRATCHPAD
        await writePromise(ow, 0); // ALARM H (unused)
        await writePromise(ow, 0); // ALARM L (unused)
        await writePromise(ow, 127); // CONFIGURATION: 12-bit mode

        // Read temperature 10 times
        for(var i = 0; i < 10; ++i) {
            await writeCommandPromise(ow, 0, 68); // CONVERT T (start temperature conversion)
            await timeoutPromise(1000);
            await writeCommandPromise(ow, 0, 190); // READ SCRATCHPAD

            var t_low = await readPromise(ow);
            var t_high = await readPromise(ow);
            var temperature = t_low | (t_high << 8);

            // Negative 12-bit values are sign-extended to 16-bit two's complement
            if (temperature > (1 << 12)) {
                temperature -= 1 << 16;
            }

            // 12 bit-mode measures in units of 1/16°C
            console.log('Temperature: ' + temperature/16.0 + ' °C');
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
