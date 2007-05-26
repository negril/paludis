/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License version 2, as published by the Free Software Foundation.
 *
 * Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef PALUDIS_GUARD_PALUDIS_INDIRECT_ITERATOR_HH
#define PALUDIS_GUARD_PALUDIS_INDIRECT_ITERATOR_HH 1

#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/operators.hh>

#include <iterator>
#include <functional>
#include <paludis/util/tr1_type_traits.hh>
#include <paludis/util/tr1_memory.hh>

/** \file
 * Declarations for various iterator helpers.
 *
 * \ingroup grpiterators
 */

namespace paludis
{
    /**
     * Return a new iterator pointing to the item after i.
     *
     * \ingroup grpiterators
     */
    template <typename T_>
    T_ next(const T_ & i)
    {
        T_ result(i);
        return ++result;
    }

     /**
     * Return a new iterator advanced by a distance of n.
     *
     * \ingroup grpiterators
     */
    template <typename T_>
    T_ next(const T_ & i, const int & n)
    {
        T_ result(i);
        std::advance(result, n);
        return result;
    }

    /**
     * Return a new iterator pointing to the item before i.
     *
     * \ingroup grpiterators
     */
    template <typename T_>
    T_ previous(const T_ & i)
    {
        T_ result(i);
        return --result;
    }

    /**
     * Like tr1::remove_pointer, for tr1::shared_ptr.
     *
     * \ingroup grpiterators
     */
    template <typename T_>
    struct RemoveSharedPointer
    {
        /// Our type, with tr1::shared_ptr removed.
        typedef T_ Type;
    };

    /**
     * Like tr1::remove_pointer, for tr1::shared_ptr.
     *
     * \ingroup grpiterators
     */
    template <typename T_>
    struct RemoveSharedPointer<tr1::shared_ptr<T_> >
    {
        /// Our type, with tr1::shared_ptr removed.
        typedef T_ Type;
    };

    /**
     * Like tr1::remove_pointer, for tr1::shared_ptr.
     *
     * \ingroup grpiterators
     */
    template <typename T_>
    struct RemoveSharedPointer<const tr1::shared_ptr<T_> >
    {
        /// Our type, with tr1::shared_ptr removed.
        typedef T_ Type;
    };

    template <typename Iter_, typename Value_ =
        typename RemoveSharedPointer<typename tr1::remove_pointer<typename Iter_::value_type>::type>::Type>
    class IndirectIterator;

    /**
     * An IndirectIterator is an iterator adapter that does one additional level
     * of dereferencing.
     *
     * \ingroup grpiterators
     * \nosubgrouping
     */
    template <typename Iter_, typename Value_>
    class IndirectIterator :
        public std::iterator<typename std::iterator_traits<Iter_>::iterator_category, Value_>,
        public relational_operators::HasRelationalOperators
    {
        template <typename A_, typename B_> friend bool operator< (
                const IndirectIterator<A_, B_> &, const IndirectIterator<A_, B_> &);

        template <typename A_, typename B_> friend bool operator== (
                const IndirectIterator<A_, B_> &, const IndirectIterator<A_, B_> &);

        private:
            Iter_ _i;

        public:
            ///\name Basic operations
            ///\{

            IndirectIterator(const Iter_ & i) :
                _i(i)
            {
            }

            IndirectIterator(const IndirectIterator & other) :
                _i(other._i)
            {
            }

            const IndirectIterator & operator= (const IndirectIterator & other)
            {
                _i = other._i;
                return *this;
            }

            ///\}

            ///\name Dereference operators
            ///\{

            Value_ & operator*() const
            {
                return **_i;
            }

            Value_ * operator->() const
            {
                return &**_i;
            }

            ///\}

            ///\name Increment, decrement operators
            ///\{

            IndirectIterator & operator++ ()
            {
                ++_i;
                return *this;
            }

            IndirectIterator operator++ (int)
            {
                IndirectIterator tmp(*this);
                ++_i;
                return tmp;
            }

            ///\}
    };

    template <typename Iter_, typename Value_>
    bool operator< (const IndirectIterator<Iter_, Value_> & a, const IndirectIterator<Iter_, Value_> & b)
    {
        return a._i < b._i;
    }

    template <typename Iter_, typename Value_>
    bool operator== (const IndirectIterator<Iter_, Value_> & a, const IndirectIterator<Iter_, Value_> & b)
    {
        return a._i == b._i;
    }

    /**
     * Convenience constructor for an IndirectIterator.
     *
     * \ingroup grpiterators
     */
    template <typename Iter_>
    IndirectIterator<Iter_,
        typename RemoveSharedPointer<typename tr1::remove_pointer<typename Iter_::value_type>::type>::Type>
    indirect_iterator(const Iter_ & i)
    {
        return IndirectIterator<Iter_,
            typename RemoveSharedPointer<typename tr1::remove_pointer<typename Iter_::value_type>::type>::Type>(i);
    }

    /**
     * A FilterInsertIterator is an insert iterator that only performs an insert
     * if a particular predicate function returns true for the object to be
     * inserted.
     *
     * \ingroup grpiterators
     * \nosubgrouping
     */
    template <typename Iter_, typename Pred_>
    class FilterInsertIterator :
        public std::iterator<typename std::iterator_traits<Iter_>::iterator_category, void, void, void, void>
    {
        private:
            Iter_ _i;
            Pred_ _p;

        public:
            /**
             * Fake a container_type for use with other iterator adapters.
             */
            typedef typename Iter_::container_type container_type;

            ///\name Basic operations
            ///\{

            FilterInsertIterator(const Iter_ & i, const Pred_ & p) :
                _i(i),
                _p(p)
            {
            }

            FilterInsertIterator(const FilterInsertIterator & other) :
                _i(other._i),
                _p(other._p)
            {
            }

            template <typename T_>
            const FilterInsertIterator & operator= (const T_ value)
            {
                if (_p(value))
                    *_i = value;
                return *this;
            }

            ~FilterInsertIterator();

            ///

            ///\name Dereference operators
            ///\{

            FilterInsertIterator & operator* ()
            {
                return *this;
            }

            FilterInsertIterator * operator-> ()
            {
                return this;
            }

            ///\}

            ///\name Increment, decrement operators
            ///\{

            FilterInsertIterator & operator++ ()
            {
                return *this;
            }

            FilterInsertIterator & operator++ (int)
            {
                return *this;
            }

            ///\}
    };

    template <typename Iter_, typename Pred_>
    FilterInsertIterator<Iter_, Pred_>::~FilterInsertIterator()
    {
    }

    /**
     * Convenience function: make a FilterInsertIterator.
     *
     * \ingroup grpiterators
     */
    template <typename Iter_, typename Pred_>
    FilterInsertIterator<Iter_, Pred_> filter_inserter(
            const Iter_ & i, const Pred_ & p)
    {
        return FilterInsertIterator<Iter_, Pred_>(i, p);
    }

    /**
     * A TransformInsertIterator is an insert iterator that calls some function
     * upon an item before inserting it.
     *
     * \ingroup grpiterators
     * \nosubgrouping
     */
    template <typename Iter_, typename Trans_>
    class TransformInsertIterator :
        public std::iterator<typename std::iterator_traits<Iter_>::iterator_category, void, void, void, void>
    {
        private:
            Iter_ _i;
            Trans_ _t;

        public:
            /**
             * Fake a container_type entry to allow a TransformInsertIterator to
             * work with other iterator adapters.
             */
            struct container_type
            {
                /// Our value type.
                typedef typename Trans_::argument_type value_type;
            };

            ///\name Basic operations
            ///\{
            TransformInsertIterator(const Iter_ & i, const Trans_ & t = Trans_()) :
                _i(i),
                _t(t)
            {
            }

            TransformInsertIterator(const TransformInsertIterator & other) :
                _i(other._i),
                _t(other._t)
            {
            }

            template <typename T_>
            const TransformInsertIterator & operator= (const T_ value)
            {
                *_i = _t(value);
                return *this;
            }

            ///\}

            ///\name Dereference operators
            ///\{

            TransformInsertIterator & operator* ()
            {
                return *this;
            }

            TransformInsertIterator * operator-> ()
            {
                return this;
            }

            ///\}

            ///\name Increment, decrement operators
            ///\{

            TransformInsertIterator & operator++ ()
            {
                return *this;
            }

            TransformInsertIterator & operator++ (int)
            {
                return *this;
            }

            ///\}

    };

    /**
     * Convenience function: make a TransformInsertIterator.
     *
     * \ingroup grpiterators
     */
    template <typename Iter_, typename Trans_>
    TransformInsertIterator<Iter_, Trans_> transform_inserter(
            const Iter_ & i, const Trans_ & t)
    {
        return TransformInsertIterator<Iter_, Trans_>(i, t);
    }

    /**
     * Convenience class: select a member of a class.
     *
     * \ingroup grpiterators
     */
    template <typename T_, typename M_, M_ T_::* m_>
    struct SelectMember :
        std::unary_function<T_, M_>
    {
        /// Carry out the selection.
        M_ operator() (const T_ & p) const
        {
            return p.*m_;
        }
    };

    /**
     * Convenience class: select the first item of a pair.
     *
     * \ingroup grpiterators
     */
    template <typename A_, typename B_>
    struct SelectFirst :
        SelectMember<std::pair<A_, B_>, A_, &std::pair<A_, B_>::first>
    {
    };

    /**
     * Convenience class: select the second item of a pair.
     *
     * \ingroup grpiterators
     */
    template <typename A_, typename B_>
    struct SelectSecond :
        SelectMember<std::pair<A_, B_>, B_, &std::pair<A_, B_>::second>
    {
    };

    /**
     * A CreateInsertIterator is an insert iterator that creates an object of
     * the specified type using the provided value.
     *
     * \ingroup grpiterators
     * \nosubgrouping
     */
    template <typename Iter_, typename Type_>
    class CreateInsertIterator :
        public std::iterator<typename std::iterator_traits<Iter_>::iterator_category, void, void, void, void>
    {
        private:
            Iter_ _i;

        public:
            /**
             * Fake a container_type to allow us to work with other iterator
             * adapters.
             */
            struct container_type
            {
                /// Our faked item type.
                typedef Type_ value_type;
            };

            ///\name Basic operations
            ///\{

            CreateInsertIterator(const Iter_ & i) :
                _i(i)
            {
            }

            CreateInsertIterator(const CreateInsertIterator & other) :
                _i(other._i)
            {
            }

            template <typename T_>
            const CreateInsertIterator & operator= (const T_ value)
            {
                *_i = Type_(value);
                return *this;
            }

            ///\}

            ///\name Dereference operators
            ///\{

            CreateInsertIterator & operator* ()
            {
                return *this;
            }

            CreateInsertIterator * operator-> ()
            {
                return this;
            }

            ///\}

            ///\name Increment, decrement operators
            ///\{

            CreateInsertIterator & operator++ ()
            {
                return *this;
            }

            CreateInsertIterator & operator++ (int)
            {
                return *this;
            }

            ///\}
    };

    /**
     * Convenience function: make a CreateInsertIterator.
     *
     * \ingroup grpiterators
     */
    template <typename Type_, typename Iter_>
    CreateInsertIterator<Iter_, Type_> create_inserter(const Iter_ & i)
    {
        return CreateInsertIterator<Iter_, Type_>(i);
    }
}

#endif
