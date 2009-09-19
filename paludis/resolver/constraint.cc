/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Ciaran McCreesh
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

#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/resolver/serialise-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/make_named_values.hh>
#include <algorithm>
#include <sstream>
#include <vector>
#include <list>

using namespace paludis;
using namespace paludis::resolver;

namespace paludis
{
    template <>
    struct Implementation<Constraints>
    {
        UseExisting strictest_use_existing_for_all;
        bool nothing_is_fine_too_for_all;
        bool all_untaken_for_all;

        Sequence<std::tr1::shared_ptr<const Constraint> > constraints;

        Implementation() :
            strictest_use_existing_for_all(static_cast<UseExisting>(last_ue - 1)),
            nothing_is_fine_too_for_all(true),
            all_untaken_for_all(true)
        {
        }
    };
}

Constraints::Constraints() :
    PrivateImplementationPattern<Constraints>(new Implementation<Constraints>)
{
}

Constraints::~Constraints()
{
}

bool
Constraints::all_untaken_for_all() const
{
    return _imp->all_untaken_for_all;
}

bool
Constraints::nothing_is_fine_too_for_all() const
{
    return _imp->nothing_is_fine_too_for_all;
}

UseExisting
Constraints::strictest_use_existing_for_all() const
{
    return _imp->strictest_use_existing_for_all;
}

Constraints::ConstIterator
Constraints::begin() const
{
    return ConstIterator(_imp->constraints.begin());
}

Constraints::ConstIterator
Constraints::end() const
{
    return ConstIterator(_imp->constraints.end());
}

void
Constraints::add(const std::tr1::shared_ptr<const Constraint> & c)
{
    _imp->constraints.push_back(c);
    _imp->strictest_use_existing_for_all = std::min(_imp->strictest_use_existing_for_all, c->use_existing());
    _imp->nothing_is_fine_too_for_all = _imp->nothing_is_fine_too_for_all && c->nothing_is_fine_too();
    _imp->all_untaken_for_all = _imp->all_untaken_for_all && c->untaken();
}

bool
Constraints::empty() const
{
    return _imp->constraints.empty();
}

void
Constraints::serialise(Serialiser & s) const
{
    s.object("Constraints")
        .member(SerialiserFlags<serialise::container>(), "items", _imp->constraints)
        ;
}

const std::tr1::shared_ptr<Constraints>
Constraints::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "Constraints");
    Deserialisator vv(*v.find_remove_member("items"), "c");
    std::tr1::shared_ptr<Constraints> result(new Constraints);
    for (int n(1), n_end(vv.member<int>("count") + 1) ; n != n_end ; ++n)
        result->add(vv.member<std::tr1::shared_ptr<Constraint> >(stringify(n)));
    return result;
}

void
Constraint::serialise(Serialiser & s) const
{
    s.object("Constraint")
        .member(SerialiserFlags<>(), "destination_type", stringify(destination_type()))
        .member(SerialiserFlags<>(), "nothing_is_fine_too", nothing_is_fine_too())
        .member(SerialiserFlags<serialise::might_be_null>(), "reason", reason())
        .member(SerialiserFlags<>(), "spec", spec())
        .member(SerialiserFlags<>(), "untaken", stringify(untaken()))
        .member(SerialiserFlags<>(), "use_existing", stringify(use_existing()))
        ;
}

namespace
{
    struct IDFinder
    {
        const std::tr1::shared_ptr<const PackageID> visit(const DependencyReason & r) const
        {
            return r.from_id();
        }

        const std::tr1::shared_ptr<const PackageID> visit(const SetReason &) const
        {
            return make_null_shared_ptr();
        }

        const std::tr1::shared_ptr<const PackageID> visit(const PresetReason &) const
        {
            return make_null_shared_ptr();
        }

        const std::tr1::shared_ptr<const PackageID> visit(const TargetReason &) const
        {
            return make_null_shared_ptr();
        }
    };
}

const std::tr1::shared_ptr<Constraint>
Constraint::deserialise(Deserialisation & d)
{
    Context context("When deserialising '" + d.raw_string() + "':");

    Deserialisator v(d, "Constraint");

    const std::tr1::shared_ptr<Reason> reason(v.member<std::tr1::shared_ptr<Reason> >("reason"));
    IDFinder id_finder;

    return make_shared_ptr(new Constraint(make_named_values<Constraint>(
                    value_for<n::destination_type>(destringify<DestinationType>(v.member<std::string>("destination_type"))),
                    value_for<n::nothing_is_fine_too>(v.member<bool>("nothing_is_fine_too")),
                    value_for<n::reason>(reason),
                    value_for<n::spec>(PackageOrBlockDepSpec::deserialise(*v.find_remove_member("spec"),
                            reason->accept_returning<std::tr1::shared_ptr<const PackageID> >(id_finder))),
                    value_for<n::untaken>(v.member<bool>("untaken")),
                    value_for<n::use_existing>(destringify<UseExisting>(v.member<std::string>("use_existing")))
            )));
}

template class PrivateImplementationPattern<Constraints>;
template class WrappedForwardIterator<Constraints::ConstIteratorTag, const std::tr1::shared_ptr<const Constraint> >;

