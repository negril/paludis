/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010, 2011, 2014 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_UNPACKAGED_INSTALLED_UNPACKAGED_ID_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_UNPACKAGED_INSTALLED_UNPACKAGED_ID_HH 1

#include <paludis/package_id.hh>
#include <paludis/name-fwd.hh>
#include <paludis/action-fwd.hh>
#include <paludis/util/pimp.hh>
#include <paludis/output_manager-fwd.hh>
#include <paludis/ndbam-fwd.hh>
#include <memory>

namespace paludis
{
    namespace unpackaged_repositories
    {
        class InstalledUnpackagedID :
            public PackageID,
            public std::enable_shared_from_this<InstalledUnpackagedID>
        {
            private:
                Pimp<InstalledUnpackagedID> _imp;

            protected:
                void need_keys_added() const override;
                void need_masks_added() const override;

            public:
                InstalledUnpackagedID(const Environment * const, const QualifiedPackageName &, const VersionSpec &,
                        const SlotName &, const RepositoryName &, const FSPath &, const std::string &, const FSPath &,
                        const NDBAM * const);

                ~InstalledUnpackagedID() override;

                const std::string canonical_form(const PackageIDCanonicalForm) const override;
                const QualifiedPackageName name() const override;
                const VersionSpec version() const override;
                const RepositoryName repository_name() const override;
                PackageDepSpec uniquely_identifying_spec() const override;

                const std::shared_ptr<const MetadataValueKey<Slot> > slot_key() const override;
                const std::shared_ptr<const MetadataCollectionKey<KeywordNameSet> > keywords_key() const override;
                const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > dependencies_key() const override;
                const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > build_dependencies_target_key() const override;
                const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > build_dependencies_host_key() const override;
                const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > run_dependencies_key() const override;
                const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > post_dependencies_key() const override;
                const std::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> > fetches_key() const override;
                const std::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> > homepage_key() const override;
                const std::shared_ptr<const MetadataValueKey<std::string> > short_description_key() const override;
                const std::shared_ptr<const MetadataValueKey<std::string> > long_description_key() const override;
                const std::shared_ptr<const MetadataTimeKey> installed_time_key() const override;
                const std::shared_ptr<const MetadataValueKey<FSPath> > fs_location_key() const override;
                const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > behaviours_key() const override;
                const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > from_repositories_key() const override;
                const std::shared_ptr<const MetadataValueKey<std::shared_ptr<const Choices> > > choices_key() const override;

                bool supports_action(const SupportsActionTestBase &) const override PALUDIS_ATTRIBUTE((warn_unused_result));
                void perform_action(Action &) const override;

                bool arbitrary_less_than_comparison(const PackageID &) const
                    override PALUDIS_ATTRIBUTE((warn_unused_result));
                std::size_t extra_hash_value() const
                    override PALUDIS_ATTRIBUTE((warn_unused_result));

                void uninstall(
                        const bool replace,
                        const std::shared_ptr<const PackageID> & if_for_install_id,
                        const std::shared_ptr<OutputManager> & output_manager) const;

                const std::shared_ptr<const Contents> contents() const override;
        };
    }
}

#endif
