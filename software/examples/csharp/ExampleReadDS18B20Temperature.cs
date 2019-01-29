using System;
using System.Threading;
using Tinkerforge;

class Example
{
	private static string HOST = "localhost";
	private static int PORT = 4223;
	private static string UID = "XYZ"; // Change XYZ to the UID of your One Wire Bricklet

	static void Main()
	{
		IPConnection ipcon = new IPConnection(); // Create IP connection
		BrickletOneWire ow = new BrickletOneWire(UID, ipcon); // Create device object

		ipcon.Connect(HOST, PORT); // Connect to brickd
		// Don't use device before ipcon is connected

		ow.WriteCommand(0, 78); // WRITE SCRATCHPAD
		ow.Write(0); // ALARM H (unused)
		ow.Write(0); // ALARM L (unused)
		ow.Write(127); // CONFIGURATION: 12-bit mode

		// Read temperature 10 times
		for(int i = 0; i < 10; i++)
		{
			ow.WriteCommand(0, 68); // CONVERT T (start temperature conversion)
			Thread.Sleep(1000); // Wait for conversion to finish
			ow.WriteCommand(0, 190); // READ SCRATCHPAD

			byte status;
			byte t_low;
			ow.Read(out t_low, out status);

			byte t_high;
			ow.Read(out t_high, out status);

			float temperature = t_low | (t_high << 8);

			// Negative 12-bit values are sign-extended to 16-bit two's complement
			if (temperature > 1 << 12) {
				temperature -= 1 << 16;
			}

			// 12-bit mode measures in units of 1/16°C
			Console.WriteLine("Temperature: " + temperature/16.0 + " °C");
		}

		Console.WriteLine("Press enter to exit");
		Console.ReadLine();
		ipcon.Disconnect();
	}
}
