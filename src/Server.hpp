/*
 * SPDX-FileCopyrightText: 2024 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef LIBSMART_STM32NETXTELNET_SERVER_HPP
#define LIBSMART_STM32NETXTELNET_SERVER_HPP

#include "Loggable.hpp"
#include "Nameable.hpp"
#include "nx_api.h"
#include "netxduo/addons/telnet/nxd_telnet_server.h"
#include "StreamRxTx.hpp"
#include "StreamSession/StreamSessionAware.hpp"

namespace Stm32NetXTelnet {
    class Server
            : protected NX_TELNET_SERVER,
              public Stm32Common::Process::ProcessInterface,
              public Stm32Common::StreamSession::StreamSessionAware,
              public Stm32ItmLogger::Loggable,
              public Stm32Common::Nameable {
    public:
        using new_connection_cb = void(NX_TELNET_SERVER_STRUCT *telnet_server_ptr, UINT logical_connection);
        using receive_data_cb = void(NX_TELNET_SERVER_STRUCT *telnet_server_ptr, UINT logical_connection,
                                     NX_PACKET *packet_ptr);
        using connection_end_cb = void(NX_TELNET_SERVER_STRUCT *telnet_server_ptr, UINT logical_connection);

        explicit Server(Stm32Common::StreamSession::ManagerInterface *session_mgr)
            : NX_TELNET_SERVER(), StreamSessionAware(session_mgr) { ; }


        /**
         * @brief Creates a Telnet server instance.
         *
         * This method initializes a Telnet server instance with specified parameters and callback functions
         * for handling new connections, receiving data, and connection termination.
         *
         * @param server_name A pointer to a character string that specifies the name of the server.
         * @param ip_ptr A pointer to an NX_IP structure that specifies the IP instance for the server.
         * @param stack_ptr A pointer to the stack memory allocated for the server.
         * @param stack_size The size of the stack memory allocated for the server.
         * @param new_connection A callback function that is invoked when a new connection is established.
         * @param receive_data A callback function that is invoked when data is received.
         * @param connection_end A callback function that is invoked when a connection ends.
         *
         * @return An unsigned integer status code indicating the result of the server creation process.
         */
        UINT create(CHAR *server_name, NX_IP *ip_ptr, VOID *stack_ptr, ULONG stack_size,
                    new_connection_cb *new_connection,
                    receive_data_cb *receive_data,
                    connection_end_cb *connection_end
        );


        /**
         * @brief Creates a Telnet server instance.
         *
         * This method initializes a Telnet server instance with specified parameters.
         *
         * @param server_name A pointer to a character string that specifies the name of the server.
         * @param ip_ptr A pointer to an NX_IP structure that specifies the IP instance for the server.
         * @param stack_ptr A pointer to the stack memory allocated for the server.
         * @param stack_size The size of the stack memory allocated for the server.
         *
         * @return An unsigned integer status code indicating the result of the server creation process.
         */
        UINT create(CHAR *server_name, NX_IP *ip_ptr, VOID *stack_ptr, ULONG stack_size);


        /**
         * @brief Handles new Telnet server connections.
         *
         * This method is invoked when a new connection to the Telnet server is established. It manages
         * the session by setting up the necessary parameters and initializing the session environment.
         *
         * @param telnet_server_ptr A pointer to the NX_TELNET_SERVER_STRUCT that specifies the Telnet server instance.
         * @param logical_connection An unsigned integer that represents the logical connection identifier for the new connection.
         */
        void new_connection(NX_TELNET_SERVER_STRUCT *telnet_server_ptr, UINT logical_connection);


        /**
         * @brief Handles incoming data for a specific Telnet server connection.
         *
         * This method processes the received data packet for a given Telnet server
         * connection and stores the extracted data into the connection's receive buffer.
         *
         * @param telnet_server_ptr A pointer to the Telnet server structure.
         * @param logical_connection The identifier for the logical connection that has received data.
         * @param packet_ptr A pointer to the NX_PACKET structure containing the received data.
         */
        void receive_data(NX_TELNET_SERVER_STRUCT *telnet_server_ptr, UINT logical_connection,
                          NX_PACKET *packet_ptr);

        /**
         * @brief Handles the termination of a Telnet connection.
         *
         * This method is called when a Telnet connection ends. It retrieves the associated session
         * using the logical connection identifier, ends the session, and removes it from the session manager.
         *
         * @param telnet_server_ptr A pointer to the NX_TELNET_SERVER_STRUCT that represents the Telnet server instance.
         * @param logical_connection The identifier of the logical connection that has ended.
         */
        void connection_end(NX_TELNET_SERVER_STRUCT *telnet_server_ptr, UINT logical_connection);


        /**
         * @brief Deletes a Telnet server instance.
         *
         * This method terminates the Telnet server instance and releases any resources associated with it.
         * It also logs informational and error messages indicating the status of the deletion process.
         *
         * @return An unsigned integer status code indicating the result of the server deletion process.
         */
        UINT del();


        /**
         * @brief Disconnects a logical connection from the Telnet server.
         *
         * This method removes a session associated with the provided logical connection ID and calls the
         * underlying NetX Duo function to perform the disconnection process.
         *
         * @param logical_connection The ID of the logical connection to be disconnected.
         *
         * @return An unsigned integer status code indicating the result of the disconnection process.
         */
        UINT disconnect(UINT logical_connection);


        /**
         * @brief Retrieves the number of currently open connections on the Telnet server.
         *
         * This method queries the Telnet server instance to get the count of currently open connections
         * and stores the result in the provided reference parameter.
         *
         * @param connection_count A reference to an unsigned integer where the count of open connections will be stored.
         *
         * @return An unsigned integer status code indicating the result of the query operation.
         */
        UINT getOpenConnectionCount(UINT &connection_count);


        /**
         * @brief Sends a packet to a specified logical connection within the Telnet server.
         *
         * This method sends a given NetX packet to a Telnet client connected through a specified logical connection.
         *
         * @param logical_connection An unsigned integer specifying the logical connection to which the packet is sent.
         * @param packet_ptr A pointer to an NX_PACKET structure that contains the data to be sent.
         * @param wait_option An unsigned long indicating the wait option if the send operation cannot be completed immediately.
         *
         * @return An unsigned integer status code indicating the result of the packet send operation.
         */
        UINT packetSend(UINT logical_connection, NX_PACKET *packet_ptr, ULONG wait_option);


        /**
         * @brief Sends a buffer of data over a logical connection.
         *
         * This method transfers the specified buffer of data over the given logical connection.
         *
         * @param logical_connection The logical connection identifier over which to send the data.
         * @param buffer A pointer to the data buffer to be sent.
         * @param szBuffer The size of the data buffer to be sent.
         * @param wait_option The wait option for packet operations.
         *
         * @return A UINT status code indicating the outcome of the send operation.
         */
        UINT bufferSend(UINT logical_connection, void *buffer, size_t szBuffer, ULONG wait_option);

#ifdef NX_TELNET_SERVER_USER_CREATE_PACKET_POOL
        UINT packetPoolSet(NX_PACKET_POOL *packet_pool_ptr);
#endif


        /**
         * @brief Starts the Telnet server instance.
         *
         * This method initiates the Telnet server by invoking the underlying NetX API to start
         * the server. It also logs the starting process and any errors encountered.
         *
         * @return An unsigned integer status code indicating the result of the server start process.
         *         A value of NX_SUCCESS (0) indicates successful start; other values indicate an error.
         */
        UINT start();


        /**
         * @brief Stops the Telnet server instance.
         *
         * This method terminates the Telnet server, signaling the session manager to end the sessions and calling
         * the NetX Duo function to stop the server.
         *
         * @return An unsigned integer status code indicating the result of the Telnet server stop process.
         */
        UINT stop();


        /**
         * @brief Sets up the server instance.
         *
         * This method overrides the base class setup function to perform any necessary initialization
         * or configuration required for the server instance to function properly.
         */
        void setup() override { ; }


        /**
         * @brief Executes the main loop of the Telnet server.
         *
         * This method runs the primary loop required for the Telnet server operation.
         * It processes session management and handles sending data from transmission buffers.
         */
        void loop() override;


        /**
         * @brief Terminates the Telnet server instance.
         *
         * This method stops the running Telnet server, effectively terminating any active connections
         * and ceasing to listen for new incoming connections.
         *
         * @return void This method does not return a value.
         */
        void end() override;


        /**
         * @brief Retrieves the session manager instance.
         *
         * This method obtains the session manager instance that manages stream sessions.
         * It overrides the base implementation to provide the specific session manager associated
         * with the stream sessions.
         *
         * @return A pointer to the ManagerInterface, which controls the session management.
         */
        Stm32Common::StreamSession::ManagerInterface *getSessionManager() override {
            return StreamSessionAware::getSessionManager();
        }

    protected:
        template<class T, class Method, Method m, class... Params>
        /**
         * @brief Invokes a specified member function on the Telnet server instance.
         *
         * This static method casts the given Telnet server pointer to the appropriate type and calls the specified member function.
         * It ensures that the provided Telnet server pointer is not null before invoking the function.
         *
         * @param telnet_server_ptr A pointer to the NX_TELNET_SERVER_STRUCT that specifies the Telnet server instance.
         * @param params The parameters to be forwarded to the member function.
         *
         * @return The result of the member function call.
         */
        static auto bounce(NX_TELNET_SERVER_STRUCT *telnet_server_ptr, Params... params) ->
            decltype(((*static_cast<T *>(telnet_server_ptr)).*m)(telnet_server_ptr, params...)) {
            assert_param(telnet_server_ptr != nullptr);
            T *telnetServer = static_cast<T *>(telnet_server_ptr);
            assert_param(telnetServer != nullptr);
            return ((*telnetServer).*m)(telnet_server_ptr, params...);
        }
    };
}

#endif
