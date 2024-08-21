/*
 * SPDX-FileCopyrightText: 2024 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef LIBSMART_STM32NETXTELNET_LIBSMART_CONFIG_DIST_HPP
#define LIBSMART_STM32NETXTELNET_LIBSMART_CONFIG_DIST_HPP

#define LIBSMART_STM32NETXTELNET

/**
 * Maximum number of connections the telnet server handles
 */
#define LIBSMART_STM32NETXTELNET_SERVER_MAX_CONNECTIONS 4


/**
 * Size of the rx buffer per telnet logicalConnection
 */
#define LIBSMART_STM32NETXTELNET_BUFFER_SIZE_RX 256


/**
 * Size of the tx buffer per telnet logicalConnection
 */
#define LIBSMART_STM32NETXTELNET_BUFFER_SIZE_TX 256

#endif
