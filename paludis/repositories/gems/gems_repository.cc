/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include "gems_repository.hh"

using namespace paludis;

#include <paludis/repositories/gems/gems_repository-sr.cc>
#include <paludis/util/collection_concrete.hh>
#include <paludis/package_database.hh>

#include <paludis/portage_dep_parser.hh>

namespace paludis
{
    template<>
    struct Implementation<GemsRepository> :
        InternalCounted<Implementation<GemsRepository> >
    {
        const GemsRepositoryParams params;

        void need_entries() const;

        Implementation(const GemsRepositoryParams & p) :
            params(p)
        {
        }
    };

    void
    Implementation<GemsRepository>::need_entries() const
    {
    }
}

bool
GemsRepository::do_has_category_named(const CategoryNamePart & c) const
{
    return stringify(c) == "gems";
}

bool
GemsRepository::do_has_package_named(const QualifiedPackageName & c) const
{
    if (! do_has_category_named(c.category))
        return false;

    _imp->need_entries();

    return false;
}

CategoryNamePartCollection::ConstPointer
GemsRepository::do_category_names() const
{
    static CategoryNamePartCollection::Pointer names(new CategoryNamePartCollection::Concrete);
    if (names->empty())
        names->insert(CategoryNamePart("gems"));

    return names;
}

QualifiedPackageNameCollection::ConstPointer
GemsRepository::do_package_names(const CategoryNamePart & c) const
{
    if (! has_category_named(c))
        return QualifiedPackageNameCollection::ConstPointer(new QualifiedPackageNameCollection::Concrete);

    _imp->need_entries();

    return QualifiedPackageNameCollection::ConstPointer(new QualifiedPackageNameCollection::Concrete);
}

VersionSpecCollection::ConstPointer
GemsRepository::do_version_specs(const QualifiedPackageName & p) const
{
    if (! has_category_named(p.category))
        return VersionSpecCollection::ConstPointer(new VersionSpecCollection::Concrete);

    _imp->need_entries();

    return VersionSpecCollection::ConstPointer(new VersionSpecCollection::Concrete);
}

bool
GemsRepository::do_has_version(const QualifiedPackageName & q, const VersionSpec &) const
{
    if (! has_category_named(q.category))
        return false;

    _imp->need_entries();

    return false;
}

VersionMetadata::ConstPointer
GemsRepository::do_version_metadata(const QualifiedPackageName & q, const VersionSpec & v) const
{
    if (! has_category_named(q.category))
        throw NoSuchPackageError(stringify(PackageDatabaseEntry(q, v, name())));

    _imp->need_entries();

    return VersionMetadata::ConstPointer(new VersionMetadata(&PortageDepParser::parse_depend));
}

bool
GemsRepository::do_is_licence(const std::string &) const
{
    return false;
}

void
GemsRepository::do_install(const QualifiedPackageName &, const VersionSpec &, const InstallOptions &) const
{
}

DepAtom::Pointer
GemsRepository::do_package_set(const SetName &) const
{
    return DepAtom::Pointer(0);
}

SetsCollection::ConstPointer
GemsRepository::sets_list() const
{
    return SetsCollection::Pointer(new SetsCollection::Concrete);
}

bool
GemsRepository::do_sync() const
{
    return false;
}

GemsRepository::GemsRepository(const GemsRepositoryParams & p) :
    Repository(RepositoryName("gems"),
            RepositoryCapabilities::create()
            .mask_interface(0)
            .installable_interface(this)
            .installed_interface(0)
            .news_interface(0)
            .sets_interface(this)
            .syncable_interface(this)
            .uninstallable_interface(0)
            .use_interface(0)
            .world_interface(0)
            .environment_variable_interface(0)
            .mirrors_interface(0)
            .virtuals_interface(0)
            .provides_interface(0)),
    PrivateImplementationPattern<GemsRepository>(new Implementation<GemsRepository>(p))
{
}

GemsRepository::~GemsRepository()
{
}

void
GemsRepository::invalidate() const
{
}

void
GemsRepository::regenerate_cache() const
{
}

RepositoryInfo::ConstPointer
GemsRepository::info(bool) const
{
    return RepositoryInfo::Pointer(new RepositoryInfo);
}

