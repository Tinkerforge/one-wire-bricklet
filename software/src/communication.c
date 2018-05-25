/* one-wire-bricklet
 * Copyright (C) 2018 Olaf Lüke <olaf@tinkerforge.com>
 *
 * communication.c: TFP protocol message handling
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

#include "communication.h"

#include "bricklib2/utility/communication_callback.h"
#include "bricklib2/utility/util_definitions.h"
#include "bricklib2/protocols/tfp/tfp.h"

#include "bricklib2/logging/logging.h"

#include "one_wire.h"
#include "configs/config_one_wire.h"

BootloaderHandleMessageResponse handle_message(const void *message, void *response) {
	switch(tfp_get_fid_from_message(message)) {
		case FID_SEARCH_BUS_LOW_LEVEL: return search_bus_low_level(message, response);
		case FID_RESET_BUS: return reset_bus(message, response);
		case FID_WRITE: return write(message, response);
		case FID_READ: return read(message, response);
		case FID_WRITE_COMMAND: return write_command(message, response);
		case FID_SET_COMMUNICATION_LED_CONFIG: return set_communication_led_config(message);
		case FID_GET_COMMUNICATION_LED_CONFIG: return get_communication_led_config(message, response);
		default: return HANDLE_MESSAGE_RESPONSE_NOT_SUPPORTED;
	}
}


BootloaderHandleMessageResponse search_bus_low_level(const SearchBusLowLevel *data, SearchBusLowLevel_Response *response) {
	response->header.length = sizeof(SearchBusLowLevel_Response);

	// If there is a stream ongoing we can handle it here (IDs are already searched)
	if(one_wire.search_bus_length > 0) {
		response->status = one_wire.response_status;
		response->identifier_length = one_wire.search_bus_length;
		response->identifier_chunk_offset = one_wire.search_bus_offset;
		for(uint8_t i = 0; i < MIN(7, one_wire.ids_num - one_wire.search_bus_offset); i++) {
			response->identifier_chunk_data[i] = one_wire.ids[i + one_wire.search_bus_offset];
		}

		if(one_wire.search_bus_length > (one_wire.search_bus_offset + 7)) {
			// If there is more data to send we increase the offset
			one_wire.search_bus_offset += 7;
		} else {
			// If we are done, we set offset and length back to 0
			one_wire.search_bus_offset = 0;
			one_wire.search_bus_length = 0;
		}

		return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
	}

	// If a function is currently executed we can't do anything
	if(one_wire.function != ONE_WIRE_FUNCTION_NONE) {
		response->status = ONE_WIRE_STATUS_BUSY;
		return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
	} 
	
	// If there is no stream ongoing and there is no function currently executed, we start a bus search
	one_wire.response_header = response->header;
	one_wire.function = ONE_WIRE_FUNCTION_SEARCH_BUS;

	return HANDLE_MESSAGE_RESPONSE_NONE;
}

BootloaderHandleMessageResponse reset_bus(const ResetBus *data, ResetBus_Response *response) {
	response->header.length = sizeof(ResetBus_Response);

	if(one_wire.function != ONE_WIRE_FUNCTION_NONE) {
		response->status = ONE_WIRE_STATUS_BUSY;
		return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
	} 
	
	one_wire.response_header = response->header;
	one_wire.function = ONE_WIRE_FUNCTION_RESET_BUS;

	return HANDLE_MESSAGE_RESPONSE_NONE;
}

BootloaderHandleMessageResponse write(const Write *data, Write_Response *response) {
	response->header.length = sizeof(Write_Response);

	if(one_wire.function != ONE_WIRE_FUNCTION_NONE) {
		response->status = ONE_WIRE_STATUS_BUSY;
		return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
	} 

	one_wire.response_header = response->header;
	one_wire.function = ONE_WIRE_FUNCTION_WRITE;
	one_wire.function_write_data = data->data;

	return HANDLE_MESSAGE_RESPONSE_NONE;
}

BootloaderHandleMessageResponse read(const Read *data, Read_Response *response) {
	response->header.length = sizeof(Read_Response);
	if(one_wire.function != ONE_WIRE_FUNCTION_NONE) {
		response->status = ONE_WIRE_STATUS_BUSY;
		return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
	} 

	one_wire.response_header = response->header;
	one_wire.function = ONE_WIRE_FUNCTION_READ;

	return HANDLE_MESSAGE_RESPONSE_NONE;
}

BootloaderHandleMessageResponse write_command(const WriteCommand *data, WriteCommand_Response *response) {
	response->header.length = sizeof(WriteCommand_Response);
	if(one_wire.function != ONE_WIRE_FUNCTION_NONE) {
		response->status = ONE_WIRE_STATUS_BUSY;
		return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
	} 

	one_wire.response_header = response->header;
	one_wire.function = ONE_WIRE_FUNCTION_WRITE_COMMAND;
	one_wire.function_write_command = data->command;
	one_wire.function_write_command_id = data->identifier;

	return HANDLE_MESSAGE_RESPONSE_NONE;
}

BootloaderHandleMessageResponse set_communication_led_config(const SetCommunicationLEDConfig *data) {
	if(data->config > ONE_WIRE_COMMUNICATION_LED_CONFIG_SHOW_COMMUNICATION) {
		return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
	}

	one_wire.com_led_state.config = data->config;

	// Set LED according to value
	if(one_wire.com_led_state.config == ONE_WIRE_COMMUNICATION_LED_CONFIG_OFF) {
		XMC_GPIO_SetOutputHigh(ONE_WIRE_COM_LED_PIN);
	} else {
		XMC_GPIO_SetOutputLow(ONE_WIRE_COM_LED_PIN);
	}

	return HANDLE_MESSAGE_RESPONSE_EMPTY;
}

BootloaderHandleMessageResponse get_communication_led_config(const GetCommunicationLEDConfig *data, GetCommunicationLEDConfig_Response *response) {
	response->header.length = sizeof(GetCommunicationLEDConfig_Response);
	response->config        = one_wire.com_led_state.config;

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}


void communication_tick(void) {
	if(one_wire.function_ready && bootloader_spitfp_is_send_possible(&bootloader_status.st)) {
		switch(one_wire.function) {
			case ONE_WIRE_FUNCTION_SEARCH_BUS: {
				if(one_wire.ids_num > 7) {
					one_wire.search_bus_length = one_wire.ids_num;
					one_wire.search_bus_offset = 7;
				}

				SearchBusLowLevel_Response response;
				response.header = one_wire.response_header;
				response.status = one_wire.response_status;
				response.identifier_length = one_wire.ids_num;
				response.identifier_chunk_offset = 0;
				for(uint8_t i = 0; i < MIN(7, one_wire.ids_num); i++) {
					response.identifier_chunk_data[i] = one_wire.ids[i];
				}

				bootloader_spitfp_send_ack_and_message(&bootloader_status, (uint8_t*)&response, sizeof(SearchBusLowLevel_Response));
				break;
			}

			case ONE_WIRE_FUNCTION_RESET_BUS: {
				ResetBus_Response response;
				response.header = one_wire.response_header;
				response.status = one_wire.response_status;
				bootloader_spitfp_send_ack_and_message(&bootloader_status, (uint8_t*)&response, sizeof(ResetBus_Response));

				break;
			}

			case ONE_WIRE_FUNCTION_WRITE: {
				Write_Response response;
				response.header = one_wire.response_header;
				response.status = one_wire.response_status;
				bootloader_spitfp_send_ack_and_message(&bootloader_status, (uint8_t*)&response, sizeof(Write_Response));

				break;
			}

			case ONE_WIRE_FUNCTION_READ: {
				Read_Response response;
				response.header = one_wire.response_header;
				response.status = one_wire.response_status;
				response.data   = one_wire.function_read_data;
				bootloader_spitfp_send_ack_and_message(&bootloader_status, (uint8_t*)&response, sizeof(Read_Response));

				break;
			}

			case ONE_WIRE_FUNCTION_WRITE_COMMAND: {
				WriteCommand_Response response;
				response.header = one_wire.response_header;
				response.status = one_wire.response_status;
				bootloader_spitfp_send_ack_and_message(&bootloader_status, (uint8_t*)&response, sizeof(WriteCommand_Response));

				break;
			}

			default:
			case ONE_WIRE_FUNCTION_NONE: {

				break;
			}
		}

		if(one_wire.response_status == ONE_WIRE_STATUS_TIMEOUT) {
			one_wire_init();
		}

		one_wire.function = ONE_WIRE_FUNCTION_NONE;
		one_wire.function_ready = false;
		one_wire.response_status = ONE_WIRE_STATUS_OK;
	} 
}

void communication_init(void) {
	one_wire.function = ONE_WIRE_FUNCTION_NONE;
	one_wire.function_ready = false;
	one_wire.response_status = ONE_WIRE_STATUS_OK;
}
