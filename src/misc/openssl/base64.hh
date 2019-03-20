/**
 * \file misc/openssl/base64.hh
 * \brief Function to encode and decode base64 messages.
 */

#pragma once

#include <string>

namespace ssl
{
    /**
     * \brief Decode a base64 message.
     *
     * \param message encoded in base64.
     * \return The decoded message.
     */
    std::string base64_decode(const std::string& b64message);

    /**
     * \brief Encode a string in base64.
     *
     * \param buffer that needs to be encoded.
     * \return The encoded message.
     */
    std::string base64_encode(const std::string& buffer);
} // namespace ssl
