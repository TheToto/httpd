/**
 * \file misc/unistd.hh
 * \brief unistd.h syscall wrappers.
 */

#pragma once

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "misc/fd.hh"
#include "misc/sys-wrapper.hh"

namespace sys
{
    /**
     * \brief close(2).
     */
    inline auto close = make_wrapper<void>(::close);

    /**
     * \brief fstat(2).
     */
    inline auto fstat = make_wrapper<int>(::fstat);

    static int open_wrapper(const char* path, int flags)
    {
        return ::open(path, flags);
    }
    /**
     * \brief open(2).
     *
     * Since open is a variadic syscall we need to call a helper with a
     * fixed number of parameters to create a wrapper out of it.
     */
    inline auto open = make_wrapper<int>(open_wrapper);
} // namespace sys
