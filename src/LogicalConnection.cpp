/*
 * SPDX-FileCopyrightText: 2024 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "LogicalConnection.hpp"
#include <climits>

size_t Stm32NetXTelnet::LogicalConnection::getWriteBuffer(uint8_t *&buffer) {
    buffer = txBuffer.getWritePointer();
    return txBuffer.getRemainingSpace();
}

size_t Stm32NetXTelnet::LogicalConnection::setWrittenBytes(size_t size) {
    return txBuffer.add(size);
}

size_t Stm32NetXTelnet::LogicalConnection::write(uint8_t data) {
    if (txBuffer.write(data) == 1) {
        return 1;
    }
    return 0;
}

int Stm32NetXTelnet::LogicalConnection::availableForWrite() {
    return txBuffer.getRemainingSpace() > INT_MAX ? INT_MAX : static_cast<int>(txBuffer.getRemainingSpace());
}

void Stm32NetXTelnet::LogicalConnection::flush() {
    loop();
}

int Stm32NetXTelnet::LogicalConnection::available() {
    return rxBuffer.getLength() > INT_MAX ? INT_MAX : static_cast<int>(rxBuffer.getLength());
}

int Stm32NetXTelnet::LogicalConnection::read() {
    return rxBuffer.read();
}

int Stm32NetXTelnet::LogicalConnection::peek() {
    return rxBuffer.peek();
}

void Stm32NetXTelnet::LogicalConnection::loop() {
    while (available() > 0) {
        write(read());
    }
}
