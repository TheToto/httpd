/**
 * \file events/events.hh
 * \brief EvenWatcher declaration.
 */

#pragma once

#include <ev.h>
#include <iostream>

namespace http
{
    /**
     * \class EventWatcher
     * \brief Abstract class representing event watchers.
     *
     * Its subclasses will act as functors and implement most of the server's
     * workflow. Every potentially blocking operation should be represented by
     * an EventWatcher.
     */
    class EventWatcher
    {
    public:
        /**
         * \brief Initialize EventWatcher's ev_io.
         *
         * Initialize the ev_io held by the EventWatcher and store the
         * EventWatcher's address in it.
         *
         * \param fd int socket fd.
         * \param flags int initialization flags.
         */
        EventWatcher(int fd, int flags)
        {
            watcher_ = ev_io();
            ev_init(&watcher_, event_callback);
            ev_io_set(&watcher_, fd, flags);
        }

        EventWatcher(const EventWatcher&) = delete;
        EventWatcher& operator=(const EventWatcher&) = delete;
        EventWatcher(EventWatcher&&) = delete;
        EventWatcher& operator=(EventWatcher&&) = delete;
        virtual ~EventWatcher() = default;

        /**
         * \brief Virtual pure functor
         *
         * Each EventWatcher deriving from this class will need to implement
         * its own workflow by overriding this functor.
         */
        virtual void operator()() = 0;

        ev_io& watcher_get() noexcept
        {
            return watcher_;
        }

    protected:
        /**
         * \brief Callback for event handling.
         *
         * Each time an event occurs in the loop, this callback will be called
         * with its associated ev_io. This method is called by libev and uses
         * dynamic dispatch to run the correct overload of the operator().
         *
         * \param watcher ev_io* which received an event.
         */
        static void event_callback(struct ev_loop* loop, ev_io* w, int revents)
        {
            std::cout << "plop";
        }

        /**
         * \brief Libev's io watcher.
         */
        ev_io watcher_;
    };
} // namespace http
