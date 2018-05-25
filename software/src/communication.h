/* one-wire-bricklet
 * Copyright (C) 2018 Olaf Lüke <olaf@tinkerforge.com>
 *
 * communication.h: TFP protocol message handling
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

#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <stdint.h>
#include <stdbool.h>

#include "bricklib2/protocols/tfp/tfp.h"
#include "bricklib2/bootloader/bootloader.h"

// Default functions
BootloaderHandleMessageResponse handle_message(const void *data, void *response);
void communication_tick(void);
void communication_init(void);

// Constants
#define ONE_WIRE_STATUS_OK 0
#define ONE_WIRE_STATUS_BUSY 1
#define ONE_WIRE_STATUS_NO_PRESENCE 2
#define ONE_WIRE_STATUS_TIMEOUT 3
#define ONE_WIRE_STATUS_ERROR 4

#define ONE_WIRE_COMMUNICATION_LED_CONFIG_OFF 0
#define ONE_WIRE_COMMUNICATION_LED_CONFIG_ON 1
#define ONE_WIRE_COMMUNICATION_LED_CONFIG_SHOW_HEARTBEAT 2
#define ONE_WIRE_COMMUNICATION_LED_CONFIG_SHOW_COMMUNICATION 3

#define ONE_WIRE_BOOTLOADER_MODE_BOOTLOADER 0
#define ONE_WIRE_BOOTLOADER_MODE_FIRMWARE 1
#define ONE_WIRE_BOOTLOADER_MODE_BOOTLOADER_WAIT_FOR_REBOOT 2
#define ONE_WIRE_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_REBOOT 3
#define ONE_WIRE_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_ERASE_AND_REBOOT 4

#define ONE_WIRE_BOOTLOADER_STATUS_OK 0
#define ONE_WIRE_BOOTLOADER_STATUS_INVALID_MODE 1
#define ONE_WIRE_BOOTLOADER_STATUS_NO_CHANGE 2
#define ONE_WIRE_BOOTLOADER_STATUS_ENTRY_FUNCTION_NOT_PRESENT 3
#define ONE_WIRE_BOOTLOADER_STATUS_DEVICE_IDENTIFIER_INCORRECT 4
#define ONE_WIRE_BOOTLOADER_STATUS_CRC_MISMATCH 5

#define ONE_WIRE_STATUS_LED_CONFIG_OFF 0
#define ONE_WIRE_STATUS_LED_CONFIG_ON 1
#define ONE_WIRE_STATUS_LED_CONFIG_SHOW_HEARTBEAT 2
#define ONE_WIRE_STATUS_LED_CONFIG_SHOW_STATUS 3

// Function and callback IDs and structs
#define FID_SEARCH_BUS_LOW_LEVEL 1
#define FID_RESET_BUS 2
#define FID_WRITE 3
#define FID_READ 4
#define FID_WRITE_COMMAND 5
#define FID_SET_COMMUNICATION_LED_CONFIG 6
#define FID_GET_COMMUNICATION_LED_CONFIG 7


typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) SearchBusLowLevel;

typedef struct {
	TFPMessageHeader header;
	uint16_t identifier_length;
	uint16_t identifier_chunk_offset;
	uint64_t identifier_chunk_data[7];
	uint8_t status;
} __attribute__((__packed__)) SearchBusLowLevel_Response;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) ResetBus;

typedef struct {
	TFPMessageHeader header;
	uint8_t status;
} __attribute__((__packed__)) ResetBus_Response;

typedef struct {
	TFPMessageHeader header;
	uint8_t data;
} __attribute__((__packed__)) Write;

typedef struct {
	TFPMessageHeader header;
	uint8_t status;
} __attribute__((__packed__)) Write_Response;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) Read;

typedef struct {
	TFPMessageHeader header;
	uint8_t data;
	uint8_t status;
} __attribute__((__packed__)) Read_Response;

typedef struct {
	TFPMessageHeader header;
	uint64_t identifier;
	uint8_t command;
} __attribute__((__packed__)) WriteCommand;

typedef struct {
	TFPMessageHeader header;
	uint8_t status;
} __attribute__((__packed__)) WriteCommand_Response;

typedef struct {
	TFPMessageHeader header;
	uint8_t config;
} __attribute__((__packed__)) SetCommunicationLEDConfig;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) GetCommunicationLEDConfig;

typedef struct {
	TFPMessageHeader header;
	uint8_t config;
} __attribute__((__packed__)) GetCommunicationLEDConfig_Response;


// Function prototypes
BootloaderHandleMessageResponse search_bus_low_level(const SearchBusLowLevel *data, SearchBusLowLevel_Response *response);
BootloaderHandleMessageResponse reset_bus(const ResetBus *data, ResetBus_Response *response);
BootloaderHandleMessageResponse write(const Write *data, Write_Response *response);
BootloaderHandleMessageResponse read(const Read *data, Read_Response *response);
BootloaderHandleMessageResponse write_command(const WriteCommand *data, WriteCommand_Response *response);
BootloaderHandleMessageResponse set_communication_led_config(const SetCommunicationLEDConfig *data);
BootloaderHandleMessageResponse get_communication_led_config(const GetCommunicationLEDConfig *data, GetCommunicationLEDConfig_Response *response);

// Callbacks


#define COMMUNICATION_CALLBACK_TICK_WAIT_MS 1
#define COMMUNICATION_CALLBACK_HANDLER_NUM 0
#define COMMUNICATION_CALLBACK_LIST_INIT \


#endif
