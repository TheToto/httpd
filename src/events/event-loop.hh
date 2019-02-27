/**
 * \file events/event-loop.hh
 * \brief EventLoop declaration.
 */

#pragma once

#include <ev.h>

#include "events/events.hh"

namespace http
{
    class EventWatcher;

    /**
     * \struct EventLoop
     * \brief Value object wrapping libev's ev_loop.
     *
     * Event loop design pattern.
     */
    struct EventLoop
    {
        /**
         * \brief Create a default EventLoop based on a default ev_loop.
         */
        EventLoop() = delete;

        /**
         * \brief Create an EventLoop from an existing ev_loop.
         *
         * \param loop ev_loop* custom ev_loop.
         */
        explicit EventLoop(struct ev_loop* ev_loop)
        {
            loop = ev_loop;
        }

        EventLoop(const EventLoop&) = default;
        EventLoop& operator=(const EventLoop&) = default;
        EventLoop(EventLoop&&) = default;
        EventLoop& operator=(EventLoop&&) = default;

        /**
         * \brief Destroy the ev_loop.
         */
        ~EventLoop() = default;

        /**
         * \brief Activate the given ev_io.
         *
         * Note that only activated watchers will receive events.
         *
         * \param watcher EventWatcher* to register in the loop.
         */
        void register_watcher(EventWatcher* ev)
        {
            ev_io_start(loop, &ev->watcher_get());
        }

        /**
         * \brief Stop the given ev_io.
         *
         * \param watcher EventWatcher* to unregister in the loop.
         */
        void unregister_watcher(EventWatcher* ev)
        {
            ev_io_stop(loop, &ev->watcher_get());
        }

        /**
         * \brief Register SIGINT ev_signal.
         *
         * \param watcher ev_signal* to register in the loop.
         */
        void register_sigint_watcher(ev_signal* sig) const
        {
            ev_signal_start(loop, sig);
        }

        /**
         * \brief Start waiting for events.
         */
        void operator()() const
        {
            ev_run(loop, 0);
        }

        /**
         * \brief Libev's event loop.
         */
        struct ev_loop* loop;
    };

} // namespace http
