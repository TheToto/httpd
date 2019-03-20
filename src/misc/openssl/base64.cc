#include "misc/openssl/base64.hh"

#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>

#include "error/init-error.hh"
#include "error/parsing-error.hh"

namespace ssl
{
    static size_t decode_length(const std::string& b64input)
    {
        size_t len = b64input.length();
        size_t padding = 0;

        if (b64input.find("==") == len - 2)
            padding = 2;
        else if (b64input.find("=") == len - 1)
            padding = 1;

        return (len * 3) / 4 - padding;
    }

    std::string base64_decode(const std::string& b64message)
    {
        BIO* bio;
        BIO* b64;

        auto decode_len = decode_length(b64message);

        std::string buffer(decode_len, '\0');

        bio = BIO_new_mem_buf(b64message.data(), -1);
        b64 = BIO_new(BIO_f_base64());
        if (!b64 || !bio)
        {
            BIO_free(bio);
            BIO_free(b64);
            throw http::InitializationError("Could not initialize BIO");
        }
        bio = BIO_push(b64, bio);

        BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
        const size_t length = BIO_read(bio, buffer.data(), b64message.length());

        if (decode_len != length)
            throw http::ParsingError("invalid base64 message");

        BIO_free_all(bio);

        return buffer;
    }

    std::string base64_encode(const std::string& buffer)
    {
        BIO* bio;
        BIO* b64;
        char* buffer_ptr;

        b64 = BIO_new(BIO_f_base64());
        bio = BIO_new(BIO_s_mem());
        if (!b64 || !bio)
        {
            BIO_free(bio);
            BIO_free(b64);
            throw http::InitializationError("Could not initialize BIO");
        }
        bio = BIO_push(b64, bio);

        BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
        BIO_write(bio, buffer.data(), buffer.length());
        BIO_flush(bio);

        const size_t len = BIO_get_mem_data(bio, &buffer_ptr);
        std::string encoded_string{buffer_ptr, len};

        BIO_free_all(bio);

        return encoded_string;
    }
} // namespace ssl
