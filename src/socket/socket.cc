#include "socket/socket.hh"

namespace http
{
    Socket::Socket(const misc::shared_fd& fd)
        : fd_{fd}
    {
        SSL_load_error_strings(); // FIXME_G mettre en global
        OpenSSL_add_ssl_algorithms(); // FIXME_G mettre en global

        const SSL_METHOD *method;

        method = SSLv23_server_method(); // FIXME_G ? dépend de la méthode
        ssl_ctx_ = SSL_CTX_new(method);

        if (!ssl_ctx_)
        {
            perror("Unable to create SSL context");
            ERR_print_errors_fp(stderr);
            exit(EXIT_FAILURE);
        }

        SSL_CTX_set_ecdh_auto(ssl_ctx_, 1);

        //FIXME_L où trouver la config? go la foutre en argument?
        const char *cert_file = std::string("tests/ssl/localhost.pem").c_str();
        const char *key_file = std::string("tests/ssl/localhost-key.pem").c_str();

        /* Set the key and cert */
        //FIXME_G check X509, check_private_key, etc...
        if (SSL_CTX_use_certificate_file(ssl_ctx_, cert_file, SSL_FILETYPE_PEM) <= 0 ||
            SSL_CTX_use_PrivateKey_file(ssl_ctx_, key_file, SSL_FILETYPE_PEM) <= 0)
        {
            ERR_print_errors_fp(stderr);
            exit(EXIT_FAILURE);
        }
    }
} // namespace http
