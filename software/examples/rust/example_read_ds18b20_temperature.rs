use std::{error::Error, io, thread, time::Duration};
use tinkerforge::{ipconnection::IpConnection, one_wire_bricklet::*};

const HOST: &str = "127.0.0.1";
const PORT: u16 = 4223;
const UID: &str = "XYZ"; // Change XYZ to the UID of your One Wire Bricklet

fn main() -> Result<(), Box<dyn Error>> {
    let ipcon = IpConnection::new(); // Create IP connection
    let one_wire_bricklet = OneWireBricklet::new(UID, &ipcon); // Create device object

    ipcon.connect(HOST, PORT).recv()??; // Connect to brickd
                                        // Don't use device before ipcon is connected

    one_wire_bricklet.write_command(0, 78); // WRITE SCRATCHPAD
    one_wire_bricklet.write(0); // ALARM H (unused)
    one_wire_bricklet.write(0); // ALARM L (unused)
    one_wire_bricklet.write(127); // CONFIGURATION: 12 bit mode

    // Read temperature 10 times
    for _i in 0..10 {
        one_wire_bricklet.write_command(0, 68); // CONVERT T (start temperature conversion)
        thread::sleep(Duration::from_millis(1000)); // Wait for conversion to finish
        one_wire_bricklet.write_command(0, 190); // READ SCRATCHPAD

        let t_low = one_wire_bricklet.read().recv()?.data;
        let t_high = one_wire_bricklet.read().recv()?.data;

        println!("Temperature: {}°C", ((t_low as u16 | (t_high as u16) << 8) as f32 / 16.0));
    }

    println!("Press enter to exit.");
    let mut _input = String::new();
    io::stdin().read_line(&mut _input)?;
    ipcon.disconnect();
    Ok(())
}