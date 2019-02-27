/**
 * /file misc/addrinfo/addrinfo-iterator.hh
 * /brief Iterator over AddrInfo declaration.
 */
#pragma once

#include <boost/iterator/iterator_facade.hpp>
#include <netdb.h>

#include "error/not-implemented.hh"

namespace misc
{
    /**
     * \class addrinfo_iterator
     * \brief Iterator over AddrInfo objects.
     */
    template <class Value>
    class addrinfo_iter
        : public boost::iterator_facade<addrinfo_iter<Value>, Value,
                                        boost::forward_traversal_tag>
    {
    public:
        /**
         * \brief Default constructor (end iterator).
         */
        addrinfo_iter()
            : m_node(nullptr)
        {}

        /**
         * \brief Constructor from a given AddrInfo.
         * \param p Value* the AddrInfo
         */
        explicit addrinfo_iter(Value* p)
            : m_node(p)
        {}

        /**
         * \brief Begin of the iterator range.
         */
        addrinfo_iter<Value> begin()
        {
            return *this;
        }

        /**
         * \brief End of the iterator range.
         */
        addrinfo_iter<Value> end()
        {
            return addrinfo_iter<Value>{};
        }

    private:
        friend class boost::iterator_core_access;

        /**
         * \brief Advance the iterator by one step.
         */
        void increment()
        {
            m_node = m_node->ai_next;
        }

        /**
         * \brief Check equality between iterators.
         */
        bool equal(const addrinfo_iter<Value>& other) const
        {
            return this->m_node == other.m_node;
        }

        /**
         * \brief Access to the value through dereferenciation
         */
        Value& dereference() const
        {
            return *m_node;
        }

        /**
         * \brief Begin of a given iterator.
         */
        Value& begin(const addrinfo_iter<Value>& n)
        {
            return n->m_node;
        }

        /**
         * \brief Current value of the iterator.
         */
        Value* m_node;
    };

    /**
     * \brief Alias to AddrInfo iterators type.
     */
    using addrinfo_iterator = addrinfo_iter<addrinfo>;
    /**
     * \brief Alias to const AddrInfo iterators type.
     */
    using addrinfo_const_iterator = addrinfo_iter<const addrinfo>;
} // namespace misc
