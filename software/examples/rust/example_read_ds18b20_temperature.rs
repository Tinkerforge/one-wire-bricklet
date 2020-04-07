use std::{error::Error, io, thread, time::Duration};
use tinkerforge::{ip_connection::IpConnection, one_wire_bricklet::*};

const HOST: &str = "localhost";
const PORT: u16 = 4223;
const UID: &str = "XYZ"; // Change XYZ to the UID of your One Wire Bricklet.

fn main() -> Result<(), Box<dyn Error>> {
    let ipcon = IpConnection::new(); // Create IP connection.
    let ow = OneWireBricklet::new(UID, &ipcon); // Create device object.

    ipcon.connect((HOST, PORT)).recv()??; // Connect to brickd.
                                          // Don't use device before ipcon is connected.

    ow.write_command(0, 78); // WRITE SCRATCHPAD
    ow.write(0); // ALARM H (unused)
    ow.write(0); // ALARM L (unused)
    ow.write(127); // CONFIGURATION: 12-bit mode

    // Read temperature 10 times
    for _i in 0..10 {
        ow.write_command(0, 68); // CONVERT T (start temperature conversion)
        thread::sleep(Duration::from_millis(1000)); // Wait for conversion to finish
        ow.write_command(0, 190).recv()?; // READ SCRATCHPAD; use recv()? to block until the command is sent to the bricklet.

        let t_low = ow.read().recv()?.data;
        let t_high = ow.read().recv()?.data;

        let mut temperature = (t_low | (t_high << 8)) as f32;

        // Negative 12-bit values are sign-extended to 16-bit two's complement
        if temperature > (1 << 12) as f32 {
            temperature -= (1 << 16) as f32;
        }

        // 12-bit mode measures in units of 1/16°C
        println!("Temperature: {} °C", temperature / 16.0);
    }

    println!("Press enter to exit.");
    let mut _input = String::new();
    io::stdin().read_line(&mut _input)?;
    ipcon.disconnect();
    Ok(())
}
