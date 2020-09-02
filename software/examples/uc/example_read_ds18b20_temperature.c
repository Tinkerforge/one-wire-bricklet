// This example is not self-contained.
// It requres usage of the example driver specific to your platform.
// See the HAL documentation.

#include "bindings/hal_common.h"
#include "bindings/bricklet_one_wire.h"

#define UID "XYZ" // Change XYZ to the UID of your One Wire Bricklet

void example_setup(TF_HalContext *hal);
void example_loop(TF_HalContext *hal);

void check(int rc, const char* msg);

static TF_OneWire ow;

void example_setup(TF_HalContext *hal) {
	// Create device object
	check(tf_one_wire_create(&ow, UID, hal), "create device object");

	uint8_t status;
	check(tf_one_wire_write_command(&ow, 0, 78, &status), "call write_command"); // WRITE SCRATCHPAD
	check(tf_one_wire_write(&ow, 0, &status), "call write"); // ALARM H (unused)
	check(tf_one_wire_write(&ow, 0, &status), "call write"); // ALARM L (unused)
	check(tf_one_wire_write(&ow, 127, &status), "call write"); // CONFIGURATION: 12-bit mode

	// Read temperature 10 times
	int i;
	for(i = 0; i < 10; ++i) {
		check(tf_one_wire_write_command(&ow, 0, 68, &status), "call write_command"); // CONVERT T (start temperature conversion)
		tf_hal_sleep_us(hal, 1000 * 1000); // Wait for conversion to finish
		check(tf_one_wire_write_command(&ow, 0, 190, &status), "call write_command"); // READ SCRATCHPAD

		uint8_t t_low;
		check(tf_one_wire_read(&ow, &t_low, &status), "read low byte");

		uint8_t t_high;
		check(tf_one_wire_read(&ow, &t_high, &status), "read high byte");

		uint16_t temperature = t_low | (uint16_t)(t_high << 8);

		// Negative 12-bit values are sign-extended to 16-bit two's complement
		if (temperature > 1 << 12) {
			temperature -= 1 << 16;
		}

		// 12-bit mode measures in units of 1/16°C
		// tf_hal_printf does not support %f or padding, so we
		// have to create the decimal number by hand.
		uint16_t temp_degrees = temperature >> 4;
		uint16_t temp_tenthousandth_degree = 625 * (temperature & 0x0F);

		char padding[5] = {0};

		if(temp_tenthousandth_degree < 1000)
			padding[0] = '0';
		if(temp_tenthousandth_degree < 100)
			padding[1] = '0';
		if(temp_tenthousandth_degree < 10)
			padding[2] = '0';
		if(temp_tenthousandth_degree < 1)
			padding[3] = '0';

		tf_hal_printf("Temperature: %I16d.%s%I16d °C\n", temp_degrees, padding, temp_tenthousandth_degree);
	}
}

void example_loop(TF_HalContext *hal) {
	// Poll for callbacks
	tf_hal_callback_tick(hal, 0);
}
