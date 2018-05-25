/* one-wire-bricklet
 * Copyright (C) 2018 Olaf Lüke <olaf@tinkerforge.com>
 *
 * one_wire.h: One Wire protocol implementation
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

#ifndef ONE_WIRE_H
#define ONE_WIRE_H

#include <stdint.h>
#include <stdbool.h>

#include "bricklib2/utility/led_flicker.h"
#include "bricklib2/protocols/tfp/tfp.h"

#define ONE_WIRE_OVERSAMPLING 16
#define ONE_WIRE_BAUDRATE_RESET 9600
#define ONE_WIRE_BAUDRATE_DATA 115200

#define ONE_WIRE_IDS_MAX 64

#define ONE_WIRE_COMMAND_MATCH_ROM    0x55
#define ONE_WIRE_COMMAND_SKIP_ROM     0xCC
#define ONE_WIRE_COMMAND_SEARCH_ROM   0xF0

typedef enum {
    ONE_WIRE_FUNCTION_NONE = 0,
    ONE_WIRE_FUNCTION_SEARCH_BUS,
    ONE_WIRE_FUNCTION_RESET_BUS,
    ONE_WIRE_FUNCTION_WRITE,
    ONE_WIRE_FUNCTION_READ,
    ONE_WIRE_FUNCTION_WRITE_COMMAND,
} OneWireFunction;

typedef struct {
    uint32_t error_count_parity;
    LEDFlickerState com_led_state;

    uint8_t value;
    OneWireFunction function;

    uint8_t search_rom_no[8];
    uint8_t search_last_discrepancy;
    uint8_t search_last_family_discrepancy;
    bool search_last_device_flag;

    uint64_t ids[ONE_WIRE_IDS_MAX];
    uint8_t ids_num;

    TFPMessageHeader response_header;
    uint8_t response_status;

    bool function_ready;
    uint8_t function_write_data;
    uint8_t function_read_data;
    uint8_t function_write_command;
    uint64_t function_write_command_id;

    uint16_t search_bus_offset;
    uint16_t search_bus_length;
} OneWire;

extern OneWire one_wire;

void one_wire_tick(void);
void one_wire_init(void);

#endif