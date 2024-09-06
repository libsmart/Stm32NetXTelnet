/*
 * SPDX-FileCopyrightText: 2024 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "LogicalConnectionMicrorl.hpp"
#include <climits>
#include <microrl.h>
#include "globals.hpp"
#include "Stm32GcodeRunner.hpp"

using namespace Stm32NetXTelnet;

LogicalConnectionMicrorl::LogicalConnectionMicrorl() : microrl() {
    // isConnectionActive = true;
}

LogicalConnectionMicrorl::~LogicalConnectionMicrorl() {
    getRxBuffer()->clear();
    getTxBuffer()->clear();
    microrl_t{};
}

void LogicalConnectionMicrorl::flush() {
    loop();
}

int LogicalConnectionMicrorl::microrlOutput(microrl *mrl, const char *str) {
    log(Stm32ItmLogger::LoggerInterface::Severity::DEBUGGING)
            ->println("Stm32NetXTelnet::LogicalConnection::microrlOutput()");

    write(str);
    return 0;
}

int LogicalConnectionMicrorl::microrlExec(microrl *mrl, int argc, const char *const *argv) {
    log(Stm32ItmLogger::LoggerInterface::Severity::INFORMATIONAL)
            ->println("Stm32NetXTelnet::LogicalConnection::microrlExec()");

    Logger.print("Tokens found: ");
    for (int i = 0; i < argc; i++) {
        Logger.printf("{%s} ", argv[i]);
    }

    auto parserRet = Stm32GcodeRunner::parser->parseArgcArgv(cmd, argc, argv);

    if (parserRet == Stm32GcodeRunner::Parser::parserReturn::OK) {
        log(Stm32ItmLogger::LoggerInterface::Severity::NOTICE)
                ->printf("Found command: %s\r\n", cmd->getName());
        Stm32GcodeRunner::CommandContext *cmdCtx{};
        Stm32GcodeRunner::worker->createCommandContext(cmdCtx);
        if (cmdCtx == nullptr) {
            println("ERROR: Command buffer full");
            return 1;
        }
        cmdCtx->setCommand(cmd);

        cmdCtx->registerOnWriteFunction([cmdCtx, this]() {
            // Debugger_log(DBG, "onWriteFn()");
            if (cmdCtx->outputLength() > 0) {
                const auto result = cmdCtx->outputRead(
                    reinterpret_cast<char *>(this->getTxBuffer()->getWritePointer()),
                    this->getTxBuffer()->getRemainingSpace());
                this->getTxBuffer()->setWrittenBytes(result);
            }
        });

        cmdCtx->registerOnCmdEndFunction([cmdCtx, this]() {
            // Debugger_log(DBG, "onCmdEndFn()");
            cmd = nullptr;
            Stm32GcodeRunner::worker->deleteCommandContext(cmdCtx);
        });

        Stm32GcodeRunner::worker->enqueueCommandContext(cmdCtx);
    } else if (parserRet == Stm32GcodeRunner::Parser::parserReturn::UNKNOWN_COMMAND) {
        println("ERROR: UNKNOWN COMMAND");
    } else if (parserRet == Stm32GcodeRunner::Parser::parserReturn::GARBAGE_STRING) {
        println("ERROR: UNKNOWN COMMAND");
    } else {
        // txBuffer.println("ERROR: ONLY WHITESPACE");
    }

    return 0;
}

void LogicalConnectionMicrorl::setup() {
    log(Stm32ItmLogger::LoggerInterface::Severity::INFORMATIONAL)
            ->println("Stm32NetXTelnet::LogicalConnection::setup()");

    /* Initialize library with microrl instance and print and execute callbacks */
    auto ret = microrl_init(this,
                            bounce<LogicalConnectionMicrorl, decltype(&LogicalConnectionMicrorl::microrlOutput),
                                &LogicalConnectionMicrorl::microrlOutput, const char *>,
                            bounce<LogicalConnectionMicrorl, decltype(&LogicalConnectionMicrorl::microrlExec),
                                &LogicalConnectionMicrorl::microrlExec, int, const char *const *>
    );
    if (ret != microrlOK) {
        log(Stm32ItmLogger::LoggerInterface::Severity::ERROR)
                ->printf("microrl_init() = 0x%02x\r\n", ret);
    }
    // userdata_ptr = this;

#if MICRORL_CFG_USE_COMPLETE
    /* Set callback for auto-completion */
    microrl_set_complete_callback(this, bounce<LogicalConnection, decltype(&LogicalConnection::microrlComplete),
                                &LogicalConnection::microrlComplete, int, const char *const *>);
#endif

#if MICRORL_CFG_USE_CTRL_C
    /* Set callback for Ctrl+C handling */
    microrl_set_sigint_callback(this, bounce<LogicalConnection, decltype(&LogicalConnection::microrlSigint),
                                &LogicalConnection::microrlSigint>);
#endif

    microrl_set_prompt(this, (char *) "");


    println();
    print(FIRMWARE_NAME);
    print(F(" v"));
    print(FIRMWARE_VERSION);
    print(F(" "));
    println(FIRMWARE_COPY);
    flush();
    delay(500);
    print(F("OK"));
    flush();

    microrl_processing_input(this, "\n", 1);
}

void LogicalConnectionMicrorl::loop() {
    if (cmd != nullptr) return;
    while (available() > 0) {
        auto ch = read();

        if (iac == 0 && ch == 0xff) {
            // Enable IAC mode
            iac++;
            continue;
        }

        if (iac == 1 && ch == 0xff) {
            // Second IAC marks a real 0xff byte
            iac = 0;
        }

        if (iac > 0) {
            // 1 byte commands
            if (iac == 1 && ch >= 0xf0 && ch <= 0xf9) {
                iacCmd = ch;
                iac = 0;
                iacCmd = 0;
            }

            // 2 byte commands
            if (iac == 1 && ch >= 0xfb && ch <= 0xfe) {
                iacCmd = ch;
                iac++;
            }
            if (iac == 2 && iacCmd >= 0xfb && iacCmd <= 0xfe) {
                iac = 0;
                iacCmd = 0;
            }

            // multi byte commands
            if (iac == 1 && ch == 0xfa) {
                iacCmd = ch;
                iac++;
            }
            // multi byte commands end
            if (iac > 2 && ch == 0xf0) {
                iac = 0;
                iacCmd = 0;
            }
        } else {
            // Send character to microrl, if not in IAC mode
            auto ret = microrl_processing_input(this, &ch, 1);
            if (ret != microrlOK) {
                log(Stm32ItmLogger::LoggerInterface::Severity::ERROR)
                        ->printf("microrl_processing_input() = 0x%02x\r\n", ret);
            }
        }
    }
}

void LogicalConnectionMicrorl::end() {
    log(Stm32ItmLogger::LoggerInterface::Severity::INFORMATIONAL)
        ->println("Stm32NetXTelnet::LogicalConnection::connectionEnd()");

    // isConnectionActive = false;
    getRxBuffer()->clear();
    getTxBuffer()->clear();
    if (cmd != nullptr) {
        auto cmdCtx = cmd->getCommandContext();
        Stm32GcodeRunner::WorkerDynamic::terminateCommandContext(cmdCtx);
    }

    // isConnectionActive = false;
    microrl_t{};
    cmd = nullptr;
    iac = 0;
    iacCmd = 0;
}
