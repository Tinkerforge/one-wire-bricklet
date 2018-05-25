/* one-wire-bricklet
 * Copyright (C) 2018 Olaf Lüke <olaf@tinkerforge.com>
 *
 * one_wire.c: One Wire protocol implementation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "one_wire.h"

#include "configs/config_one_wire.h"

#include "bricklib2/os/coop_task.h"
#include "bricklib2/hal/system_timer/system_timer.h"
#include "bricklib2/logging/logging.h"

#include "communication.h"

#include <string.h>

static const uint8_t one_wire_crc_table[] = {
      0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65,
    157,195, 33,127,252,162, 64, 30, 95,  1,227,189, 62, 96,130,220,
     35,125,159,193, 66, 28,254,160,225,191, 93,  3,128,222, 60, 98,
    190,224,  2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255,
     70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89,  7,
    219,133,103, 57,186,228,  6, 88, 25, 71,165,251,120, 38,196,154,
    101, 59,217,135,  4, 90,184,230,167,249, 27, 69,198,152,122, 36,
    248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91,  5,231,185,
    140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205,
     17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80,
    175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238,
     50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115,
    202,148,118, 40,171,245, 23, 73,  8, 86,180,234,105, 55,213,139,
     87,  9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22,
    233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168,
    116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53
};

OneWire one_wire;
CoopTask one_wire_task;

#define one_wire_rxa_irq_handler IRQ_Hdlr_13


void __attribute__((optimize("-O3"))) __attribute__ ((section (".ram_code"))) one_wire_rxa_irq_handler() {
	one_wire.error_count_parity++;

	logw("Parity Error: %d\n\r", one_wire.error_count_parity);
}


void one_wire_init_uart(void) {
	// For prototype only, remove me!!!
	const XMC_GPIO_CONFIG_t proto_config = {
		.mode = XMC_GPIO_MODE_INPUT_TRISTATE,
		.input_hysteresis = XMC_GPIO_INPUT_HYSTERESIS_STANDARD
	};
	XMC_GPIO_Init(P0_0, &proto_config);

	// RX pin configuration.
	const XMC_GPIO_CONFIG_t rx_pin_config = {
		.mode = XMC_GPIO_MODE_INPUT_PULL_UP,
		.input_hysteresis = XMC_GPIO_INPUT_HYSTERESIS_STANDARD
	};

	// TX pin configuration.
	const XMC_GPIO_CONFIG_t tx_pin_config = {
		.mode = ONE_WIRE_TX_PIN_AF,
		.output_level = XMC_GPIO_OUTPUT_LEVEL_HIGH
	};

	// Configure  pins.
	XMC_GPIO_Init(ONE_WIRE_RX_PIN, &rx_pin_config);
	XMC_GPIO_Init(ONE_WIRE_TX_PIN, &tx_pin_config);

	// Initialize USIC channel in UART mode.

	// USIC channel configuration.
	XMC_UART_CH_CONFIG_t config = {
        .baudrate = ONE_WIRE_BAUDRATE_RESET,
        .stop_bits = 1,
        .data_bits = 8,
        .oversampling = ONE_WIRE_OVERSAMPLING,
        .frame_length = 8,
		.parity_mode = XMC_USIC_CH_PARITY_MODE_NONE
    };

	XMC_UART_CH_Init(ONE_WIRE_USIC, &config);

	// Set input source path.
	XMC_UART_CH_SetInputSource(ONE_WIRE_USIC, ONE_WIRE_RX_INPUT, ONE_WIRE_RX_SOURCE);

	// Configure TX FIFO.
	XMC_USIC_CH_TXFIFO_Configure(ONE_WIRE_USIC, 32, XMC_USIC_CH_FIFO_SIZE_16WORDS, 8);

	// Configure RX FIFO.
	XMC_USIC_CH_RXFIFO_Configure(ONE_WIRE_USIC, 48, XMC_USIC_CH_FIFO_SIZE_16WORDS, 8);

	// UART protocol events.

	XMC_USIC_CH_SetInterruptNodePointer(
		ONE_WIRE_USIC,
		XMC_USIC_CH_INTERRUPT_NODE_POINTER_ALTERNATE_RECEIVE,
		ONE_WIRE_SERVICE_REQUEST_RXA
	);

	// Set priority and enable NVIC node for RXA interrupt.
	NVIC_SetPriority((IRQn_Type)ONE_WIRE_IRQ_RXA, ONE_WIRE_IRQ_RXA_PRIORITY);
	NVIC_EnableIRQ((IRQn_Type)ONE_WIRE_IRQ_RXA);

	// Restart UART: Stop.
	while(XMC_UART_CH_Stop(ONE_WIRE_USIC) != XMC_UART_CH_STATUS_OK);

	// Restart UART: Start.
	XMC_UART_CH_Start(ONE_WIRE_USIC);

	XMC_USIC_CH_EnableEvent(ONE_WIRE_USIC, XMC_USIC_CH_EVENT_ALTERNATIVE_RECEIVE);
}

uint8_t one_wire_task_transceive_uart_byte(uint8_t byte) {
	XMC_USIC_CH_RXFIFO_Flush(ONE_WIRE_USIC);
	ONE_WIRE_USIC->IN[0] = byte;

	uint32_t t = system_timer_get_ms();
	while(XMC_USIC_CH_RXFIFO_IsEmpty(ONE_WIRE_USIC)) {
		coop_task_yield();
		if(system_timer_is_time_elapsed_ms(t, 250)) {
			one_wire.response_status = ONE_WIRE_STATUS_TIMEOUT;
		}
	}

	return ONE_WIRE_USIC->OUTR;
}

bool one_wire_task_bit(bool value) {
	uint8_t bit = one_wire_task_transceive_uart_byte(value ? 0xFF : 0x00);
	return bit == 0xFF;
}

uint8_t one_wire_task_byte(uint8_t value) {
	led_flicker_increase_counter(&one_wire.com_led_state);

	uint8_t ret = 0;
	for(uint8_t i = 0; i < 8; i++) {
		ret |= one_wire_task_bit(value & (1 << i)) << i;
		if(one_wire.response_status == ONE_WIRE_STATUS_TIMEOUT) {
			return 0;
		}
	}

	return ret;
}

void one_wire_task_reset(void) {
	XMC_USIC_CH_SetBaudrate(ONE_WIRE_USIC, ONE_WIRE_BAUDRATE_RESET, ONE_WIRE_OVERSAMPLING);
	one_wire.value = one_wire_task_transceive_uart_byte(0xF0);
	if(one_wire.response_status != ONE_WIRE_STATUS_TIMEOUT) {
		switch(one_wire.value) {
			case 0x00: one_wire.response_status = ONE_WIRE_STATUS_ERROR;       break;
			case 0xF0: one_wire.response_status = ONE_WIRE_STATUS_NO_PRESENCE; break;
			default:   one_wire.response_status = ONE_WIRE_STATUS_OK;          break;
		}
	}
	XMC_USIC_CH_SetBaudrate(ONE_WIRE_USIC, ONE_WIRE_BAUDRATE_DATA, ONE_WIRE_OVERSAMPLING);
}

bool one_wire_task_search(void) {
	bool search_result = false;

	// If the last call was not the last one
	if(!one_wire.search_last_device_flag) {
		uint8_t crc = 0;
		uint8_t id_bit_number = 1;
		uint8_t last_zero = 0;
		uint8_t rom_byte_number = 0;
		uint8_t rom_byte_mask = 1;

		one_wire_task_reset();
		if(one_wire.response_status != ONE_WIRE_STATUS_OK) {
			// reset the search
			one_wire.search_last_discrepancy = 0;
			one_wire.search_last_device_flag = false;
			one_wire.search_last_family_discrepancy = 0;
			return false;
		}

		// Issue the search command 
		one_wire_task_byte(ONE_WIRE_COMMAND_SEARCH_ROM);
		if(one_wire.response_status == ONE_WIRE_STATUS_TIMEOUT) {
			return false;
		}

		// loop to do the search
		do {
			// Read a bit and its complement
			bool id_bit           = one_wire_task_bit(true);
			if(one_wire.response_status == ONE_WIRE_STATUS_TIMEOUT) {
				return false;
			}
			bool cmp_id_bit       = one_wire_task_bit(true);
			if(one_wire.response_status == ONE_WIRE_STATUS_TIMEOUT) {
				return false;
			}
			bool search_direction = false;

			// Check for no devices on 1-wire
			if((id_bit == 1) && (cmp_id_bit == 1)) {
				one_wire.response_status = ONE_WIRE_STATUS_NO_PRESENCE;
            	break;
			} else {
				// All devices coupled have 0 or 1
				if(id_bit != cmp_id_bit) {
					search_direction = id_bit;  // Bit write value for search
				} else {
					// If this discrepancy if before the Last Discrepancy
					// on a previous next then pick the same as last time
					if(id_bit_number < one_wire.search_last_discrepancy) {
						search_direction = ((one_wire.search_rom_no[rom_byte_number] & rom_byte_mask) > 0);
					} else {
						// If equal to last pick 1, if not then pick 0
						search_direction = (id_bit_number == one_wire.search_last_discrepancy);
					}

					// If 0 was picked then record its position in LastZero
					if(search_direction == 0) {
						last_zero = id_bit_number;
					
						// Check for Last discrepancy in family
						if(last_zero < 9) {
							one_wire.search_last_family_discrepancy = last_zero;
						}
					}
				}

				// Set or clear the bit in the ROM byte rom_byte_number
				// with mask rom_byte_mask
				if(search_direction == 1) {
					one_wire.search_rom_no[rom_byte_number] |= rom_byte_mask;
				} else {
					one_wire.search_rom_no[rom_byte_number] &= ~rom_byte_mask;
				}

				// Serial number search direction write bit
				one_wire_task_bit(search_direction);
				if(one_wire.response_status == ONE_WIRE_STATUS_TIMEOUT) {
					return false;
				}

				// Increment the byte counter id_bit_number
				// and shift the mask rom_byte_mask
				id_bit_number++;
				rom_byte_mask <<= 1;

				// If the mask is 0 then go to new SerialNum byte rom_byte_number and reset mask
				if(rom_byte_mask == 0) {
					crc = one_wire_crc_table[crc ^ one_wire.search_rom_no[rom_byte_number]];
					rom_byte_number++;
					rom_byte_mask = 1;
				}
			}
		} while(rom_byte_number < 8);  // Loop until through all ROM bytes 0-7

		// If the search was successful then
		if((id_bit_number >= 65) && (crc == 0)) {
			// Search successful
			one_wire.search_last_discrepancy = last_zero;
		

			// Check for last device
			if(one_wire.search_last_discrepancy == 0) {
				one_wire.search_last_device_flag = true;
			}

			search_result = true;
		}
	}

	// If no device found then reset counters so next 'search' will be like a first
	if(!search_result || !one_wire.search_rom_no[0]) {
		one_wire.search_last_discrepancy = 0;
		one_wire.search_last_device_flag = false;
		one_wire.search_last_family_discrepancy = 0;
		search_result = false;
	}

	return search_result;
}

void one_wire_task_search_all(void) {
	uint8_t num = 0;
	while(one_wire_task_search()) {
		for(uint8_t i = 0; i < 8; i++) {
			one_wire.ids[num] |= ((uint64_t)one_wire.search_rom_no[i]) << ((uint64_t)(i*8));
		}
		num++;
	}
	one_wire.ids_num = num;
}


void one_wire_task_tick(void) {
	while(true) {
		if(!one_wire.function_ready) {
			switch(one_wire.function) {
				case ONE_WIRE_FUNCTION_SEARCH_BUS: {
					one_wire_task_search_all();
					one_wire.function_ready = true;

					break;
				}

				case ONE_WIRE_FUNCTION_RESET_BUS: {
					one_wire_task_reset();
					one_wire.function_ready = true;

					break;
				}

				case ONE_WIRE_FUNCTION_WRITE: {
					uint8_t data = one_wire_task_byte(one_wire.function_write_data);
					if(one_wire.response_status != ONE_WIRE_STATUS_TIMEOUT) {
						if(data == one_wire.function_write_data) {
							one_wire.response_status = ONE_WIRE_STATUS_OK;
						} else {
							one_wire.response_status = ONE_WIRE_STATUS_ERROR;
						}
					}

					one_wire.function_ready = true;

					break;
				}

				case ONE_WIRE_FUNCTION_READ: {
					one_wire.function_read_data = one_wire_task_byte(0xFF);
					if(one_wire.response_status != ONE_WIRE_STATUS_TIMEOUT) {
						one_wire.response_status = ONE_WIRE_STATUS_OK;
					}

					one_wire.function_ready = true;

					break;
				}

				case ONE_WIRE_FUNCTION_WRITE_COMMAND: {
					one_wire_task_reset();

					if(one_wire.function_write_command_id != 0) {
						uint8_t *id = (uint8_t*)&one_wire.function_write_command_id;
						one_wire_task_byte(ONE_WIRE_COMMAND_MATCH_ROM);
						if(one_wire.response_status != ONE_WIRE_STATUS_TIMEOUT) {
							for(uint8_t i = 0; i < 8; i++) {
								one_wire_task_byte(id[i]);
								if(one_wire.response_status == ONE_WIRE_STATUS_TIMEOUT) {
									break;
								}
							}
						}
					} else {
						one_wire_task_byte(ONE_WIRE_COMMAND_SKIP_ROM);
					}

					if(one_wire.response_status != ONE_WIRE_STATUS_TIMEOUT) {
						one_wire_task_byte(one_wire.function_write_command);
						if(one_wire.response_status != ONE_WIRE_STATUS_TIMEOUT) {
							one_wire.response_status = ONE_WIRE_STATUS_OK;
						}
					}
					one_wire.function_ready = true;

					break;
				}

				default:
				case ONE_WIRE_FUNCTION_NONE: {
					break;
				}
			}
		}

		coop_task_yield();
	}
}

void one_wire_init(void) {
	static bool coop_init_done = false;

	memset(&one_wire, 0, sizeof(OneWire));

	const XMC_GPIO_CONFIG_t led_pin_config = {
		.mode = XMC_GPIO_MODE_OUTPUT_PUSH_PULL,
		.output_level = XMC_GPIO_OUTPUT_LEVEL_HIGH
	};
	XMC_GPIO_Init(ONE_WIRE_COM_LED_PIN, &led_pin_config);

	one_wire.com_led_state.config  = 3; //COMMUNICATION_LED_CONFIG_SHOW_COMMUNICATION;
	one_wire.com_led_state.counter = 0;
	one_wire.com_led_state.start   = 0;

	one_wire_init_uart();

	if(!coop_init_done) {
		coop_task_init(&one_wire_task, one_wire_task_tick);	
		coop_init_done = true;
	}
}


void one_wire_tick(void) {
	led_flicker_tick(&one_wire.com_led_state, system_timer_get_ms(), ONE_WIRE_COM_LED_PIN);

	coop_task_tick(&one_wire_task);
}

