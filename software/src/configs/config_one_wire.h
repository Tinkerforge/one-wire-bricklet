/* one-wire-bricklet
 * Copyright (C) 2018, 2023 Olaf Lüke <olaf@tinkerforge.com>
 *
 * config_one_wire.h: One Wire specific configurations
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

#ifndef CONFIG_ONE_WIRE_H
#define CONFIG_ONE_WIRE_H

#include "xmc_common.h"
#include "xmc_uart.h"
#include "xmc_gpio.h"

#define ONE_WIRE_USIC_CHANNEL        USIC0_CH1
#define ONE_WIRE_USIC                XMC_UART0_CH1

#define ONE_WIRE_RX_PIN              P0_7
#define ONE_WIRE_RX_INPUT            XMC_USIC_CH_INPUT_DX0
#define ONE_WIRE_RX_SOURCE           0b011 // DX0D.

#define ONE_WIRE_TX_PIN              P0_6
#define ONE_WIRE_TX_PIN_AF           (XMC_GPIO_MODE_OUTPUT_PUSH_PULL_ALT7 | P0_6_AF_U0C1_DOUT0)

#define ONE_WIRE_RX11_PIN            P0_6
#define ONE_WIRE_RX11_INPUT          XMC_USIC_CH_INPUT_DX0
#define ONE_WIRE_RX11_SOURCE         0b010 // DX0C.

#define ONE_WIRE_TX11_PIN            P0_7
#define ONE_WIRE_TX11_PIN_AF         (XMC_GPIO_MODE_OUTPUT_PUSH_PULL_ALT7 | P0_7_AF_U0C1_DOUT0)

#define ONE_WIRE_VERSION_DETECTION0  P0_9
#define ONE_WIRE_VERSION_DETECTION1  P0_12

#define ONE_WIRE_COM_LED_PIN         P1_1

#define ONE_WIRE_SERVICE_REQUEST_RX  2  // RX.
#define ONE_WIRE_SERVICE_REQUEST_TX  3  // TX.
#define ONE_WIRE_SERVICE_REQUEST_RXA 4  // Alternate RX (parity error).

#define ONE_WIRE_IRQ_RX              11
#define ONE_WIRE_IRQ_RX_PRIORITY     0

#define ONE_WIRE_IRQ_TX              12
#define ONE_WIRE_IRQ_TX_PRIORITY     1

#define ONE_WIRE_IRQ_RXA             13
#define ONE_WIRE_IRQ_RXA_PRIORITY    0

#endif