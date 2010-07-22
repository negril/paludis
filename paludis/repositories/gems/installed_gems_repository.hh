/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_GEMS_INSTALLED_GEMS_REPOSITORY_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_GEMS_INSTALLED_GEMS_REPOSITORY_HH 1

#include <paludis/repository.hh>
#include <paludis/repositories/gems/params-fwd.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <memory>

namespace paludis
{
    /**
     * Repository for installed Gem packages.
     *
     * \ingroup grpgemsrepository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE InstalledGemsRepository :
        public Repository,
        public RepositoryDestinationInterface,
        public std::enable_shared_from_this<InstalledGemsRepository>,
        private PrivateImplementationPattern<InstalledGemsRepository>
    {
        private:
            PrivateImplementationPattern<InstalledGemsRepository>::ImpPtr & _imp;
            void _add_metadata_keys() const;

            void need_category_names() const;
            void need_ids() const;

        protected:
            virtual void need_keys_added() const;

        public:
            /**
             * Constructor.
             */
            InstalledGemsRepository(const gems::InstalledRepositoryParams &);

            /**
             * Destructor.
             */
            ~InstalledGemsRepository();

            virtual void invalidate();

            virtual void invalidate_masks();

            /* RepositoryDestinationInterface */

            virtual bool is_suitable_destination_for(const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool is_default_destination() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool want_pre_post_phases() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual void merge(const MergeParams &) PALUDIS_ATTRIBUTE((noreturn));

        public:
            /* Repository */

            virtual std::shared_ptr<const PackageIDSequence> package_ids(
                    const QualifiedPackageName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::shared_ptr<const QualifiedPackageNameSet> package_names(
                    const CategoryNamePart &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::shared_ptr<const CategoryNamePartSet> category_names() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool has_package_named(const QualifiedPackageName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool has_category_named(const CategoryNamePart &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool some_ids_might_support_action(const SupportsActionTestBase &) const;

            virtual bool some_ids_might_not_be_masked() const;

            virtual const bool is_unimportant() const;

            virtual bool sync(const std::shared_ptr<OutputManager> &) const;

            /* Keys */

            virtual const std::shared_ptr<const MetadataValueKey<std::string> > format_key() const;
            virtual const std::shared_ptr<const MetadataValueKey<FSEntry> > location_key() const;
            virtual const std::shared_ptr<const MetadataValueKey<FSEntry> > installed_root_key() const;
            virtual const std::shared_ptr<const MetadataValueKey<std::string> > accept_keywords_key() const;
            virtual const std::shared_ptr<const MetadataValueKey<std::string> > sync_host_key() const;

            ///\name RepositoryFactory functions
            ///\{

            static RepositoryName repository_factory_name(
                    const Environment * const env,
                    const std::function<std::string (const std::string &)> &);

            static std::shared_ptr<Repository> repository_factory_create(
                    Environment * const env,
                    const std::function<std::string (const std::string &)> &);

            static std::shared_ptr<const RepositoryNameSet> repository_factory_dependencies(
                    const Environment * const env,
                    const std::function<std::string (const std::string &)> &);

            ///\}

            ///\name Set methods
            ///\{

            virtual void populate_sets() const;

            ///\}

            virtual HookResult perform_hook(const Hook & hook)
                PALUDIS_ATTRIBUTE((warn_unused_result));
    };
}


#endif
