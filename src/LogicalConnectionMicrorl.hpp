/*
 * SPDX-FileCopyrightText: 2024 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef LIBSMART_STM32NETXTELNET_LOGICALCONNECTIONMICRORL_HPP
#define LIBSMART_STM32NETXTELNET_LOGICALCONNECTIONMICRORL_HPP

#include <microrl.h>
#include "AbstractCommand.hpp"
#include "Loggable.hpp"
#include "Nameable.hpp"
#include "StreamRxTx.hpp"

namespace Stm32NetXTelnet {
    /**
     * @brief Bounce Function
     *
     * This template function acts as a bounce function to call a member function (Method) on an object (T) passed as a
     * pointer using the input thread_input. It forwards any additional parameters to the member function call.
     *
     * @tparam T The class type of the object on which the member function will be called
     * @tparam Method The type of the member function
     * @tparam m Pointer to the member function
     * @tparam Params The types of additional parameters to forward to the member function call
     *
     * @param thread_input The integral value representing the pointer to the object on which the member function will be called
     * @param params The additional parameters to be forwarded to the member function
     *
     * @return The return value of the member function call
     *
     * @note The caller is responsible for ensuring that thread_input represents a valid pointer to an object of type T
     * and the member function pointed to by m is of correct signature and can be called with the provided parameters.
     */
    template<class T, class Method, Method m, class... Params>
    static auto bounce(microrl *mrl, Params... params) ->
        decltype(((*reinterpret_cast<T *>(mrl->userdata_ptr)).*m)(mrl, params...)) {
        assert_param(mrl != nullptr);
        assert_param(mrl->userdata_ptr != nullptr);
        return ((*reinterpret_cast<T *>(mrl->userdata_ptr)).*m)(mrl, params...);
    }

    class Server;

    class LogicalConnectionMicrorl : public Stm32Common::Nameable, public Stm32ItmLogger::Loggable,
                              public Stm32Common::StreamRxTx<
                                  LIBSMART_STM32NETXTELNET_BUFFER_SIZE_RX,
                                  LIBSMART_STM32NETXTELNET_BUFFER_SIZE_TX> {
    public:
        friend Server;

        void flush() override;

        int microrlOutput(microrl *mrl, const char *str);

        int microrlExec(microrl *mrl, int argc, const char *const *argv);

        char **microrlComplete(microrl *mrl, int argc, const char *const *argv);

        void microrlSigint(microrl *mrl);

        virtual void setup();

        virtual void loop();

    private:
        microrl_t mrl{};
        Stm32GcodeRunner::AbstractCommand *cmd{};
    };
}

#endif
