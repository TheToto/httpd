#include "socket/ssl-socket.hh"

namespace http
{
    SSLSocket::SSLSocket(int domain, int type, int protocol, SSL_CTX* ssl_ctx)
        : Socket{std::make_shared<misc::FileDescriptor>(sys::socket(domain,
                                                                    type,
                                                                    protocol))}
    {}
}
