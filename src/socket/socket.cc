#include "socket/socket.hh"

namespace http
{
    Socket::Socket(const misc::shared_fd& fd)
        : fd_{fd}
    {}
} // namespace http
