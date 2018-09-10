#define IPCON_EXPOSE_MILLISLEEP

#include <stdio.h>

// FIXME: This example is incomplete

#include "ip_connection.h"
#include "bricklet_one_wire.h"

#define HOST "localhost"
#define PORT 4223
#define UID "XYZ" // Change XYZ to the UID of your One Wire Bricklet

int main(void) {
	// Create IP connection
	IPConnection ipcon;
	ipcon_create(&ipcon);

	// Create device object
	OneWire ow;
	one_wire_create(&ow, UID, &ipcon);

	// Connect to brickd
	if(ipcon_connect(&ipcon, HOST, PORT) < 0) {
		fprintf(stderr, "Could not connect\n");
		return 1;
	}
	// Don't use device before ipcon is connected

	one_wire_write_command(&ow, 0, 78); // WRITE SCRATCHPAD
	one_wire_write(&ow, 0); // ALARM H (unused)
	one_wire_write(&ow, 0); // ALARM L (unused)
	one_wire_write(&ow, 127); // CONFIGURATION: 12 bit mode

	// Read temperature 10 times
	int i;
	for(i = 0; i < 10; ++i) {
		one_wire_write_command(&ow, 0, 68); // CONVERT T (start temperature conversion)
		millisleep(1000); // Wait for conversion to finish
		one_wire_write_command(&ow, 0, 190); // READ SCRATCHPAD
	}

	printf("Press key to exit\n");
	getchar();
	one_wire_destroy(&ow);
	ipcon_destroy(&ipcon); // Calls ipcon_disconnect internally
	return 0;
}
