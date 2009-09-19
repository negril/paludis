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

#include <paludis/resolver/resolver.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/resolver/qpn_s.hh>
#include <paludis/resolver/sanitised_dependencies.hh>
#include <paludis/resolver/arrow.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/resolver/destinations.hh>
#include <paludis/resolver/resolver_functions.hh>
#include <paludis/resolver/suggest_restart.hh>
#include <paludis/resolver/resolutions.hh>
#include <paludis/resolver/serialise.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/log.hh>
#include <paludis/util/join.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/mutex.hh>
#include <paludis/environment.hh>
#include <paludis/notifier_callback.hh>
#include <paludis/dep_spec_flattener.hh>
#include <paludis/spec_tree.hh>
#include <paludis/slot_requirement.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/generator.hh>
#include <paludis/selection.hh>
#include <paludis/metadata_key.hh>
#include <paludis/match_package.hh>
#include <paludis/action.hh>
#include <paludis/version_spec.hh>
#include <paludis/version_requirements.hh>
#include <paludis/package_database.hh>
#include <paludis/repository.hh>
#include <paludis/choice.hh>
#include <paludis/spec_tree.hh>
#include <iostream>
#include <iomanip>
#include <list>
#include <map>
#include <set>
#include "config.h"

using namespace paludis;
using namespace paludis::resolver;

typedef std::map<QPN_S, std::tr1::shared_ptr<Resolution> > ResolutionsByQPN_SMap;
typedef std::map<QualifiedPackageName, std::set<QualifiedPackageName> > Rewrites;

namespace paludis
{
    template <>
    struct Implementation<Resolver>
    {
        const Environment * const env;
        const ResolverFunctions fns;

        ResolutionsByQPN_SMap resolutions_by_qpn_s;

        mutable Mutex rewrites_mutex;
        mutable Rewrites rewrites;
        mutable bool has_rewrites;

        std::tr1::shared_ptr<ResolutionLists> resolution_lists;

        Implementation(const Environment * const e, const ResolverFunctions & f) :
            env(e),
            fns(f),
            has_rewrites(false),
            resolution_lists(new ResolutionLists(make_named_values<ResolutionLists>(
                            value_for<n::all>(new Resolutions),
                            value_for<n::errors>(new Resolutions),
                            value_for<n::ordered>(new Resolutions),
                            value_for<n::untaken>(new Resolutions)
                            )))
        {
        }
    };
}

Resolver::Resolver(const Environment * const e, const ResolverFunctions & f) :
    PrivateImplementationPattern<Resolver>(new Implementation<Resolver>(e, f))
{
}

Resolver::~Resolver()
{
}

void
Resolver::resolve()
{
    Context context("When finding an appropriate resolution:");

    _resolve_decide_with_dependencies();
    _resolve_destinations();
    _resolve_arrows();
    _resolve_order();
}

void
Resolver::_resolve_decide_with_dependencies()
{
    Context context("When resolving and adding dependencies recursively:");

    enum State { deciding_non_suggestions, deciding_suggestions, finished } state = deciding_non_suggestions;
    bool changed(true);
    while (true)
    {
        if (! changed)
            state = State(state + 1);
        if (state == finished)
            break;

        changed = false;
        for (ResolutionsByQPN_SMap::iterator i(_imp->resolutions_by_qpn_s.begin()), i_end(_imp->resolutions_by_qpn_s.end()) ;
                i != i_end ; ++i)
        {
            /* we've already decided */
            if (i->second->decision())
                continue;

            /* we're only being suggested. don't do this on the first pass, so
             * we don't have to do restarts for suggestions later becoming hard
             * deps. */
            if (state == deciding_non_suggestions && i->second->constraints()->all_untaken_for_all())
                continue;

            _imp->env->trigger_notifier_callback(NotifierCallbackResolverStepEvent());

            changed = true;
            _decide(i->first, i->second);

            /* we don't have anything left to do unless we're a 'real' decision */
            switch (i->second->decision()->kind())
            {
                case dk_existing_no_change:
                case dk_changes_to_make:
                    break;

                case dk_nothing_no_change:
                case dk_unable_to_decide:
                case last_dk:
                    continue;
            }

            /* we don't do dependencies for suggestions */
            if (i->second->decision()->taken())
                _add_dependencies(i->first, i->second);
        }
    }
}

void
Resolver::_resolve_destinations()
{
    Context context("When resolving destinations:");

    for (ResolutionsByQPN_SMap::iterator i(_imp->resolutions_by_qpn_s.begin()), i_end(_imp->resolutions_by_qpn_s.end()) ;
            i != i_end ; ++i)
    {
        if (i->second->destinations())
            continue;

        i->second->destinations() = _make_destinations_for(i->first, i->second);
    }
}

const std::tr1::shared_ptr<Destinations>
Resolver::_make_destinations_for(
        const QPN_S & qpn_s,
        const std::tr1::shared_ptr<const Resolution> & resolution) const
{
    switch (resolution->decision()->kind())
    {
        case dk_existing_no_change:
        case dk_nothing_no_change:
        case dk_unable_to_decide:
            return make_shared_ptr(new Destinations(make_named_values<Destinations>(
                            value_for<n::slash>(make_null_shared_ptr())
                            )));

        case dk_changes_to_make:
        case last_dk:
            break;
    }

    bool requires_slash(true); // (resolution->constraints()->for_destination_type(dt_slash)->required());
    return make_shared_ptr(new Destinations(make_named_values<Destinations>(
                    value_for<n::slash>(requires_slash ? _make_slash_destination_for(qpn_s, resolution) : make_null_shared_ptr())
                    )));
}


const std::tr1::shared_ptr<Destination>
Resolver::_make_slash_destination_for(const QPN_S & qpn_s,
        const std::tr1::shared_ptr<const Resolution> & resolution) const
{
    Context context("When finding / destination for '" + stringify(qpn_s) + "':");

    if ((! resolution->decision()) || (! resolution->decision()->if_package_id()))
        throw InternalError(PALUDIS_HERE, "resolver bug: not decided yet");

    std::tr1::shared_ptr<const Repository> repo;
    for (PackageDatabase::RepositoryConstIterator r(_imp->env->package_database()->begin_repositories()),
            r_end(_imp->env->package_database()->end_repositories()) ;
            r != r_end ; ++r)
    {
        if ((! (*r)->installed_root_key()) || ((*r)->installed_root_key()->value() != FSEntry("/")))
            continue;

        if ((*r)->destination_interface() && (*r)->destination_interface()->is_suitable_destination_for(
                    *resolution->decision()->if_package_id()))
        {
            if (repo)
                throw InternalError(PALUDIS_HERE, "unimplemented: multiple destinations, don't know which to take");
            else
                repo = *r;
        }
    }

    if (! repo)
        throw InternalError(PALUDIS_HERE, "unimplemented: no destinations" +
                stringify(*resolution->decision()->if_package_id()));

    return make_shared_ptr(new Destination(make_named_values<Destination>(
                    value_for<n::replacing>(_find_replacing(resolution->decision()->if_package_id(), repo)),
                    value_for<n::repository>(repo->name())
                    )));
}

const std::tr1::shared_ptr<const PackageIDSequence>
Resolver::_find_replacing(
        const std::tr1::shared_ptr<const PackageID> & id,
        const std::tr1::shared_ptr<const Repository> & repo) const
{
    Context context("When working out what is replaced by '" + stringify(*id) +
            "' when it is installed to '" + stringify(repo->name()) + "':");

    std::set<RepositoryName, RepositoryNameComparator> repos;

    if (repo->installed_root_key())
    {
        for (PackageDatabase::RepositoryConstIterator r(_imp->env->package_database()->begin_repositories()),
                r_end(_imp->env->package_database()->end_repositories()) ;
                r != r_end ; ++r)
            if ((*r)->installed_root_key() &&
                    (*r)->installed_root_key()->value() == repo->installed_root_key()->value())
                repos.insert((*r)->name());
    }
    else
        repos.insert(repo->name());

    std::tr1::shared_ptr<PackageIDSequence> result(new PackageIDSequence);
    for (std::set<RepositoryName, RepositoryNameComparator>::const_iterator r(repos.begin()),
            r_end(repos.end()) ;
            r != r_end ; ++r)
    {
        std::tr1::shared_ptr<const PackageIDSequence> ids((*_imp->env)[selection::AllVersionsUnsorted(
                    generator::Package(id->name()) & generator::InRepository(*r))]);
        for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                i != i_end ; ++i)
        {
            if ((*i)->version() == id->version() || _same_slot(*i, id))
                result->push_back(*i);
        }
    }

    return result;
}

bool
Resolver::_same_slot(const std::tr1::shared_ptr<const PackageID> & a,
        const std::tr1::shared_ptr<const PackageID> & b) const
{
    if (a->slot_key())
        return b->slot_key() && a->slot_key()->value() == b->slot_key()->value();
    else
        return ! b->slot_key();
}

void
Resolver::add_target_with_reason(const PackageDepSpec & spec, const std::tr1::shared_ptr<const Reason> & reason)
{
    Context context("When adding target '" + stringify(spec) + "':");

    _imp->env->trigger_notifier_callback(NotifierCallbackResolverStepEvent());

    std::tr1::shared_ptr<const QPN_S_Sequence> qpn_s_s(_get_qpn_s_s_for(spec, reason));
    if (qpn_s_s->empty())
        qpn_s_s = _get_error_qpn_s_s_for(spec, reason);

    for (QPN_S_Sequence::ConstIterator qpn_s(qpn_s_s->begin()), qpn_s_end(qpn_s_s->end()) ;
            qpn_s != qpn_s_end ; ++qpn_s)
    {
        Context context_2("When adding constraints from target '" + stringify(spec) + "' to qpn:s '"
                + stringify(*qpn_s) + "':");

        const std::tr1::shared_ptr<Resolution> dep_resolution(_resolution_for_qpn_s(*qpn_s, true));
        const std::tr1::shared_ptr<ConstraintSequence> constraints(_make_constraints_from_target(*qpn_s, spec, reason));

        for (ConstraintSequence::ConstIterator c(constraints->begin()), c_end(constraints->end()) ;
                c != c_end ; ++c)
            _apply_resolution_constraint(*qpn_s, dep_resolution, *c);
    }
}

void
Resolver::add_target(const PackageDepSpec & spec)
{
    add_target_with_reason(spec, make_shared_ptr(new TargetReason));
}

void
Resolver::add_target(const SetName & set_name)
{
    Context context("When adding set target '" + stringify(set_name) + "':");
    _imp->env->trigger_notifier_callback(NotifierCallbackResolverStepEvent());

    const std::tr1::shared_ptr<const SetSpecTree> set(_imp->env->set(set_name));
    if (! set)
        throw InternalError(PALUDIS_HERE, "unimplemented: no such set");

    DepSpecFlattener<SetSpecTree, PackageDepSpec> flattener(_imp->env);
    set->root()->accept(flattener);

    const std::tr1::shared_ptr<Reason> reason(new SetReason(set_name, make_shared_ptr(new TargetReason)));
    for (DepSpecFlattener<SetSpecTree, PackageDepSpec>::ConstIterator s(flattener.begin()), s_end(flattener.end()) ;
            s != s_end ; ++s)
        add_target_with_reason(**s, reason);
}

const std::tr1::shared_ptr<Resolution>
Resolver::_create_resolution_for_qpn_s(const QPN_S & qpn_s) const
{
    return make_shared_ptr(new Resolution(make_named_values<Resolution>(
                    value_for<n::already_ordered>(false),
                    value_for<n::arrows>(make_shared_ptr(new ArrowSequence)),
                    value_for<n::constraints>(_initial_constraints_for(qpn_s)),
                    value_for<n::decision>(make_null_shared_ptr()),
                    value_for<n::destinations>(make_null_shared_ptr()),
                    value_for<n::qpn_s>(qpn_s),
                    value_for<n::sanitised_dependencies>(make_null_shared_ptr())
                    )));
}

const std::tr1::shared_ptr<Resolution>
Resolver::_resolution_for_qpn_s(const QPN_S & qpn_s, const bool create)
{
    ResolutionsByQPN_SMap::iterator i(_imp->resolutions_by_qpn_s.find(qpn_s));
    if (_imp->resolutions_by_qpn_s.end() == i)
    {
        if (create)
        {
            std::tr1::shared_ptr<Resolution> resolution(_create_resolution_for_qpn_s(qpn_s));
            i = _imp->resolutions_by_qpn_s.insert(std::make_pair(qpn_s, resolution)).first;
            _imp->resolution_lists->all()->append(resolution);
        }
        else
            throw InternalError(PALUDIS_HERE, "resolver bug: expected resolution for "
                    + stringify(qpn_s) + " to exist, but it doesn't");
    }

    return i->second;
}

const std::tr1::shared_ptr<Resolution>
Resolver::_resolution_for_qpn_s(const QPN_S & qpn_s) const
{
    ResolutionsByQPN_SMap::const_iterator i(_imp->resolutions_by_qpn_s.find(qpn_s));
    if (_imp->resolutions_by_qpn_s.end() == i)
        throw InternalError(PALUDIS_HERE, "resolver bug: expected resolution for "
                + stringify(qpn_s) + " to exist, but it doesn't");

    return i->second;
}

const std::tr1::shared_ptr<ConstraintSequence>
Resolver::_make_constraints_from_target(
        const QPN_S & qpn_s,
        const PackageDepSpec & spec,
        const std::tr1::shared_ptr<const Reason> & reason) const
{
    const std::tr1::shared_ptr<ConstraintSequence> result(new ConstraintSequence);
    result->push_back(make_shared_ptr(new Constraint(make_named_values<Constraint>(
                        value_for<n::destination_type>(dt_slash),
                        value_for<n::nothing_is_fine_too>(false),
                        value_for<n::reason>(reason),
                        value_for<n::spec>(spec),
                        value_for<n::untaken>(false),
                        value_for<n::use_existing>(_imp->fns.get_use_existing_fn()(qpn_s, spec, reason))
                        ))));
    return result;
}

const std::tr1::shared_ptr<ConstraintSequence>
Resolver::_make_constraints_from_dependency(const QPN_S & qpn_s, const SanitisedDependency & dep,
        const std::tr1::shared_ptr<const Reason> & reason) const
{
    const std::tr1::shared_ptr<ConstraintSequence> result(new ConstraintSequence);
    if (dep.spec().if_package())
    {
        result->push_back(make_shared_ptr(new Constraint(make_named_values<Constraint>(
                            value_for<n::destination_type>(dt_slash),
                            value_for<n::nothing_is_fine_too>(false),
                            value_for<n::reason>(reason),
                            value_for<n::spec>(*dep.spec().if_package()),
                            value_for<n::untaken>(! _imp->fns.take_dependency_fn()(qpn_s, dep,
                                    reason)),
                            value_for<n::use_existing>(_imp->fns.get_use_existing_fn()(qpn_s, *dep.spec().if_package(),
                                    reason))
                            ))));
    }
    else if (dep.spec().if_block())
    {
        /* nothing is fine too if there's nothing installed matching the block. */
        const std::tr1::shared_ptr<const PackageIDSequence> ids((*_imp->env)[selection::SomeArbitraryVersion(
                    generator::Matches(dep.spec().if_block()->blocking(), MatchPackageOptions()) |
                    filter::InstalledAtRoot(FSEntry("/")))]);

        result->push_back(make_shared_ptr(new Constraint(make_named_values<Constraint>(
                            value_for<n::destination_type>(dt_slash),
                            value_for<n::nothing_is_fine_too>(ids->empty()),
                            value_for<n::reason>(reason),
                            value_for<n::spec>(dep.spec()),
                            value_for<n::untaken>(false),
                            value_for<n::use_existing>(ue_if_possible)
                            ))));
    }
    else
        throw InternalError(PALUDIS_HERE, "resolver bug: huh? it's not a block and it's not a package");

    return result;
}

void
Resolver::_apply_resolution_constraint(
        const QPN_S & qpn_s,
        const std::tr1::shared_ptr<Resolution> & resolution,
        const std::tr1::shared_ptr<const Constraint> & constraint)
{
    if (resolution->decision())
        _verify_new_constraint(qpn_s, resolution, constraint);

    resolution->constraints()->add(constraint);
}

void
Resolver::_verify_new_constraint(const QPN_S & qpn_s,
        const std::tr1::shared_ptr<const Resolution> & resolution,
        const std::tr1::shared_ptr<const Constraint> & constraint)
{
    bool ok;

    if (resolution->decision()->if_package_id())
    {
        if (constraint->spec().if_package())
            ok = match_package(*_imp->env, *constraint->spec().if_package(),
                    *resolution->decision()->if_package_id(), MatchPackageOptions());
        else
            ok = ! match_package(*_imp->env, constraint->spec().if_block()->blocking(),
                    *resolution->decision()->if_package_id(), MatchPackageOptions());
    }
    else
        ok = constraint->nothing_is_fine_too();

    if (ok && dk_existing_no_change == resolution->decision()->kind())
    {
        switch (constraint->use_existing())
        {
            case ue_if_possible:
                break;

            case ue_only_if_transient:
                ok = resolution->decision()->is_transient();
                break;

            case ue_if_same:
                ok = resolution->decision()->is_same();
                break;

            case ue_if_same_version:
                ok = resolution->decision()->is_same_version();
                break;

            case ue_never:
            case last_ue:
                ok = false;
                break;
        }
    }

    if (ok && ! constraint->untaken())
        ok = resolution->decision()->taken();

    if (! ok)
        _made_wrong_decision(qpn_s, resolution, constraint);
}

void
Resolver::_made_wrong_decision(const QPN_S & qpn_s,
        const std::tr1::shared_ptr<const Resolution> & resolution,
        const std::tr1::shared_ptr<const Constraint> & constraint)
{
    /* can we find a resolution that works for all our constraints? */
    std::tr1::shared_ptr<Resolution> adapted_resolution(make_shared_ptr(new Resolution(*resolution)));
    adapted_resolution->constraints()->add(constraint);

    const std::tr1::shared_ptr<Decision> decision(_try_to_find_decision_for(qpn_s, adapted_resolution));
    if (decision)
        _suggest_restart_with(qpn_s, resolution, constraint, decision);
    else
    {
        std::string old_decision("none");
        if (resolution->decision())
        {
            if (resolution->decision()->if_package_id())
                old_decision = stringify(*resolution->decision()->if_package_id());
            else
                old_decision = stringify(resolution->decision()->kind());
        }

        std::stringstream str;
        Serialiser ser(str);
        constraint->serialise(ser);

        throw InternalError(PALUDIS_HERE, "unimplemented: made decision " + old_decision +
                ", now can't make one because of " + str.str());
    }
}

void
Resolver::_suggest_restart_with(const QPN_S & qpn_s,
        const std::tr1::shared_ptr<const Resolution> & resolution,
        const std::tr1::shared_ptr<const Constraint> & constraint,
        const std::tr1::shared_ptr<const Decision> & decision) const
{
    throw SuggestRestart(qpn_s, resolution->decision(), constraint, decision,
            _make_constraint_for_preloading(qpn_s, decision, constraint->destination_type()));
}

const std::tr1::shared_ptr<const Constraint>
Resolver::_make_constraint_for_preloading(
        const QPN_S & qpn_s,
        const std::tr1::shared_ptr<const Decision> & d,
        const DestinationType t) const
{
    const std::tr1::shared_ptr<PresetReason> reason(new PresetReason);

    if (! d->if_package_id())
        throw InternalError(PALUDIS_HERE, "resolver bug: not decided. shouldn't happen.");

    return make_shared_ptr(new Constraint(make_named_values<Constraint>(
                    value_for<n::destination_type>(t),
                    value_for<n::nothing_is_fine_too>(false),
                    value_for<n::reason>(reason),
                    value_for<n::spec>(d->if_package_id()->uniquely_identifying_spec()),
                    value_for<n::untaken>(! d->taken()),
                    value_for<n::use_existing>(_imp->fns.get_use_existing_fn()(
                            qpn_s, d->if_package_id()->uniquely_identifying_spec(), reason))
                    )));
}

void
Resolver::_decide(const QPN_S & qpn_s, const std::tr1::shared_ptr<Resolution> & resolution)
{
    Context context("When deciding upon an origin ID to use for '" + stringify(qpn_s) + "':");

    std::tr1::shared_ptr<Decision> decision(_try_to_find_decision_for(qpn_s, resolution));
    if (decision)
        resolution->decision() = decision;
    else
        resolution->decision() = _cannot_decide_for(qpn_s, resolution);
}

void
Resolver::_add_dependencies(const QPN_S & our_qpn_s, const std::tr1::shared_ptr<Resolution> & our_resolution)
{
    if (! our_resolution->decision()->if_package_id())
        throw InternalError(PALUDIS_HERE, "resolver bug: not decided. shouldn't happen.");

    Context context("When adding dependencies for '" + stringify(our_qpn_s) + "' with '"
            + stringify(*our_resolution->decision()->if_package_id()) + "':");

    const std::tr1::shared_ptr<SanitisedDependencies> deps(new SanitisedDependencies);
    deps->populate(*this, our_resolution->decision()->if_package_id());
    our_resolution->sanitised_dependencies() = deps;

    for (SanitisedDependencies::ConstIterator s(deps->begin()), s_end(deps->end()) ;
            s != s_end ; ++s)
    {
        Context context_2("When handling dependency '" + stringify(s->spec()) + "':");

        if (! _care_about_dependency_spec(our_qpn_s, our_resolution, *s))
            continue;

        const std::tr1::shared_ptr<DependencyReason> reason(new DependencyReason(
                    our_resolution->decision()->if_package_id(), *s));

        std::tr1::shared_ptr<const QPN_S_Sequence> qpn_s_s;

        if (s->spec().if_package())
            qpn_s_s = _get_qpn_s_s_for(*s->spec().if_package(), reason);
        else
            qpn_s_s = _get_qpn_s_s_for_blocker(*s->spec().if_block());

        if (qpn_s_s->empty())
        {
            if (s->spec().if_package())
                qpn_s_s = _get_error_qpn_s_s_for(*s->spec().if_package(), reason);
            else
            {
                /* blocking on something that doesn't exist is fine */
                qpn_s_s.reset(new QPN_S_Sequence);
            }
        }

        for (QPN_S_Sequence::ConstIterator qpn_s(qpn_s_s->begin()), qpn_s_end(qpn_s_s->end()) ;
                qpn_s != qpn_s_end ; ++qpn_s)
        {
            const std::tr1::shared_ptr<Resolution> dep_resolution(_resolution_for_qpn_s(*qpn_s, true));
            const std::tr1::shared_ptr<ConstraintSequence> constraints(_make_constraints_from_dependency(our_qpn_s, *s, reason));

            for (ConstraintSequence::ConstIterator c(constraints->begin()), c_end(constraints->end()) ;
                    c != c_end ; ++c)
                _apply_resolution_constraint(*qpn_s, dep_resolution, *c);
        }
    }
}

bool
Resolver::_care_about_dependency_spec(const QPN_S & qpn_s,
        const std::tr1::shared_ptr<const Resolution> & resolution, const SanitisedDependency & dep) const
{
    return _imp->fns.care_about_dep_fn()(qpn_s, resolution, dep);
}

namespace
{
    struct GetDependencyReason
    {
        const DependencyReason * visit(const DependencyReason & r) const
        {
            return &r;
        }

        const DependencyReason * visit(const SetReason & r) const
        {
            return r.reason_for_set()->accept_returning<const DependencyReason *>(*this);
        }

        const DependencyReason * visit(const TargetReason &) const
        {
            return 0;
        }

        const DependencyReason * visit(const PresetReason &) const
        {
            return 0;
        }
    };

    struct ArrowInfo
    {
        bool causes_pre_arrow;
        bool ignorable;

        ArrowInfo(const DependencyReason & reason) :
            causes_pre_arrow(false),
            ignorable(true)
        {
            const std::tr1::shared_ptr<const ActiveDependencyLabels> labels(
                    reason.sanitised_dependency().active_dependency_labels());

            if (labels->type_labels()->empty())
                throw InternalError(PALUDIS_HERE, "resolver bug: why did that happen?");

            std::for_each(indirect_iterator(labels->type_labels()->begin()),
                    indirect_iterator(labels->type_labels()->end()), accept_visitor(*this));
        }

        void visit(const DependencyBuildLabel &)
        {
            causes_pre_arrow = true;
            ignorable = false;
        }

        void visit(const DependencyRunLabel &)
        {
            causes_pre_arrow = true;
        }

        void visit(const DependencyPostLabel &)
        {
        }

        void visit(const DependencyInstallLabel &)
        {
            causes_pre_arrow = true;
            ignorable = false;
        }

        void visit(const DependencyCompileLabel &)
        {
            causes_pre_arrow = true;
            ignorable = false;
        }
    };
}

void
Resolver::_resolve_arrows()
{
    Context context("When creating arrows for order resolution:");

    for (ResolutionsByQPN_SMap::iterator i(_imp->resolutions_by_qpn_s.begin()), i_end(_imp->resolutions_by_qpn_s.end()) ;
            i != i_end ; ++i)
        for (Constraints::ConstIterator c(i->second->constraints()->begin()),
                c_end(i->second->constraints()->end()) ;
                c != c_end ; ++c)
            _resolve_arrow(i->first, i->second, *c);
}

void
Resolver::_resolve_arrow(
        const QPN_S & qpn_s,
        const std::tr1::shared_ptr<Resolution> &,
        const std::tr1::shared_ptr<const Constraint> & constraint)
{
    GetDependencyReason gdr;
    const DependencyReason * if_dependency_reason(constraint->reason()->accept_returning<const DependencyReason *>(gdr));
    if (! if_dependency_reason)
            return;

    const QPN_S from_qpns(if_dependency_reason->from_id());
    const std::tr1::shared_ptr<Resolution> resolution(_resolution_for_qpn_s(from_qpns, false));

    if (constraint->spec().if_block())
    {
        if (constraint->spec().if_block()->strong())
        {
            resolution->arrows()->push_back(make_shared_ptr(new Arrow(make_named_values<Arrow>(
                                value_for<n::ignorable_pass>(0),
                                value_for<n::to_qpn_s>(qpn_s)
                                ))));
        }
    }
    else
    {
        ArrowInfo a(*if_dependency_reason);
        if (a.causes_pre_arrow)
        {
            int ignorable_pass(0);
            if (_already_met(if_dependency_reason->sanitised_dependency()))
                ignorable_pass = 1;
            else if (a.ignorable)
                ignorable_pass = 2;

            resolution->arrows()->push_back(make_shared_ptr(new Arrow(make_named_values<Arrow>(
                                value_for<n::ignorable_pass>(ignorable_pass),
                                value_for<n::to_qpn_s>(qpn_s)
                                ))));
        }
    }
}

bool
Resolver::_already_met(const SanitisedDependency & dep) const
{
    if (dep.spec().if_package())
    {
        std::tr1::shared_ptr<const PackageIDSequence> ids((*_imp->env)[selection::SomeArbitraryVersion(
                    generator::Matches(*dep.spec().if_package(), MatchPackageOptions()) |
                    filter::InstalledAtRoot(FSEntry("/")))]);
        return ! ids->empty();
    }
    else if (dep.spec().if_block())
    {
        /* it's imposing an arrow, so it's a strong block */
        return false;
    }
    else
        throw InternalError(PALUDIS_HERE, "resolver bug: huh? it's not a block and it's not a package");
}

void
Resolver::_resolve_order()
{
    Context context("When finding an order for selected packages:");

    bool done(false);

    for (ResolutionsByQPN_SMap::iterator i(_imp->resolutions_by_qpn_s.begin()), i_end(_imp->resolutions_by_qpn_s.end()) ;
            i != i_end ; ++i)
    {
        switch (i->second->decision()->kind())
        {
            case dk_existing_no_change:
            case dk_nothing_no_change:
                i->second->already_ordered() = true;
                break;

            case dk_unable_to_decide:
                i->second->already_ordered() = true;
                if (i->second->decision()->taken())
                    _imp->resolution_lists->errors()->append(i->second);
                else
                    _imp->resolution_lists->untaken()->append(i->second);
                break;

            case dk_changes_to_make:
                if (! i->second->decision()->taken())
                {
                    i->second->already_ordered() = true;
                    _imp->resolution_lists->untaken()->append(i->second);
                }
                break;

            case last_dk:
                break;
        }
    }

    while (! done)
    {
        bool any(false);
        done = true;

        int ignore_pass(0);
        while (true)
        {
            for (ResolutionsByQPN_SMap::iterator i(_imp->resolutions_by_qpn_s.begin()), i_end(_imp->resolutions_by_qpn_s.end()) ;
                    i != i_end ; ++i)
            {
                if (i->second->already_ordered())
                    continue;

                if (_can_order_now(i->first, i->second, ignore_pass))
                {
                    if (0 != ignore_pass)
                        Log::get_instance()->message("resolver.cycle_breaking", ll_warning, lc_context)
                            << "Had to use cycle breaking with ignore pass " << ignore_pass
                            << " to order " << i->first << " because of cycle "
                            << _find_cycle(i->first, false);

                    _imp->env->trigger_notifier_callback(NotifierCallbackResolverStepEvent());
                    _do_order(i->first, i->second);
                    any = true;

                    if (0 != ignore_pass)
                        break;
                }
                else
                    done = false;
            }

            if ((! done) && (! any))
            {
                if (ignore_pass >= 2)
                    _unable_to_order_more();
                else
                    ++ignore_pass;
            }
            else
                break;
        }
    }
}

bool
Resolver::_can_order_now(const QPN_S &, const std::tr1::shared_ptr<const Resolution> & resolution,
        const int ignorable_pass) const
{
    for (ArrowSequence::ConstIterator a(resolution->arrows()->begin()), a_end(resolution->arrows()->end()) ;
            a != a_end ; ++a)
    {
        if (0 != (*a)->ignorable_pass())
            if ((*a)->ignorable_pass() <= ignorable_pass)
                continue;

        const std::tr1::shared_ptr<const Resolution> dep_resolution(_resolution_for_qpn_s((*a)->to_qpn_s()));
        if (! dep_resolution->already_ordered())
            return false;
    }

    return true;
}

void
Resolver::_do_order(const QPN_S &, const std::tr1::shared_ptr<Resolution> & resolution)
{
    _imp->resolution_lists->ordered()->append(resolution);
    resolution->already_ordered() = true;
}

void
Resolver::_unable_to_order_more() const
{
    std::cout << "Unable to order any of the following:" << std::endl;

    for (ResolutionsByQPN_SMap::const_iterator i(_imp->resolutions_by_qpn_s.begin()),
            i_end(_imp->resolutions_by_qpn_s.end()) ;
            i != i_end ; ++i)
    {
        if (i->second->already_ordered())
            continue;

        std::cout << "  * " << *i->second->decision()->if_package_id() << " because of cycle "
            << _find_cycle(i->first, true)
            << std::endl;
    }

    throw InternalError(PALUDIS_HERE, "unimplemented: unfixable dep cycle");
}

const std::tr1::shared_ptr<Constraints>
Resolver::_initial_constraints_for(const QPN_S & qpn_s) const
{
    return _imp->fns.get_initial_constraints_for_fn()(qpn_s);
}

Resolver::ResolutionsByQPN_SConstIterator
Resolver::begin_resolutions_by_qpn_s() const
{
    return ResolutionsByQPN_SConstIterator(_imp->resolutions_by_qpn_s.begin());
}

Resolver::ResolutionsByQPN_SConstIterator
Resolver::end_resolutions_by_qpn_s() const
{
    return ResolutionsByQPN_SConstIterator(_imp->resolutions_by_qpn_s.end());
}

int
Resolver::find_any_score(const QPN_S & our_qpn_s, const SanitisedDependency & dep) const
{
    Context context("When working out whether we'd like '" + stringify(dep.spec()) + "' because of '"
            + stringify(our_qpn_s) + "':");

    if (dep.spec().if_block())
        throw InternalError(PALUDIS_HERE, "unimplemented: blockers inside || blocks are horrid");

    const PackageDepSpec & spec(*dep.spec().if_package());

    int operator_bias(0);
    if (spec.version_requirements_ptr() && ! spec.version_requirements_ptr()->empty())
    {
        int score(-1);
        for (VersionRequirements::ConstIterator v(spec.version_requirements_ptr()->begin()),
                v_end(spec.version_requirements_ptr()->end()) ;
                v != v_end ; ++v)
        {
            int local_score(0);

            switch (v->version_operator().value())
            {
                case vo_greater:
                case vo_greater_equal:
                    local_score = 9;
                    break;

                case vo_equal:
                case vo_tilde:
                case vo_nice_equal_star:
                case vo_stupid_equal_star:
                case vo_tilde_greater:
                    local_score = 2;
                    break;

                case vo_less_equal:
                case vo_less:
                    local_score = 1;
                    break;

                case last_vo:
                    local_score = 1;
                    break;
            }

            if (score == -1)
                score = local_score;
            else
                switch (spec.version_requirements_mode())
                {
                    case vr_and:
                        score = std::min(score, local_score);
                        break;

                    case vr_or:
                        score = std::max(score, local_score);
                        break;

                    case last_vr:
                        break;
                }
        }
        operator_bias = score;
    }
    else
    {
        /* don't bias no operator over a >= operator, so || ( >=foo-2 bar )
         * still likes foo. */
        operator_bias = 9;
    }

    /* best: already installed */
    {
        const std::tr1::shared_ptr<const PackageIDSequence> installed_ids((*_imp->env)[selection::BestVersionOnly(
                    generator::Matches(spec, MatchPackageOptions()) |
                    filter::InstalledAtRoot(FSEntry("/")))]);
        if (! installed_ids->empty())
            return 50 + operator_bias;
    }

    /* next: already installed, except with the wrong options */
    if (spec.additional_requirements_ptr())
    {
        const std::tr1::shared_ptr<const PackageIDSequence> installed_ids((*_imp->env)[selection::BestVersionOnly(
                    generator::Matches(spec, MatchPackageOptions() + mpo_ignore_additional_requirements) |
                    filter::InstalledAtRoot(FSEntry("/")))]);
        if (! installed_ids->empty())
            return 40 + operator_bias;
    }

    const std::tr1::shared_ptr<DependencyReason> reason(new DependencyReason(
                _resolution_for_qpn_s(our_qpn_s)->decision()->if_package_id(), dep));
    const std::tr1::shared_ptr<const QPN_S_Sequence> qpn_s_s(_get_qpn_s_s_for(spec, reason));

    /* next: will already be installing */
    {
        for (QPN_S_Sequence::ConstIterator qpn_s(qpn_s_s->begin()), qpn_s_end(qpn_s_s->end()) ;
                qpn_s != qpn_s_end ; ++qpn_s)
        {
            ResolutionsByQPN_SMap::const_iterator i(_imp->resolutions_by_qpn_s.find(*qpn_s));
            if (i != _imp->resolutions_by_qpn_s.end())
                return 30 + operator_bias;
        }
    }

    /* next: could install */
    {
        for (QPN_S_Sequence::ConstIterator qpn_s(qpn_s_s->begin()), qpn_s_end(qpn_s_s->end()) ;
                qpn_s != qpn_s_end ; ++qpn_s)
        {
            const std::tr1::shared_ptr<Resolution> resolution(_create_resolution_for_qpn_s(*qpn_s));
            const std::tr1::shared_ptr<ConstraintSequence> constraints(_make_constraints_from_dependency(our_qpn_s, dep, reason));
            for (ConstraintSequence::ConstIterator c(constraints->begin()), c_end(constraints->end()) ;
                    c != c_end ; ++c)
                resolution->constraints()->add(*c);
            const std::tr1::shared_ptr<Decision> decision(_try_to_find_decision_for(*qpn_s, resolution));
            if (decision)
                return 20 + operator_bias;
        }
    }

    /* next: exists */
    {
        const std::tr1::shared_ptr<const PackageIDSequence> ids((*_imp->env)[selection::BestVersionOnly(
                    generator::Matches(spec, MatchPackageOptions() + mpo_ignore_additional_requirements)
                    )]);
        if (! ids->empty())
            return 10 + operator_bias;
    }

    /* yay, people are depping upon packages that don't exist again. I SMELL A LESSPIPE. */
    return 0;
}

const std::string
Resolver::_find_cycle(const QPN_S & start_qpn_s, const int ignorable_pass) const
{
    std::stringstream result;

    std::set<QPN_S> seen;
    QPN_S current(start_qpn_s);

    bool first(true);
    while (true)
    {
        if (! first)
            result << " -> ";
        first = false;

        result << current;

        if (! seen.insert(current).second)
            break;

        bool ok(false);
        const std::tr1::shared_ptr<const Resolution> resolution(_resolution_for_qpn_s(current));
        for (ArrowSequence::ConstIterator a(resolution->arrows()->begin()), a_end(resolution->arrows()->end()) ;
                a != a_end ; ++a)
        {
            if (_can_order_now(current, resolution, ignorable_pass))
                continue;

            const std::tr1::shared_ptr<const Resolution> to_resolution(_resolution_for_qpn_s((*a)->to_qpn_s()));
            if (to_resolution->already_ordered())
                continue;

            ok = true;
            current = (*a)->to_qpn_s();
            break;
        }

        if (! ok)
            throw InternalError(PALUDIS_HERE, "resolver bug: there's a cycle, but we don't know what it is");
    }

    return result.str();
}

namespace
{
    struct SlotNameFinder
    {
        std::tr1::shared_ptr<SlotName> visit(const SlotExactRequirement & s)
        {
            return make_shared_ptr(new SlotName(s.slot()));
        }

        std::tr1::shared_ptr<SlotName> visit(const SlotAnyUnlockedRequirement &)
        {
            return make_null_shared_ptr();
        }

        std::tr1::shared_ptr<SlotName> visit(const SlotAnyLockedRequirement &)
        {
            return make_null_shared_ptr();
        }
    };
}

const std::tr1::shared_ptr<const QPN_S_Sequence>
Resolver::_get_qpn_s_s_for_blocker(const BlockDepSpec & spec) const
{
    Context context("When finding slots for '" + stringify(spec) + "':");

    std::tr1::shared_ptr<SlotName> exact_slot;
    if (spec.blocking().slot_requirement_ptr())
    {
        SlotNameFinder f;
        exact_slot = spec.blocking().slot_requirement_ptr()->accept_returning<std::tr1::shared_ptr<SlotName> >(f);
    }

    std::tr1::shared_ptr<QPN_S_Sequence> result(new QPN_S_Sequence);
    if (exact_slot)
        result->push_back(QPN_S(spec.blocking(), exact_slot));
    else
    {
        const std::tr1::shared_ptr<const PackageIDSequence> ids((*_imp->env)[selection::BestVersionInEachSlot(
                    generator::Package(*spec.blocking().package_ptr())
                    )]);
        for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                i != i_end ; ++i)
            result->push_back(QPN_S(*i));
    }

    return result;
}

const std::tr1::shared_ptr<QPN_S_Sequence>
Resolver::_get_qpn_s_s_for(
        const PackageDepSpec & spec,
        const std::tr1::shared_ptr<const Reason> & reason) const
{
    Context context("When finding slots for '" + stringify(spec) + "':");

    std::tr1::shared_ptr<SlotName> exact_slot;

    if (spec.slot_requirement_ptr())
    {
        SlotNameFinder f;
        exact_slot = spec.slot_requirement_ptr()->accept_returning<std::tr1::shared_ptr<SlotName> >(f);
    }

    if (exact_slot)
    {
        std::tr1::shared_ptr<QPN_S_Sequence> result(new QPN_S_Sequence);
        result->push_back(QPN_S(spec, exact_slot));
        return result;
    }
    else
        return _imp->fns.get_qpn_s_s_for_fn()(spec, reason);
}

const std::tr1::shared_ptr<QPN_S_Sequence>
Resolver::_get_error_qpn_s_s_for(
        const PackageDepSpec & spec,
        const std::tr1::shared_ptr<const Reason> &) const
{
    Context context("When finding slots for '" + stringify(spec) + "', which can't be found the normal way:");

    std::tr1::shared_ptr<QPN_S_Sequence> result(new QPN_S_Sequence);
    result->push_back(QPN_S(spec, make_null_shared_ptr()));
    return result;
}

const std::tr1::shared_ptr<Decision>
Resolver::_try_to_find_decision_for(
        const QPN_S & qpn_s,
        const std::tr1::shared_ptr<const Resolution> & resolution) const
{
    const std::tr1::shared_ptr<const PackageID> existing_id(_find_existing_id_for(qpn_s, resolution));
    std::pair<const std::tr1::shared_ptr<const PackageID>, bool> installable_id_best(_find_installable_id_for(qpn_s, resolution));
    const std::tr1::shared_ptr<const PackageID> installable_id(installable_id_best.first);
    bool best(installable_id_best.second);

    if (resolution->constraints()->nothing_is_fine_too_for_all() && ! existing_id)
    {
        /* nothing existing, but nothing's ok */
        return make_shared_ptr(new Decision(make_named_values<Decision>(
                        value_for<n::if_package_id>(make_null_shared_ptr()),
                        value_for<n::is_best>(false),
                        value_for<n::is_same>(false),
                        value_for<n::is_same_version>(false),
                        value_for<n::is_transient>(false),
                        value_for<n::kind>(dk_nothing_no_change),
                        value_for<n::taken>(! resolution->constraints()->all_untaken_for_all())
                        )));
    }
    else if (installable_id && ! existing_id)
    {
        /* there's nothing suitable existing. */
        return make_shared_ptr(new Decision(make_named_values<Decision>(
                        value_for<n::if_package_id>(installable_id),
                        value_for<n::is_best>(best),
                        value_for<n::is_same>(false),
                        value_for<n::is_same_version>(false),
                        value_for<n::is_transient>(false),
                        value_for<n::kind>(dk_changes_to_make),
                        value_for<n::taken>(! resolution->constraints()->all_untaken_for_all())
                        )));
    }
    else if (existing_id && ! installable_id)
    {
        /* there's nothing installable. this may or may not be ok. */
        bool is_transient(existing_id->transient_key() && existing_id->transient_key()->value());

        switch (resolution->constraints()->strictest_use_existing_for_all())
        {
            case ue_if_possible:
                break;

            case ue_only_if_transient:
            case ue_if_same:
            case ue_if_same_version:
                if (! is_transient)
                    return make_null_shared_ptr();
                break;

            case ue_never:
                return make_null_shared_ptr();

            case last_ue:
                break;
        }

        return make_shared_ptr(new Decision(make_named_values<Decision>(
                        value_for<n::if_package_id>(existing_id),
                        value_for<n::is_best>(false),
                        value_for<n::is_same>(true),
                        value_for<n::is_same_version>(true),
                        value_for<n::is_transient>(is_transient),
                        value_for<n::kind>(dk_existing_no_change),
                        value_for<n::taken>(! resolution->constraints()->all_untaken_for_all())

                        )));
    }
    else if ((! existing_id) && (! installable_id))
    {
        return make_null_shared_ptr();
    }
    else if (existing_id && installable_id)
    {
        bool is_same_version(existing_id->version() == installable_id->version());
        bool is_same(false);

        if (is_same_version)
        {
            is_same = true;

            std::set<ChoiceNameWithPrefix> common;
            if (existing_id->choices_key() && installable_id->choices_key())
            {
                std::set<ChoiceNameWithPrefix> i_common, u_common;
                for (Choices::ConstIterator k(installable_id->choices_key()->value()->begin()),
                        k_end(installable_id->choices_key()->value()->end()) ;
                        k != k_end ; ++k)
                {
                    if (! (*k)->consider_added_or_changed())
                        continue;

                    for (Choice::ConstIterator i((*k)->begin()), i_end((*k)->end()) ;
                            i != i_end ; ++i)
                        if ((*i)->explicitly_listed())
                            i_common.insert((*i)->name_with_prefix());
                }

                for (Choices::ConstIterator k(existing_id->choices_key()->value()->begin()),
                        k_end(existing_id->choices_key()->value()->end()) ;
                        k != k_end ; ++k)
                {
                    if (! (*k)->consider_added_or_changed())
                        continue;

                    for (Choice::ConstIterator i((*k)->begin()), i_end((*k)->end()) ;
                            i != i_end ; ++i)
                        if ((*i)->explicitly_listed())
                            u_common.insert((*i)->name_with_prefix());
                }

                std::set_intersection(
                        i_common.begin(), i_common.end(),
                        u_common.begin(), u_common.end(),
                        std::inserter(common, common.begin()));
            }

            for (std::set<ChoiceNameWithPrefix>::const_iterator f(common.begin()), f_end(common.end()) ;
                    f != f_end ; ++f)
                if (installable_id->choices_key()->value()->find_by_name_with_prefix(*f)->enabled() !=
                        existing_id->choices_key()->value()->find_by_name_with_prefix(*f)->enabled())
                {
                    is_same = false;
                    break;
                }
        }

        bool is_transient(existing_id->transient_key() && existing_id->transient_key()->value());

        /* we've got existing and installable. do we have any reason not to pick the existing id? */
        switch (resolution->constraints()->strictest_use_existing_for_all())
        {
            case ue_only_if_transient:
            case ue_never:
                return make_shared_ptr(new Decision(make_named_values<Decision>(
                                value_for<n::if_package_id>(installable_id),
                                value_for<n::is_best>(best),
                                value_for<n::is_same>(is_same),
                                value_for<n::is_same_version>(is_same_version),
                                value_for<n::is_transient>(false),
                                value_for<n::kind>(dk_changes_to_make),
                                value_for<n::taken>(! resolution->constraints()->all_untaken_for_all())
                                )));

            case ue_if_same:
                if (is_same)
                    return make_shared_ptr(new Decision(make_named_values<Decision>(
                                    value_for<n::if_package_id>(existing_id),
                                    value_for<n::is_best>(false),
                                    value_for<n::is_same>(is_same),
                                    value_for<n::is_same_version>(is_same_version),
                                    value_for<n::is_transient>(false),
                                    value_for<n::kind>(dk_existing_no_change),
                                    value_for<n::taken>(! resolution->constraints()->all_untaken_for_all())
                                    )));
                else
                    return make_shared_ptr(new Decision(make_named_values<Decision>(
                                    value_for<n::if_package_id>(installable_id),
                                    value_for<n::is_best>(best),
                                    value_for<n::is_same>(is_same),
                                    value_for<n::is_same_version>(is_same_version),
                                    value_for<n::is_transient>(is_transient),
                                    value_for<n::kind>(dk_changes_to_make),
                                    value_for<n::taken>(! resolution->constraints()->all_untaken_for_all())
                                    )));

            case ue_if_same_version:
                if (is_same_version)
                    return make_shared_ptr(new Decision(make_named_values<Decision>(
                                    value_for<n::if_package_id>(existing_id),
                                    value_for<n::is_best>(false),
                                    value_for<n::is_same>(is_same),
                                    value_for<n::is_same_version>(is_same_version),
                                    value_for<n::is_transient>(false),
                                    value_for<n::kind>(dk_existing_no_change),
                                    value_for<n::taken>(! resolution->constraints()->all_untaken_for_all())
                                    )));
                else
                    return make_shared_ptr(new Decision(make_named_values<Decision>(
                                    value_for<n::if_package_id>(installable_id),
                                    value_for<n::is_best>(best),
                                    value_for<n::is_same>(is_same),
                                    value_for<n::is_same_version>(is_same_version),
                                    value_for<n::is_transient>(is_transient),
                                    value_for<n::kind>(dk_changes_to_make),
                                    value_for<n::taken>(! resolution->constraints()->all_untaken_for_all())
                                    )));

            case ue_if_possible:
                return make_shared_ptr(new Decision(make_named_values<Decision>(
                                value_for<n::if_package_id>(existing_id),
                                value_for<n::is_best>(false),
                                value_for<n::is_same>(is_same),
                                value_for<n::is_same_version>(is_same_version),
                                value_for<n::is_transient>(false),
                                value_for<n::kind>(dk_existing_no_change),
                                value_for<n::taken>(! resolution->constraints()->all_untaken_for_all())
                                )));

            case last_ue:
                break;
        }
    }

    throw InternalError(PALUDIS_HERE, "resolver bug: shouldn't be reached");
}

const std::tr1::shared_ptr<Decision>
Resolver::_cannot_decide_for(
        const QPN_S &,
        const std::tr1::shared_ptr<const Resolution> & resolution) const
{
    return make_shared_ptr(new Decision(make_named_values<Decision>(
                    value_for<n::if_package_id>(make_null_shared_ptr()),
                    value_for<n::is_best>(false),
                    value_for<n::is_same>(false),
                    value_for<n::is_same_version>(false),
                    value_for<n::is_transient>(false),
                    value_for<n::kind>(dk_unable_to_decide),
                    value_for<n::taken>(! resolution->constraints()->all_untaken_for_all())
                    )));
}

const std::tr1::shared_ptr<const PackageID>
Resolver::_find_existing_id_for(const QPN_S & qpn_s, const std::tr1::shared_ptr<const Resolution> & resolution) const
{
    const std::tr1::shared_ptr<const PackageIDSequence> ids((*_imp->env)[selection::AllVersionsSorted(
                generator::Package(qpn_s.package()) |
                qpn_s.make_slot_filter() |
                filter::InstalledAtRoot(FSEntry("/"))
                )]);

    return _find_id_for_from(qpn_s, resolution, ids).first;
}

const std::pair<const std::tr1::shared_ptr<const PackageID>, bool>
Resolver::_find_installable_id_for(const QPN_S & qpn_s, const std::tr1::shared_ptr<const Resolution> & resolution) const
{
    const std::tr1::shared_ptr<const PackageIDSequence> ids((*_imp->env)[selection::AllVersionsSorted(
                generator::Package(qpn_s.package()) |
                qpn_s.make_slot_filter() |
                filter::SupportsAction<InstallAction>() |
                filter::NotMasked()
                )]);

    return _find_id_for_from(qpn_s, resolution, ids);
}

const std::pair<const std::tr1::shared_ptr<const PackageID>, bool>
Resolver::_find_id_for_from(
        const QPN_S &, const std::tr1::shared_ptr<const Resolution> & resolution,
        const std::tr1::shared_ptr<const PackageIDSequence> & ids) const
{
    bool best(true);
    for (PackageIDSequence::ReverseConstIterator i(ids->rbegin()), i_end(ids->rend()) ;
            i != i_end ; ++i)
    {
        bool ok(true);
        for (Constraints::ConstIterator c(resolution->constraints()->begin()),
                c_end(resolution->constraints()->end()) ;
                c != c_end ; ++c)
        {
            if ((*c)->spec().if_package())
                ok = ok && match_package(*_imp->env, *(*c)->spec().if_package(), **i, MatchPackageOptions());
            else
                ok = ok && ! match_package(*_imp->env, (*c)->spec().if_block()->blocking(), **i, MatchPackageOptions());

            if (! ok)
                break;
        }

        if (ok)
            return std::make_pair(*i, best);

        best = false;
    }

    return std::make_pair(make_null_shared_ptr(), false);
}

const std::tr1::shared_ptr<const ResolutionLists>
Resolver::resolution_lists() const
{
    return _imp->resolution_lists;
}

const std::tr1::shared_ptr<DependencySpecTree>
Resolver::rewrite_if_special(const PackageOrBlockDepSpec & s, const QPN_S & our_qpn_s) const
{
    if (s.if_package() && s.if_package()->package_ptr())
    {
        if (s.if_package()->package_ptr()->category() != CategoryNamePart("virtual"))
            return make_null_shared_ptr();
        _need_rewrites();

        Rewrites::const_iterator r(_imp->rewrites.find(*s.if_package()->package_ptr()));
        if (r == _imp->rewrites.end())
            return make_null_shared_ptr();

        std::tr1::shared_ptr<DependencySpecTree> result(new DependencySpecTree(make_shared_ptr(new AllDepSpec)));
        std::tr1::shared_ptr<DependencySpecTree::NodeType<AnyDepSpec>::Type> any(result->root()->append(make_shared_ptr(new AnyDepSpec)));
        for (std::set<QualifiedPackageName>::const_iterator n(r->second.begin()), n_end(r->second.end()) ;
                n != n_end ; ++n)
            any->append(make_shared_ptr(new PackageDepSpec(PartiallyMadePackageDepSpec(*s.if_package()).package(*n))));

        return result;
    }
    else if (s.if_block() && s.if_block()->blocking().package_ptr())
    {
        if (s.if_block()->blocking().package_ptr()->category() != CategoryNamePart("virtual"))
            return make_null_shared_ptr();
        _need_rewrites();

        Rewrites::const_iterator r(_imp->rewrites.find(*s.if_block()->blocking().package_ptr()));
        if (r == _imp->rewrites.end())
            return make_null_shared_ptr();

        std::tr1::shared_ptr<DependencySpecTree> result(new DependencySpecTree(make_shared_ptr(new AllDepSpec)));
        for (std::set<QualifiedPackageName>::const_iterator n(r->second.begin()), n_end(r->second.end()) ;
                n != n_end ; ++n)
        {
            if (*n == our_qpn_s.package())
                continue;

            PackageDepSpec spec(PartiallyMadePackageDepSpec(s.if_block()->blocking()).package(*n));
            std::string prefix(s.if_block()->blocking().text());
            std::string::size_type p(prefix.find_first_not_of('!'));
            if (std::string::npos != p)
                prefix.erase(p);
            result->root()->append(make_shared_ptr(new BlockDepSpec(prefix + stringify(spec), spec, s.if_block()->strong())));
        }

        return result;
    }
    else
        return make_null_shared_ptr();
}

void
Resolver::_need_rewrites() const
{
#ifdef ENABLE_VIRTUALS_REPOSITORY
    Lock lock(_imp->rewrites_mutex);
    if (_imp->has_rewrites)
        return;
    _imp->has_rewrites = true;

    const std::tr1::shared_ptr<const PackageIDSequence> ids((*_imp->env)[selection::AllVersionsSorted(
                generator::InRepository(RepositoryName("virtuals")) +
                generator::InRepository(RepositoryName("installed-virtuals"))
                )]);
    for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
            i != i_end ; ++i)
    {
        if (! ((*i)->virtual_for_key()))
            throw InternalError(PALUDIS_HERE, "huh? " + stringify(**i) + " has no virtual_for_key");
        _imp->rewrites.insert(std::make_pair((*i)->name(), std::set<QualifiedPackageName>())).first->second.insert(
                (*i)->virtual_for_key()->value()->name());
    }
#endif
}

template class WrappedForwardIterator<Resolver::ResolutionsByQPN_SConstIteratorTag,
         const std::pair<const QPN_S, std::tr1::shared_ptr<Resolution> > >;

