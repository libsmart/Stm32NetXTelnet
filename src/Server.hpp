/*
 * SPDX-FileCopyrightText: 2024 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef LIBSMART_STM32NETXTELNET_SERVER_HPP
#define LIBSMART_STM32NETXTELNET_SERVER_HPP

// #include "globals.hpp"
#include "Loggable.hpp"
#include "Nameable.hpp"
#include "nx_api.h"
#include "netxduo/addons/telnet/nxd_telnet_server.h"
#include "LogicalConnectionMicrorl.hpp"
#include "StreamRxTx.hpp"

namespace Stm32NetXTelnet {
    class Server : protected NX_TELNET_SERVER, public Stm32ItmLogger::Loggable, public Stm32Common::Nameable {
    public:
        using logicalConnection_t = LogicalConnectionMicrorl;

        Server() : NX_TELNET_SERVER() {
            self = this;
        }

        UINT create(CHAR *server_name,
                    NX_IP *ip_ptr,
                    VOID *stack_ptr,
                    ULONG stack_size,
                    void (*new_connection)(
                        struct NX_TELNET_SERVER_STRUCT *telnet_server_ptr,
                        UINT logical_connection),
                    void (*receive_data)(
                        struct NX_TELNET_SERVER_STRUCT *telnet_server_ptr,
                        UINT logical_connection,
                        NX_PACKET *packet_ptr),
                    void (*connection_end)(
                        struct NX_TELNET_SERVER_STRUCT *telnet_server_ptr,
                        UINT logical_connection));

        UINT create(CHAR *server_name,
                    NX_IP *ip_ptr,
                    VOID *stack_ptr,
                    ULONG stack_size);

        static void new_connection(NX_TELNET_SERVER_STRUCT *telnet_server_ptr, UINT logical_connection);

        static void receive_data(NX_TELNET_SERVER_STRUCT *telnet_server_ptr, UINT logical_connection,
                                 NX_PACKET *packet_ptr);

        static void connection_end(NX_TELNET_SERVER_STRUCT *telnet_server_ptr, UINT logical_connection);


        UINT del();

        UINT disconnect(UINT logical_connection);

        UINT getOpenConnectionCount(UINT &connection_count);

        UINT packetSend(UINT logical_connection,
                        NX_PACKET *packet_ptr,
                        ULONG wait_option);

        UINT bufferSend(UINT logical_connection, void *buffer, size_t szBuffer, ULONG wait_option);

#ifdef NX_TELNET_SERVER_USER_CREATE_PACKET_POOL
        UINT packetPoolSet(NX_PACKET_POOL *packet_pool_ptr);
#endif

        UINT start();

        UINT stop();


        virtual void loop();

        logicalConnection_t *getLogicalConnection(UINT logical_connection);

    private:
        inline static Server *self{};
        logicalConnection_t *logicalConnection[LIBSMART_STM32NETXTELNET_SERVER_MAX_CONNECTIONS]{};
    };
}

#endif
