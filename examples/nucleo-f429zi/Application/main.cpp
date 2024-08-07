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
#include "globals.hpp"
#include "RunEvery.hpp"


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

    /*
    static char hostname[] = FIRMWARE_NAME"-000000";
    snprintf(hostname, sizeof(hostname), FIRMWARE_NAME"-%02X%02X%02X",
             heth.Init.MACAddr[3], heth.Init.MACAddr[4], heth.Init.MACAddr[5]);
    Stm32NetX::NX->getConfig()->hostname = hostname;
    Stm32NetX::NX->begin();
    */

}


/**
 * @brief This function is the main loop that executes continuously.
 * The function is called inside the mainLoopThread().
 * @see mainLoopThread() in AZURE_RTOS/App/app_azure_rtos.c
 */
void loop() {

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
