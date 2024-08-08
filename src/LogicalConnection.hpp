/*
 * SPDX-FileCopyrightText: 2024 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef LIBSMART_STM32NETXTELNET_LOGICALCONNECTION_HPP
#define LIBSMART_STM32NETXTELNET_LOGICALCONNECTION_HPP

#include "Loggable.hpp"

namespace Stm32NetXTelnet {
    class Server;

    class LogicalConnection : public Stm32ItmLogger::Loggable, public Stm32Common::Stream {
    public:
        friend Server;

        size_t getWriteBuffer(uint8_t *&buffer) override;

        size_t setWrittenBytes(size_t size) override;

        size_t write(uint8_t data) override;

        int availableForWrite() override;

        void flush() override;

        int available() override;

        int read() override;

        int peek() override;

        virtual void setup() { ; }

        virtual void loop();

    private:
        Stm32Common::StringBuffer<LIBSMART_STM32NETXTELNET_BUFFER_SIZE_RX> rxBuffer{};
        Stm32Common::StringBuffer<LIBSMART_STM32NETXTELNET_BUFFER_SIZE_TX> txBuffer{};
    };
}

#endif
