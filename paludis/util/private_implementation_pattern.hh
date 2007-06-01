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

#ifndef PALUDIS_GUARD_PALUDIS_PRIVATE_IMPLEMENTATION_PATTERN_HH
#define PALUDIS_GUARD_PALUDIS_PRIVATE_IMPLEMENTATION_PATTERN_HH 1

#include <paludis/util/instantiation_policy.hh>

/** \file
 * Declarations for the PrivateImplementationPattern pattern.
 *
 * \ingroup grppimp
 */

namespace paludis
{
    /**
     * Private implementation data, to be specialised for any class that
     * uses PrivateImplementationPattern.
     *
     * \ingroup grppimp
     */
    template <typename C_>
    struct Implementation;

    /**
     * A class descended from PrivateImplementationPattern has an associated
     * Implementation instance.
     *
     * \ingroup grppimp
     */
    template <typename C_>
    class PrivateImplementationPattern :
        private InstantiationPolicy<PrivateImplementationPattern<C_>, instantiation_method::NonCopyableTag>
    {
        protected:
            /**
             * Smart pointer to our implementation.
             *
             * \see PrivateImplementationPattern
             * \ingroup grppimp
             */
            class ImpPtr
            {
                private:
                    ImpPtr(const ImpPtr &);
                    void operator= (const ImpPtr &);
                    Implementation<C_> * _ptr;

                public:
                    ///\name Basic operations
                    ///\{

                    ImpPtr(Implementation<C_> * p);

                    ~ImpPtr();

                    ///\}

                    ///\name Dereference operators
                    //\{

                    Implementation<C_> * operator-> ();

                    const Implementation<C_> * operator-> () const;

                    ///\}

                    /**
                     * Reset to a new Implementation.
                     */
                    void reset(Implementation<C_> * p);
            };

            /**
             * Pointer to our implementation data.
             */
            ImpPtr _imp;

        public:
            /**
             * Constructor.
             */
            explicit PrivateImplementationPattern(Implementation<C_> * i);

            ~PrivateImplementationPattern();
    };
}

#endif
