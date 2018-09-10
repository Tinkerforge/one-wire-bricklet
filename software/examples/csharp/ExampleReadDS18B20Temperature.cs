using System;
using System.Threading;
using Tinkerforge;

// FIXME: This example is incomplete

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
		ow.Write(127); // CONFIGURATION: 12 bit mode

		// Read temperature 10 times
		for(int i = 0; i < 10; i++)
		{
			ow.WriteCommand(0, 68); // CONVERT T (start temperature conversion)
			Thread.Sleep(1000); // Wait for conversion to finish
			ow.WriteCommand(0, 190); // READ SCRATCHPAD
		}

		Console.WriteLine("Press enter to exit");
		Console.ReadLine();
		ipcon.Disconnect();
	}
}
