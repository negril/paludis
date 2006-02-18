/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#include "dep_atom.hh"
#include "package_database.hh"
#include "indirect_iterator.hh"
#include "stringify.hh"
#include "match_package.hh"

#include <list>
#include <map>
#include <set>

using namespace paludis;

std::ostream &
paludis::operator<< (std::ostream & s, const PackageDatabaseEntry & v)
{
    s << v.get<pde_name>() << "-" << v.get<pde_version>() << "::" << v.get<pde_repository>();
    return s;
}

PackageDatabaseError::PackageDatabaseError(const std::string & message) throw () :
    Exception(message)
{
}

PackageDatabaseLookupError::PackageDatabaseLookupError(const std::string & message) throw () :
    PackageDatabaseError(message)
{
}

DuplicateRepositoryError::DuplicateRepositoryError(const std::string & name) throw () :
    PackageDatabaseError("A repository named '" + name + "' already exists")
{
}

NoSuchPackageError::NoSuchPackageError(const std::string & name) throw () :
    PackageDatabaseLookupError("Could not find '" + name + "'")
{
}

NoSuchRepositoryError::NoSuchRepositoryError(const std::string & name) throw () :
    PackageDatabaseLookupError("Could not find repository '" + name + "'")
{
}

NoSuchVersionError::NoSuchVersionError(const std::string & name,
        const VersionSpec & version) throw () :
    PackageDatabaseLookupError("No version of '" + name + "' named '" + stringify(version) + "'")
{
}

namespace paludis
{
    /**
     * Implementation data for a PackageDatabase.
     */
    template<>
    struct Implementation<PackageDatabase> :
        InternalCounted<Implementation<PackageDatabase> >
    {
        /**
         * Our Repository instances.
         */
        std::list<Repository::ConstPointer> repositories;
    };
}
PackageDatabase::PackageDatabase() :
    PrivateImplementationPattern<PackageDatabase>(new Implementation<PackageDatabase>)
{
}

PackageDatabase::~PackageDatabase()
{
}

void
PackageDatabase::add_repository(const Repository::ConstPointer r)
{
    Context c("When adding a repository named '" + stringify(r->name()) + "':");

    IndirectIterator<std::list<Repository::ConstPointer>::const_iterator, const Repository>
        r_c(_implementation->repositories.begin()),
        r_end(_implementation->repositories.end());
    for ( ; r_c != r_end ; ++r_c)
        if (r_c->name() == r->name())
            throw DuplicateRepositoryError(stringify(r->name()));

    _implementation->repositories.push_back(r);
}

VersionMetadata::ConstPointer
PackageDatabase::fetch_metadata(const PackageDatabaseEntry & e) const
{
    const Repository::ConstPointer rr(fetch_repository(e.get<pde_repository>()));
    if (! rr->has_category_named(e.get<pde_name>().get<qpn_category>()))
        throw NoSuchPackageError(stringify(e.get<pde_name>()));
    if (! rr->has_package_named(e.get<pde_name>()))
        throw NoSuchPackageError(stringify(e.get<pde_name>()));
    if (! rr->has_version(e.get<pde_name>(), e.get<pde_version>()))
        throw NoSuchVersionError(stringify(e.get<pde_name>()), e.get<pde_version>());
    return rr->version_metadata(e.get<pde_name>().get<qpn_category>(),
            e.get<pde_name>().get<qpn_package>(), e.get<pde_version>());
}

Repository::ConstPointer
PackageDatabase::fetch_repository(const RepositoryName & n) const
{
    std::list<Repository::ConstPointer>::const_iterator
        r(_implementation->repositories.begin()),
        r_end(_implementation->repositories.end());
    for ( ; r != r_end ; ++r)
        if ((*r)->name() == n)
            return *r;

    throw NoSuchRepositoryError(stringify(n));
}

QualifiedPackageName
PackageDatabase::fetch_unique_qualified_package_name(
        const PackageNamePart & p) const
{
    QualifiedPackageNameCollection::Pointer result(new QualifiedPackageNameCollection);

    IndirectIterator<std::list<Repository::ConstPointer>::const_iterator, const Repository>
        r(_implementation->repositories.begin()),
        r_end(_implementation->repositories.end());
    for ( ; r != r_end ; ++r)
    {
        CategoryNamePartCollection::ConstPointer cats(r->category_names());
        CategoryNamePartCollection::Iterator c(cats->begin()), c_end(cats->end());
        for ( ; c != c_end ; ++c)
            if (r->has_package_named(*c, p))
                    result->insert(QualifiedPackageName(*c, p));
    }

    if (result->empty())
        throw NoSuchPackageError(stringify(p));
    if (result->size() > 1)
        throw AmbiguousPackageNameError(stringify(p), result->begin(), result->end());

    return *(result->begin());
}

PackageDatabaseEntryCollection::Pointer
PackageDatabase::query(const PackageDepAtom * const a) const
{
    PackageDatabaseEntryCollection::Pointer result(new PackageDatabaseEntryCollection);

    IndirectIterator<std::list<Repository::ConstPointer>::const_iterator, const Repository>
        r(_implementation->repositories.begin()),
        r_end(_implementation->repositories.end());
    for ( ; r != r_end ; ++r)
    {
        if (! r->has_category_named(a->package().get<qpn_category>()))
            continue;

        if (! r->has_package_named(a->package()))
            continue;

        VersionSpecCollection::ConstPointer versions(r->version_specs(a->package()));
        VersionSpecCollection::Iterator v(versions->begin()), v_end(versions->end());
        for ( ; v != v_end ; ++v)
        {
            PackageDatabaseEntry e(a->package(), *v, r->name());
            if (! match_package(this, *a, e))
                continue;

            result->insert(e);
        }
    }


    return result;
}

const RepositoryName &
PackageDatabase::better_repository(const RepositoryName & r1,
        const RepositoryName & r2) const
{
    IndirectIterator<std::list<Repository::ConstPointer>::const_iterator, const Repository>
        r(_implementation->repositories.begin()),
        r_end(_implementation->repositories.end());
    for ( ; r != r_end ; ++r)
    {
        if (r->name() == r1)
            return r2;
        else if (r->name() == r2)
            return r1;
    }

    throw InternalError(PALUDIS_HERE, "better_repository called on non-owned repositories");
}

RepositoryName
PackageDatabase::favourite_repository() const
{
    if (_implementation->repositories.empty())
        return RepositoryName("unnamed");
    else
        return (*_implementation->repositories.begin())->name();
}

PackageDatabaseEntryCollection::Pointer
PackageDatabase::query(PackageDepAtom::ConstPointer a) const
{
    return query(a.raw_pointer());
}

PackageDatabase::RepositoryIterator
PackageDatabase::begin_repositories() const
{
    return _implementation->repositories.begin();
}

PackageDatabase::RepositoryIterator
PackageDatabase::end_repositories() const
{
    return _implementation->repositories.end();
}
