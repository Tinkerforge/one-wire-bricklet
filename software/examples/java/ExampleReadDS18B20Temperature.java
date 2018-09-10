import com.tinkerforge.IPConnection;
import com.tinkerforge.BrickletOneWire;

public class ExampleReadDS18B20Temperature {
	private static final String HOST = "localhost";
	private static final int PORT = 4223;

	// Change XYZ to the UID of your One Wire Bricklet
	private static final String UID = "XYZ";

	// Note: To make the example code cleaner we do not handle exceptions. Exceptions
	//       you might normally want to catch are described in the documentation
	public static void main(String args[]) throws Exception {
		IPConnection ipcon = new IPConnection(); // Create IP connection
		BrickletOneWire ow = new BrickletOneWire(UID, ipcon); // Create device object

		ipcon.connect(HOST, PORT); // Connect to brickd
		// Don't use device before ipcon is connected

		ow.writeCommand(0, 78); // WRITE SCRATCHPAD
		ow.write(0); // ALARM H (unused)
		ow.write(0); // ALARM L (unused)
		ow.write(127); // CONFIGURATION: 12 bit mode

		// Read temperature 10 times
		for(int i = 0; i < 10; i++) {
			ow.writeCommand(0, 68); // CONVERT T (start temperature conversion)
			Thread.sleep(1000); // Wait for conversion to finish
			ow.writeCommand(0, 190); // READ SCRATCHPAD

			int tLow = ow.read().data;
			int tHigh = ow.read().data;

			System.out.println("Temperature: " + (tLow | (tHigh << 8))/16.0  + " °C");
		}

		System.out.println("Press key to exit"); System.in.read();
		ipcon.disconnect();
	}
}
