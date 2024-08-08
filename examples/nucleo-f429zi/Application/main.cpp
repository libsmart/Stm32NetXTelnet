/*
 * SPDX-FileCopyrightText: 2024 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: AGPL-3.0-only
 */

/**
 * This file holds the main setup() and loop() functions for C++ code.
 * If a RTOS is used, loop() is called in the main task and setup() is called before RTOS initialization.
 * @see App_ThreadX_Init() in Core/Src/app_threadx.c
 */

#include "main.hpp"

#include <Server.hpp>
#include <Stm32NetXTelnet.hpp>

#include "eth.h"
#include "globals.hpp"
#include "RunEvery.hpp"
#include "RunOnce.hpp"
#include "Stm32NetX.hpp"


/**
 * @brief Setup function.
 * This function is called once at the beginning of the program before ThreadX is initialized.
 * @see main() in Core/Src/main.c
 */
void setup() {
    Stm32ItmLogger::logger.setSeverity(Stm32ItmLogger::LoggerInterface::Severity::INFORMATIONAL)
            ->println("::setup()");

    dummyCpp = 0;
    dummyCandCpp = 0;
}


void loopOnce() {
    Stm32ItmLogger::logger.setSeverity(Stm32ItmLogger::LoggerInterface::Severity::INFORMATIONAL)
            ->println("::loopOnce()");

#ifdef TX_ENABLE_STACK_CHECKING
    tx_thread_stack_error_notify(Stack_Error_Handler);
#endif

    static char hostname[] = FIRMWARE_NAME"-000000";
    snprintf(hostname, sizeof(hostname), FIRMWARE_NAME"-%02X%02X%02X",
             heth.Init.MACAddr[3], heth.Init.MACAddr[4], heth.Init.MACAddr[5]);
    Stm32NetX::NX->getConfig()->hostname = hostname;
    Stm32NetX::NX->begin();
}

void new_connection(NX_TELNET_SERVER_STRUCT *telnet_server_ptr, UINT logical_connection) {
    Stm32ItmLogger::logger.setSeverity(Stm32ItmLogger::LoggerInterface::Severity::DEBUGGING)
            ->println("::new_connection()");
}

void receive_data(NX_TELNET_SERVER_STRUCT *telnet_server_ptr, UINT logical_connection, NX_PACKET *packet_ptr) {
    Stm32ItmLogger::logger.setSeverity(Stm32ItmLogger::LoggerInterface::Severity::DEBUGGING)
            ->println("::receive_data()");

    nx_packet_release(packet_ptr);
}

void connection_end(NX_TELNET_SERVER_STRUCT *telnet_server_ptr, UINT logical_connection) {
    Stm32ItmLogger::logger.setSeverity(Stm32ItmLogger::LoggerInterface::Severity::DEBUGGING)
            ->println("::connection_end()");
}

/**
 * @brief This function is the main loop that executes continuously.
 * The function is called inside the mainLoopThread().
 * @see mainLoopThread() in AZURE_RTOS/App/app_azure_rtos.c
 */
void loop() {
    static Stm32NetXTelnet::Server telnetServer;
    static UCHAR stackTelnet[2048];
    static Stm32Common::RunOnce roTelnet;

    if (Stm32NetX::NX->isIpSet()) {
        roTelnet.loop([]() {
            Stm32ItmLogger::logger.setSeverity(Stm32ItmLogger::LoggerInterface::Severity::INFORMATIONAL)
                    ->println("::loop() roTelnet");

            telnetServer.setLogger(&Logger);

            telnetServer.create(
                (char *) "Telnet Server",
                Stm32NetX::NX->getIpInstance(),
                stackTelnet,
                sizeof(stackTelnet)
            );


            // telnetServer.create(
            //     "Telnet Server",
            //     Stm32NetX::NX->getIpInstance(),
            //     stackTelnet,
            //     sizeof(stackTelnet),
            //     new_connection,
            //     receive_data,
            //     connection_end
            // );

            telnetServer.start();
        });

        telnetServer.loop();
    }

    static Stm32Common::RunEvery re1(3000);
    re1.loop([]() {
        // telnetServer.broadcast()->printf("counter = %d\r\n", dummyCpp);
    });

    static Stm32Common::RunEvery re2(300);
    re2.loop([]() {
        HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, dummyCpp & 1 ? GPIO_PIN_RESET : GPIO_PIN_SET);
        HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, dummyCpp & 2 ? GPIO_PIN_RESET : GPIO_PIN_SET);
        HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, dummyCpp & 4 ? GPIO_PIN_RESET : GPIO_PIN_SET);
        // Logger.printf("counter = %d\r\n", dummyCpp);
        dummyCpp++;
        dummyCandCpp++;
    });
}


/**
 * @brief This function handles fatal errors.
 * @see Error_Handler() in Core/Src/main.c
 */
[[noreturn]] void errorHandler() {
    for (;;) {
        //        for (uint32_t i = (SystemCoreClock / 10); i > 0; i--) { UNUSED(i); }
    }
}


[[noreturn]] void Stack_Error_Handler(TX_THREAD *thread_ptr) {
    Logger.print("==> Stack_Error_Handler() in ");
    Logger.println(thread_ptr->tx_thread_name);
    __disable_irq();
    for (;;) { ; }
}
