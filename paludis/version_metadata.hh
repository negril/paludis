/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#ifndef PALUDIS_GUARD_PALUDIS_VERSION_METADATA_HH
#define PALUDIS_GUARD_PALUDIS_VERSION_METADATA_HH 1

#include <paludis/name.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/version_metadata.hh>
#include <set>
#include <string>

namespace paludis
{
    /**
     * Represents a VersionMetadata key value.
     */
    enum VersionMetadataKey
    {
        vmk_depend,                 ///< DEPEND
        vmk_rdepend,                ///< RDEPEND
        vmk_slot,                   ///< SLOT
        vmk_src_uri,                ///< SRC_URI
        vmk_restrict,               ///< RESTRICT
        vmk_license,                ///< LICENSE
        vmk_licence = vmk_license,  ///< Convenience spelling for vmk_license
        vmk_keywords,               ///< KEYWORDS
        vmk_inherited,              ///< INHERITED
        vmk_iuse,                   ///< IUSE
        vmk_pdepend,                ///< PDEPEND
        vmk_provide,                ///< PROVIDE
        vmk_eapi,                   ///< EAPI
        vmk_homepage,               ///< HOMEPAGE
        vmk_description,            ///< DESCRIPTION
        vmk_virtual,                ///< Are we a virtual for something (empty is no)?
        vmk_e_keywords,             ///< KEYWORDS from ECLASS, for QA checks
        last_vmk                    ///< Number of items (keep at end!)
    };

    /**
     * Holds the metadata associated with a particular version.
     */
    class VersionMetadata :
        private InstantiationPolicy<VersionMetadata, instantiation_method::NonCopyableTag>,
        private PrivateImplementationPattern<VersionMetadata>,
        public InternalCounted<VersionMetadata>
    {
        public:
            /**
             * Constructor.
             */
            VersionMetadata();

            /**
             * Destructor.
             */
            ~VersionMetadata();

            /**
             * Fetch the value of a key.
             */
            const std::string & get(const VersionMetadataKey key) const;

            /// \name Convenience iterators
            ///{
            typedef std::set<UseFlagName>::const_iterator IuseIterator;
            IuseIterator begin_iuse() const;
            IuseIterator end_iuse() const;

            typedef std::set<KeywordName>::const_iterator KeywordIterator;
            KeywordIterator begin_keywords() const;
            KeywordIterator end_keywords() const;
            ///}

            /**
             * Set a key, return self.
             */
            VersionMetadata & set(const VersionMetadataKey key, const std::string & value);
    };
}

#endif
