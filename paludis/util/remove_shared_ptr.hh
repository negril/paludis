/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_REMOVE_SHARED_PTR_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_REMOVE_SHARED_PTR_HH 1

#include <tr1/memory>

namespace paludis
{
    /**
     * Like std::tr1::remove_pointer for std::tr1::shared_ptr.
     *
     * \ingroup g_utils
     */
    template <typename T_>
    struct RemoveSharedPtr
    {
        /// T_ with the std::tr1::shared_ptr removed.
        typedef T_ Type;
    };

    /**
     * Like std::tr1::remove_pointer for std::tr1::shared_ptr.
     *
     * \ingroup g_utils
     */
    template <typename T_>
    struct RemoveSharedPtr<std::tr1::shared_ptr<T_> >
    {
        /// T_ with the std::tr1::shared_ptr removed.
        typedef T_ Type;
    };

    /**
     * Like std::tr1::remove_pointer for std::tr1::shared_ptr.
     *
     * \ingroup g_utils
     */
    template <typename T_>
    struct RemoveSharedPtr<std::tr1::shared_ptr<const T_> >
    {
        /// T_ with the std::tr1::shared_ptr removed.
        typedef const T_ Type;
    };

    /**
     * Like std::tr1::remove_pointer for std::tr1::shared_ptr.
     *
     * \ingroup g_utils
     */
    template <typename T_>
    struct RemoveSharedPtr<const T_>
    {
        /// T_ with the std::tr1::shared_ptr removed.
        typedef const typename RemoveSharedPtr<T_>::Type Type;
    };

    /**
     * Like std::tr1::remove_pointer for std::tr1::shared_ptr.
     *
     * \ingroup g_utils
     */
    template <typename T_>
    struct RemoveSharedPtr<T_ &>
    {
        /// T_ with the std::tr1::shared_ptr removed.
        typedef typename RemoveSharedPtr<T_>::Type Type;
    };
}

#endif
