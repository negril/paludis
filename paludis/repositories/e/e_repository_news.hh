/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_E_REPOSITORY_NEWS_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_E_REPOSITORY_NEWS_HH 1

#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/config_file.hh>

/** \file
 * Declaration for the ERepositoryNews class.
 *
 * \ingroup grperepository
 */

namespace paludis
{
    class Environment;
    class FSEntry;
    class ERepository;

    /**
     * Holds the news/ data for a ERepository instance.
     *
     * \ingroup grperepository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE ERepositoryNews :
        private PrivateImplementationPattern<ERepositoryNews>,
        private InstantiationPolicy<ERepositoryNews, instantiation_method::NonCopyableTag>
    {
        public:
            ///\name Basic operations
            ///\{

            ERepositoryNews(const Environment * const, const ERepository * const,
                    const ERepositoryParams &);
            ~ERepositoryNews();

            ///\}

            void update_news() const;
    };

    /**
     * A NewsFile represents a GLEP 42 news file.
     *
     * \ingroup grpnewsconfigfile
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE NewsFile :
        private PrivateImplementationPattern<NewsFile>
    {
        public:
            ///\name Basic operations
            ///\{

            /**
             * Constructor, from a filename.
             */
            NewsFile(const FSEntry & filename);

            ~NewsFile();

            ///\}

            ///\name Iterate over our Display-If-Installed headers
            ///\{

            /// Tag for DisplayIfInstalledConstIterator.
            struct DisplayIfInstalledConstIteratorTag;

            typedef libwrapiter::ForwardIterator<DisplayIfInstalledConstIteratorTag,
                    const std::string> DisplayIfInstalledConstIterator;

            DisplayIfInstalledConstIterator begin_display_if_installed() const;

            DisplayIfInstalledConstIterator end_display_if_installed() const;

            ///\}

            ///\name Iterate over our Display-If-Keyword headers
            ///\{

            /// Tag for DisplayIfKeywordConstIterator.
            struct DisplayIfKeywordConstIteratorTag;

            typedef libwrapiter::ForwardIterator<DisplayIfKeywordConstIteratorTag,
                    const std::string> DisplayIfKeywordConstIterator;

            DisplayIfKeywordConstIterator begin_display_if_keyword() const;

            DisplayIfKeywordConstIterator end_display_if_keyword() const;

            ///\}

            ///\name Iterate over our Display-If-Profile headers
            ///\{

            /// Tag for DisplayIfProfileConstIterator.
            struct DisplayIfProfileConstIteratorTag;

            typedef libwrapiter::ForwardIterator<DisplayIfProfileConstIteratorTag,
                    const std::string> DisplayIfProfileConstIterator;

            DisplayIfProfileConstIterator begin_display_if_profile() const;

            DisplayIfProfileConstIterator end_display_if_profile() const;

            ///\}
    };
}

#endif
