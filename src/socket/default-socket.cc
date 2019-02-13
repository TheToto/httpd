#include "socket/default-socket.hh"

#include "misc/socket.hh"
#include "misc/fd.hh"
#include <memory>

namespace http
{
    DefaultSocket::DefaultSocket(const misc::shared_fd& fd)
        : Socket{fd}
    {}

    DefaultSocket::DefaultSocket(int domain, int type, int protocol)
        : Socket{std::make_shared<misc::FileDescriptor>(
                sys::socket(domain, type, protocol))}
    {}

    ssize_t DefaultSocket::recv(void* dst, size_t len)
    {
        return sys::recv(*fd_, dst, len, 0);
    }

    ssize_t DefaultSocket::send(const void* src, size_t len)
    {
        return sys::send(*fd_, src, len, 0);
    }

    ssize_t DefaultSocket::sendfile(misc::shared_fd& fd, off_t& offset, size_t len)
    {
        return sys::sendfile(*fd_, *fd, &offset, len);
    }

    void DefaultSocket::bind(const sockaddr* addr, socklen_t addrlen)
    {
        sys::bind(*fd_, addr, addrlen);
    }

    void DefaultSocket::listen(int backlog)
    {
        sys::listen(*fd_, backlog);
    }

    void DefaultSocket::setsockopt(int level, int optname, int optval)
    {
        sys::setsockopt(*fd_, level, optname, &optval, sizeof(int));
    }

    shared_socket DefaultSocket::accept(sockaddr* addr, socklen_t* addrlen)
    {
        auto fd = std::make_shared<misc::FileDescriptor>(sys::accept(*fd_, addr, addrlen));
        auto s = new DefaultSocket(fd);
        return shared_socket(s);
    }

    void DefaultSocket::connect(const sockaddr* addr, socklen_t len)
    {
        sys::connect(*fd_, addr, len);
    }

} // namespace http
