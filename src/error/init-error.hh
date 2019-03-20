#pragma once

#include <stdexcept>
#include <string>

namespace http
{
    struct InitializationError : public std::logic_error
    {
        explicit InitializationError(const std::string& what_arg)
            : std::logic_error{"Initialization error: " + what_arg}
        {}
        virtual ~InitializationError() = default;
    };
} // namespace http
