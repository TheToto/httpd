#include "socket/ssl-socket.hh"

#include "misc/openssl/ssl.hh"
#define BUFSIZE 8192
namespace http
{
    SSLSocket::SSLSocket(const misc::shared_fd& fd, SSL_CTX* ssl_ctx)
        : Socket{fd}
    {
        ssl_ = std::shared_ptr<SSL>(SSL_new(ssl_ctx), SSL_free);
        SSL_set_fd(ssl_.get(), *fd_);

        ctx_ = ssl_ctx;
    }

    SSLSocket::SSLSocket(int domain, int type, int protocol, SSL_CTX* ssl_ctx)
        : Socket{std::make_shared<misc::FileDescriptor>(
              sys::socket(domain, type, protocol))}
    {
        ssl_ = std::shared_ptr<SSL>(SSL_new(ssl_ctx), SSL_free);
        SSL_set_fd(ssl_.get(), *fd_);

        ctx_ = ssl_ctx;
    }

    ssize_t SSLSocket::recv(void* dst, size_t len)
    {
        return ssl::read(ssl_.get(), dst, len);
        //return SSL_read(ssl_.get(), dst, len);
        //return sys::recv(*fd_, dst, len, 0);
    }

    ssize_t SSLSocket::send(const void* src, size_t len)
    {
        return ssl::write(ssl_.get(), src, len);
        //return SSL_write(ssl_.get(), src, len);
        //return sys::send(*fd_, src, len, 0);
    }

    ssize_t SSLSocket::sendfile(misc::shared_fd& src, off_t& offset,
                                size_t count)
    {
        if (count > BUFSIZE)
            count = BUFSIZE;
        char buffer[BUFSIZE];
        // Peek data
        off_t old = sys::lseek(*src, offset, SEEK_CUR);
        sys::lseek(*src, offset, SEEK_SET);
        int peeked = sys::read(*src, buffer, count);
        sys::lseek(*src, old, SEEK_SET);
        // Send data (as much as possible)
        int sended = ssl::write(ssl_.get(), buffer, peeked);
        // Return number of bytes sended
        offset += sended;
        return sended;
        //return sys::sendfile(*fd_, *fd, &offset, len);
    }

    void SSLSocket::bind(const sockaddr* addr, socklen_t addrlen)
    {
        sys::bind(*fd_, addr, addrlen);
    }

    void SSLSocket::listen(int backlog)
    {
        sys::listen(*fd_, backlog);
    }

    void SSLSocket::setsockopt(int level, int optname, int optval)
    {
        sys::setsockopt(*fd_, level, optname, &optval, sizeof(int));
    }

    shared_socket SSLSocket::accept(sockaddr* addr, socklen_t* addrlen)
    {
        auto fd = std::make_shared<misc::FileDescriptor>(
            sys::accept(*fd_, addr, addrlen));

        ssl::accept(ssl_.get());
        //SSL_accept(ssl_.get());

        auto s = new SSLSocket(fd, ctx_);
        return shared_socket(s);
    }

    void SSLSocket::connect(const sockaddr* addr, socklen_t len)
    {
        sys::connect(*fd_, addr, len);
        ssl::connect(ssl_.get());
        //SSL_connect(ssl_.get());
    }

} // namespace http
