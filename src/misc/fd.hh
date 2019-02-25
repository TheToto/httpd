/**
 * \file misc/fd.hh
 * \brief FileDescriptor declaration.
 */
#pragma once

#include <fcntl.h>
#include <memory>
#include <unistd.h>
#include <utility>

#include "misc/sys-wrapper.hh"
#include "misc/unistd.hh"

namespace misc
{
    /**
     * \class FileDescriptor
     * \brief A resource object representing a fd.
     */
    struct FileDescriptor
    {
        /**
         * \brief Construct a FileDescriptor from a given fd.
         * \param fd int
         */
        FileDescriptor(int fd = -1)
            : fd_{fd}
        {}

        ~FileDescriptor()
        {
            if (*this)
                sys::close(fd_);
        }

        FileDescriptor(const FileDescriptor&) = delete;
        FileDescriptor& operator=(const FileDescriptor&) = delete;
        FileDescriptor(FileDescriptor&& fileDescriptor)
            : fd_{std::exchange(fileDescriptor.fd_, -1)}
        {}
        FileDescriptor& operator=(FileDescriptor&& fileDescriptor);

        /**
         * \brief Implicit conversion to int.
         */
        operator int() const&
        {
            return fd_;
        }

        /**
         * \brief Implicit conversion to bool.
         *
         * \return Whether or not the FileDescriptor holds a valid fd.
         */
        operator bool() const&
        {
            return fd_ != -1;
        }

        /**
         * \brief The fd held by the FileDescriptor object.
         */
        int fd_;
    };

    /**
     * \brief Alias to the smart pointer used to manage FileDescriptors
     * memory allocation.
     */
    using shared_fd = std::shared_ptr<FileDescriptor>;
} // namespace misc
