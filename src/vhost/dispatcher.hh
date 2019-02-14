/**
 * \file vhost/dispatcher.hh
 * \brief Dispatcher declaration.
 */

#pragma once

#include <vector>
#include "vhost/vhost.hh"

namespace http
{
    /**
     * \class Dispatcher
     * \brief Instance in charge of dispatching requests between vhosts.
     */
    class Dispatcher
    {
    public:
        Dispatcher() = default;
        Dispatcher(const Dispatcher&) = delete;
        Dispatcher& operator=(const Dispatcher&) = delete;
        Dispatcher(Dispatcher&&) = delete;
        Dispatcher& operator=(Dispatcher&&) = delete;

        void register_vhost(shared_vhost vhost)
        {
            vhosts_.push_back(vhost);
        }
        shared_vhost operator()(Request& r);

    private:
        std::vector<shared_vhost> vhosts_;
    };

    /**
     * \brief Service object.
     */
    extern Dispatcher dispatcher;
} // namespace http
