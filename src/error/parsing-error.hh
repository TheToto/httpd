#pragma once

#include <stdexcept>
#include <string>

namespace http
{
    struct ParsingError : public std::logic_error
    {
        explicit ParsingError(const std::string& what_arg)
            : std::logic_error{"Parsing error: " + what_arg}
        {}
        virtual ~ParsingError() = default;
    };
} // namespace http
