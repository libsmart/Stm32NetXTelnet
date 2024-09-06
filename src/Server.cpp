/*
 * SPDX-FileCopyrightText: 2024 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "Server.hpp"
#include "LogicalConnection.hpp"
#include "Stm32NetX.hpp"
#include "StreamRxTx.hpp"

UINT Stm32NetXTelnet::Server::create(CHAR *server_name, NX_IP *ip_ptr, void *stack_ptr, ULONG stack_size,
                                     new_connection_cb *new_connection,
                                     receive_data_cb *receive_data,
                                     connection_end_cb *connection_end) {
    log(Stm32ItmLogger::LoggerInterface::Severity::INFORMATIONAL)
            ->println("Stm32NetXTelnet::Server::create()");

    // https://github.com/eclipse-threadx/rtos-docs/blob/main/rtos-docs/netx-duo/netx-duo-telnet/chapter3.md#nx_telnet_server_create
    const auto ret = nx_telnet_server_create(
        this,
        server_name,
        ip_ptr,
        stack_ptr,
        stack_size,
        new_connection,
        receive_data,
        connection_end
    );
    if (ret != NX_SUCCESS) {
        log(Stm32ItmLogger::LoggerInterface::Severity::ERROR)
                ->printf("nx_telnet_server_create() = 0x%02x\r\n", ret);
    }
    return ret;
}

UINT Stm32NetXTelnet::Server::create(CHAR *server_name, NX_IP *ip_ptr, void *stack_ptr, ULONG stack_size) {
    return create(
        server_name,
        ip_ptr,
        stack_ptr,
        stack_size,
        bounce<Server, decltype(&Server::new_connection), &Server::new_connection, UINT>,
        bounce<Server, decltype(&Server::receive_data), &Server::receive_data, UINT, NX_PACKET *>,
        bounce<Server, decltype(&Server::connection_end), &Server::connection_end, UINT>
    );
}

void Stm32NetXTelnet::Server::new_connection(NX_TELNET_SERVER_STRUCT *telnet_server_ptr, UINT logical_connection) {
    Stm32ItmLogger::logger.setSeverity(Stm32ItmLogger::LoggerInterface::Severity::DEBUGGING)
            ->println("Stm32NetXTelnet::Server::new_connection()");

    LIBSMART_UNUSED(telnet_server_ptr);

    auto session = getSessionManager()->getNewSession(logical_connection);
    if (session != nullptr) {
        char name[25]{};
        snprintf(name, sizeof(name), "Telnet Session %d", logical_connection);
        session->setName(name);
        session->setLogger(getLogger());
        session->setup();
    }
}

void Stm32NetXTelnet::Server::receive_data(NX_TELNET_SERVER_STRUCT *telnet_server_ptr, UINT logical_connection,
                                           NX_PACKET *packet_ptr) {
    // Stm32ItmLogger::logger.setSeverity(Stm32ItmLogger::LoggerInterface::Severity::DEBUGGING)
    // ->println("Stm32NetXTelnet::Server::receive_data()");

    LIBSMART_UNUSED(telnet_server_ptr);

    auto session = getSessionManager()->getSessionById(logical_connection);
    if(session != nullptr) {
        ULONG length = 0;
        ULONG bytes_copied = 0;
        nx_packet_length_get(packet_ptr, &length);
        if (length <= session->getRxBuffer()->availableForWrite()) {
            auto ret = nx_packet_data_retrieve(packet_ptr, session->getRxBuffer()->getWritePointer(), &bytes_copied);
            if (ret == NX_SUCCESS) {
                session->getRxBuffer()->setWrittenBytes(bytes_copied);
            }
        }
    }
    nx_packet_release(packet_ptr);
}

void Stm32NetXTelnet::Server::connection_end(NX_TELNET_SERVER_STRUCT *telnet_server_ptr, UINT logical_connection) {
    Stm32ItmLogger::logger.setSeverity(Stm32ItmLogger::LoggerInterface::Severity::DEBUGGING)
            ->println("Stm32NetXTelnet::Server::connection_end()");

    LIBSMART_UNUSED(telnet_server_ptr);

    auto session = getSessionManager()->getNewSession(logical_connection);
    if (session != nullptr) {
        session->end();
        getSessionManager()->removeSession(session);
    }
}

UINT Stm32NetXTelnet::Server::del() {
    log(Stm32ItmLogger::LoggerInterface::Severity::INFORMATIONAL)
            ->println("Stm32NetXTelnet::Server::del()");


    getSessionManager()->end();

    // https://github.com/eclipse-threadx/rtos-docs/blob/main/rtos-docs/netx-duo/netx-duo-telnet/chapter3.md#nx_telnet_server_delete
    const auto ret = nx_telnet_server_delete(this);
    if (ret != NX_SUCCESS) {
        log(Stm32ItmLogger::LoggerInterface::Severity::ERROR)
                ->printf("nx_telnet_server_delete() = 0x%02x\r\n", ret);
    }
    return ret;
}

UINT Stm32NetXTelnet::Server::disconnect(UINT logical_connection) {
    log(Stm32ItmLogger::LoggerInterface::Severity::INFORMATIONAL)
            ->println("Stm32NetXTelnet::Server::disconnect()");


    getSessionManager()->removeSession(getSessionManager()->getSessionById(logical_connection));

    // https://github.com/eclipse-threadx/rtos-docs/blob/main/rtos-docs/netx-duo/netx-duo-telnet/chapter3.md#nx_telnet_server_disconnect
    const auto ret = nx_telnet_server_disconnect(this, logical_connection);
    if (ret != NX_SUCCESS) {
        log(Stm32ItmLogger::LoggerInterface::Severity::ERROR)
                ->printf("nx_telnet_server_disconnect() = 0x%02x\r\n", ret);
    }
    return ret;
}

UINT Stm32NetXTelnet::Server::getOpenConnectionCount(UINT &connection_count) {
    log(Stm32ItmLogger::LoggerInterface::Severity::INFORMATIONAL)
            ->println("Stm32NetXTelnet::Server::getOpenConnectionCount()");

    // https://github.com/eclipse-threadx/rtos-docs/blob/main/rtos-docs/netx-duo/netx-duo-telnet/chapter3.md#nx_telnet_server_get_open_connection_count
    const auto ret = nx_telnet_server_get_open_connection_count(this, &connection_count);
    if (ret != NX_SUCCESS) {
        log(Stm32ItmLogger::LoggerInterface::Severity::ERROR)
                ->printf("nx_telnet_server_get_open_connection_count() = 0x%02x\r\n", ret);
    }
    return ret;
}

UINT Stm32NetXTelnet::Server::packetSend(UINT logical_connection, NX_PACKET *packet_ptr, ULONG wait_option) {
    // log(Stm32ItmLogger::LoggerInterface::Severity::DEBUGGING)
    // ->println("Stm32NetXTelnet::Server::packetSend()");

    // https://github.com/eclipse-threadx/rtos-docs/blob/main/rtos-docs/netx-duo/netx-duo-telnet/chapter3.md#nx_telnet_server_packet_send
    const auto ret = nx_telnet_server_packet_send(this, logical_connection, packet_ptr, wait_option);
    if (ret != NX_SUCCESS) {
        log(Stm32ItmLogger::LoggerInterface::Severity::ERROR)
                ->printf("nx_telnet_server_packet_send() = 0x%02x\r\n", ret);
    }
    return ret;
}

UINT Stm32NetXTelnet::Server::bufferSend(UINT logical_connection, void *buffer, size_t szBuffer, ULONG wait_option) {
    NX_PACKET *packet{};
    // NX_PACKET_POOL *packetPool = this->nx_telnet_server_packet_pool_ptr;
    NX_PACKET_POOL *packetPool = Stm32NetX::NX->getPacketPool();

    // check, if there are bytes to write

    auto ret = nx_packet_allocate(packetPool, &packet, NX_TCP_PACKET, wait_option);
    if (ret != NX_SUCCESS) {
        log(Stm32ItmLogger::LoggerInterface::Severity::ERROR)
                ->printf("nx_packet_allocate() = 0x%02x\r\n", ret);
        return ret;
    }
    ret = nx_packet_data_append(packet, buffer, szBuffer, packetPool, wait_option);
    if (ret != NX_SUCCESS) {
        log(Stm32ItmLogger::LoggerInterface::Severity::ERROR)
                ->printf("nx_packet_data_append() = 0x%02x\r\n", ret);
        nx_packet_release(packet);
        return ret;
    }
    return packetSend(logical_connection, packet, wait_option);
}

#ifdef NX_TELNET_SERVER_USER_CREATE_PACKET_POOL
UINT Stm32NetXTelnet::Server::packetPoolSet(NX_PACKET_POOL *packet_pool_ptr) {
    log(Stm32ItmLogger::LoggerInterface::Severity::INFORMATIONAL)
        ->println("Stm32NetXTelnet::Server::packetPoolSet()");

    // https://github.com/eclipse-threadx/rtos-docs/blob/main/rtos-docs/netx-duo/netx-duo-telnet/chapter3.md#nx_telnet_server_packet_pool_set
    const auto ret = nx_telnet_server_packet_pool_set(this packet_pool_ptr);
    if (ret != NX_SUCCESS) {
        log(Stm32ItmLogger::LoggerInterface::Severity::ERROR)
                ->printf("nx_telnet_server_packet_pool_set() = 0x%02x\r\n", ret);
    }
    return ret;
}
#endif

UINT Stm32NetXTelnet::Server::start() {
    log(Stm32ItmLogger::LoggerInterface::Severity::INFORMATIONAL)
            ->println("Stm32NetXTelnet::Server::start()");

    // https://github.com/eclipse-threadx/rtos-docs/blob/main/rtos-docs/netx-duo/netx-duo-telnet/chapter3.md#nx_telnet_server_start
    const auto ret = nx_telnet_server_start(this);
    if (ret != NX_SUCCESS) {
        log(Stm32ItmLogger::LoggerInterface::Severity::ERROR)
                ->printf("nx_telnet_server_start() = 0x%02x\r\n", ret);
    }
    getSessionManager()->setup();
    return ret;
}

UINT Stm32NetXTelnet::Server::stop() {
    log(Stm32ItmLogger::LoggerInterface::Severity::INFORMATIONAL)
            ->println("Stm32NetXTelnet::Server::stop()");

    getSessionManager()->end();

    // https://github.com/eclipse-threadx/rtos-docs/blob/main/rtos-docs/netx-duo/netx-duo-telnet/chapter3.md#nx_telnet_server_stop
    const auto ret = nx_telnet_server_stop(this);
    if (ret != NX_SUCCESS) {
        log(Stm32ItmLogger::LoggerInterface::Severity::ERROR)
                ->printf("nx_telnet_server_stop() = 0x%02x\r\n", ret);
    }
    return ret;
}

void Stm32NetXTelnet::Server::loop() {
    // Call the loop() function of the connections

    getSessionManager()->loop();

    NX_PACKET *packet{};
    // NX_PACKET_POOL *packetPool = this->nx_telnet_server_packet_pool_ptr;
    NX_PACKET_POOL *packetPool = Stm32NetX::NX->getPacketPool();

    // check, if there are bytes to write
    auto session = getSessionManager()->getFirstSession();
    while (session != nullptr) {
        if (session->getTxBuffer()->available() > 0) {
            auto szBuffer = session->getTxBuffer()->available();
            auto ret = bufferSend(session->getId(),
                (void *) session->getTxBuffer()->getReadPointer(), szBuffer, 100);
            if (ret == NX_SUCCESS) {
                session->getTxBuffer()->remove(szBuffer);
            }
        }
        session = getSessionManager()->getNextSession(session);
    }
}

void Stm32NetXTelnet::Server::end() {
    stop();
}

