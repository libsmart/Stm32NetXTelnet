/*
 * SPDX-FileCopyrightText: 2024 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef LIBSMART_STM32NETXTELNET_LOGICALCONNECTIONMICRORL_HPP
#define LIBSMART_STM32NETXTELNET_LOGICALCONNECTIONMICRORL_HPP

#include <microrl.h>
#include <StreamSession/StreamSessionInterface.hpp>
#include "AbstractCommand.hpp"
#include "Loggable.hpp"
#include "Nameable.hpp"
#include "StreamRxTx.hpp"

namespace Stm32NetXTelnet {

    class Server;

    class LogicalConnectionMicrorl : protected microrl_t,
                                     public Stm32Common::StreamSession::StreamSessionInterface,
                                     public Stm32Common::StreamRxTx<
                                         LIBSMART_STM32NETXTELNET_BUFFER_SIZE_RX,
                                         LIBSMART_STM32NETXTELNET_BUFFER_SIZE_TX> {
    public:
        friend Server;

        LogicalConnectionMicrorl();

        ~LogicalConnectionMicrorl() override;

        /**
         * @brief Flush the current connection buffer
         *
         * This method triggers the `loop()` function to process any pending tasks or data
         * in the buffer associated with a LogicalConnectionMicrorl instance.
         *
         * @note This method is typically used to ensure all data is processed and the
         * connection state is up to date.
         */
        void flush() override;

        int microrlOutput(microrl *mrl, const char *str);

        int microrlExec(microrl *mrl, int argc, const char *const *argv);

        char **microrlComplete(microrl *mrl, int argc, const char *const *argv) { return nullptr; }

        void microrlSigint(microrl *mrl) { ; }

        void setup() override;

        void loop() override;

        void end() override;

    private:
        // bool isConnectionActive = false;
        Stm32GcodeRunner::AbstractCommand *cmd{};
        uint8_t iac = 0;
        uint8_t iacCmd = 0;

    protected:
        template<class T, class Method, Method m, class... Params>
        /**
         * @brief Bounce Function
         *
         * This template function calls a member function (Method) on an object (T) passed through a microrl pointer.
         * It verifies that the given microrl pointer and the session cast from it are valid before invoking the member function.
         *
         * @tparam T The class type of the object on which the member function will be called
         * @tparam Method The type of the member function
         * @tparam m Pointer to the member function
         * @tparam Params The types of additional parameters to forward to the member function call
         *
         * @param mrl A pointer to a microrl object representing the session
         * @param params The additional parameters to be forwarded to the member function
         *
         * @return The return value of the member function call
         *
         * @note The caller is responsible for ensuring that the microrl pointer represents a valid session and the member function
         *       pointed to by m is of the correct signature and can be called with the provided parameters.
         */
        static auto bounce(microrl *mrl, Params... params) ->
            decltype(((*static_cast<T *>(mrl)).*m)(mrl, params...)) {
            assert_param(mrl != nullptr);
            T *session = static_cast<T *>(mrl);
            assert_param(session != nullptr);
            return ((*static_cast<T *>(session)).*m)(mrl, params...);
        }
    };
}

#endif
