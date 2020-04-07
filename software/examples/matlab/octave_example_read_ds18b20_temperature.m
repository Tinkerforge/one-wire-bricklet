function octave_example_read_ds18b20_temperature()
    more off;

    HOST = "localhost";
    PORT = 4223;
    UID = "XYZ"; % Change XYZ to the UID of your One Wire Bricklet

    ipcon = javaObject("com.tinkerforge.IPConnection"); % Create IP connection
    ow = javaObject("com.tinkerforge.BrickletOneWire", UID, ipcon); % Create device object

    ipcon.connect(HOST, PORT); % Connect to brickd
    % Don't use device before ipcon is connected

    ow.writeCommand(0, 78); % WRITE SCRATCHPAD
    ow.write(0); % ALARM H (unused)
    ow.write(0); % ALARM L (unused)
    ow.write(127); % CONFIGURATION: 12-bit mode

    % Read temperature 10 times
    for i = 0:9
        ow.writeCommand(0, 68); % CONVERT T (start temperature conversion)
        pause(1); % Wait for conversion to finish
        ow.writeCommand(0, 190); % READ SCRATCHPAD

        t_low = ow.read();
        t_high = ow.read();

        temperature = bitor(java2int(t_low.data), bitshift(java2int(t_high.data), 8));

        % Negative 12-bit values are sign-extended to 16-bit two's complement
        if (temperature > bitshift(1, 12))
            temperature -= bitshift(1, 16);
        end

        % 12-bit mode measures in units of 1/16°C
        fprintf('Temperature: %f °C\n', temperature/16.0);
    end

    input("Press key to exit\n", "s");
    ipcon.disconnect();
end

function int = java2int(value)
    if compare_versions(version(), "3.8", "<=")
        int = value.intValue();
    else
        int = value;
    end
end
