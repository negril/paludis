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

#include <paludis/dep_atom.hh>
#include <paludis/dep_atom_flattener.hh>
#include <paludis/dep_atom_pretty_printer.hh>
#include <paludis/dep_list/dep_list.hh>
#include <paludis/dep_list/exceptions.hh>
#include <paludis/dep_list/range_rewriter.hh>
#include <paludis/match_package.hh>
#include <paludis/hashed_containers.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/join.hh>
#include <paludis/util/log.hh>
#include <paludis/util/save.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/tokeniser.hh>

#include <algorithm>
#include <functional>
#include <vector>
#include <set>

using namespace paludis;

#include <paludis/dep_list/dep_list-sr.cc>

DepListOptions::DepListOptions() :
    reinstall(dl_reinstall_never),
    reinstall_scm(dl_reinstall_scm_never),
    target_type(dl_target_package),
    upgrade(dl_upgrade_always),
    downgrade(dl_downgrade_as_needed),
    new_slots(dl_new_slots_always),
    fall_back(dl_fall_back_as_needed_except_targets),
    installed_deps_pre(dl_deps_discard),
    installed_deps_runtime(dl_deps_try_post),
    installed_deps_post(dl_deps_try_post),
    uninstalled_deps_pre(dl_deps_pre),
    uninstalled_deps_runtime(dl_deps_pre_or_post),
    uninstalled_deps_post(dl_deps_post),
    uninstalled_deps_suggested(dl_deps_try_post),
    suggested(dl_suggested_show),
    circular(dl_circular_error),
    use(dl_use_deps_standard),
    blocks(dl_blocks_accumulate),
    dependency_tags(false)
{
    /* when changing the above, also see src/paludis/command_line.cc. */
}

namespace paludis
{
    typedef std::list<DepListEntry> MergeList;
    typedef MakeHashedMultiMap<QualifiedPackageName, MergeList::iterator>::Type MergeListIndex;

    template<>
    struct Implementation<DepList> :
        InternalCounted<Implementation<DepList> >
    {
        const Environment * const env;
        CountedPtr<DepListOptions, count_policy::ExternalCountTag> opts;

        MergeList merge_list;
        MergeList::const_iterator current_merge_list_entry;
        MergeList::iterator merge_list_insert_position;
        long merge_list_generation;

        MergeListIndex merge_list_index;

        DepAtom::ConstPointer current_top_level_target;

        bool throw_on_blocker;

        const PackageDatabaseEntry * current_pde() const
        {
            if (current_merge_list_entry != merge_list.end())
                return &current_merge_list_entry->package;
            return 0;
        }

        Implementation(const Environment * const e, const DepListOptions & o) :
            env(e),
            opts(new DepListOptions(o)),
            current_merge_list_entry(merge_list.end()),
            merge_list_insert_position(merge_list.end()),
            merge_list_generation(0),
            current_top_level_target(0),
            throw_on_blocker(o.blocks == dl_blocks_error)
        {
        }
    };
}

namespace
{
    struct GenerationGreaterThan
    {
        long g;

        GenerationGreaterThan(long gg) :
            g(gg)
        {
        }

        template <typename T_>
        bool operator() (const T_ & e) const
        {
            return e.generation > g;
        }
    };

    struct RemoveTagsWithGenerationGreaterThan
    {
        long g;

        RemoveTagsWithGenerationGreaterThan(long gg) :
            g(gg)
        {
        }

        void operator() (DepListEntry & e) const
        {
            /* see EffSTL 9 for why this is so painful */
            if (e.tags->empty())
                return;
            DepListEntryTags::Pointer t(new DepListEntryTags::Concrete);
            GenerationGreaterThan pred(g);
            for (DepListEntryTags::Iterator i(e.tags->begin()), i_end(e.tags->end()) ;
                    i != i_end ; ++i)
                if (! pred(*i))
                    t->insert(*i);
            std::swap(e.tags, t);
        }
    };

    class DepListTransaction
    {
        protected:
            MergeList & _list;
            MergeListIndex & _index;
            long & _generation;
            int _initial_generation;
            bool _committed;

        public:
            DepListTransaction(MergeList & l, MergeListIndex & i, long & g) :
                _list(l),
                _index(i),
                _generation(g),
                _initial_generation(g),
                _committed(false)
            {
                ++_generation;
            }

            void commit()
            {
                _committed = true;
            }

            ~DepListTransaction()
            {
                if (_committed)
                    return;

                /* See EffSTL 9 */
                GenerationGreaterThan pred(_initial_generation);
                for (MergeList::iterator i(_list.begin()) ; i != _list.end() ; )
                {
                    if (! pred(*i))
                        ++i;
                    else
                    {
                        for (std::pair<MergeListIndex::iterator, MergeListIndex::iterator> p(
                                    _index.equal_range(i->package.name)) ; p.first != p.second ; )
                            if (p.first->second == i)
                                _index.erase(p.first++);
                            else
                                ++p.first;

                        _list.erase(i++);
                    }
                }

                std::for_each(_list.begin(), _list.end(),
                        RemoveTagsWithGenerationGreaterThan(_initial_generation));
            }
    };

    struct MatchDepListEntryAgainstPackageDepAtom
    {
        const Environment * const env;
        const PackageDepAtom * const a;

        MatchDepListEntryAgainstPackageDepAtom(const Environment * const ee,
                const PackageDepAtom * const aa) :
            env(ee),
            a(aa)
        {
        }

        bool operator() (const std::pair<const QualifiedPackageName, MergeList::const_iterator> & e)
        {
            switch (e.second->kind)
            {
                case dlk_virtual:
                case dlk_package:
                case dlk_provided:
                case dlk_already_installed:
                case dlk_subpackage:
                    return match_package(env, a, e.second->package);

                case dlk_block:
                case dlk_masked:
                case dlk_suggested:
                    return false;

                case last_dlk:
                    ;
            }

            throw InternalError(PALUDIS_HERE, "Bad e.second->kind");
        }
    };

    struct IsViableAnyDepAtomChild
    {
        const Environment * const env;
        const PackageDatabaseEntry * const pde;

        IsViableAnyDepAtomChild(const Environment * const e, const PackageDatabaseEntry * const p) :
            env(e),
            pde(p)
        {
        }

        bool operator() (PackageDepAtom::ConstPointer atom)
        {
            const UseDepAtom * const u(atom->as_use_dep_atom());
            if (0 != u)
                return env->query_use(u->flag(), pde) ^ u->inverse();
            else
                return true;
        }
    };

    struct IsInterestingPDADepAtomChild
    {
        const Environment * const env;

        IsInterestingPDADepAtomChild(const Environment * const e) :
            env(e)
        {
        }

        bool operator() (PackageDepAtom::ConstPointer atom)
        {
            const PackageDepAtom * const u(atom->as_package_dep_atom());
            if (0 != u)
            {
                return ! env->package_database()->query(PackageDepAtom(u->package()),
                        is_installed_only, qo_whatever)->empty();
            }
            else
                return false;
        }
    };
}

struct DepList::QueryVisitor :
    DepAtomVisitorTypes::ConstVisitor
{
    bool result;
    const DepList * const d;
    bool ignore_current_pde;

    QueryVisitor(const DepList * const dd, const bool i) :
        result(true),
        d(dd),
        ignore_current_pde(i)
    {
    }

    void visit(const PlainTextDepAtom * const) PALUDIS_ATTRIBUTE((noreturn));
    void visit(const PackageDepAtom * const);
    void visit(const UseDepAtom * const);
    void visit(const AnyDepAtom * const);
    void visit(const BlockDepAtom * const);
    void visit(const AllDepAtom * const);
};

void
DepList::QueryVisitor::visit(const PlainTextDepAtom * const)
{
    throw InternalError(PALUDIS_HERE, "Got PlainTextDepAtom?");
}

void
DepList::QueryVisitor::visit(const PackageDepAtom * const a)
{
    /* a pda matches if we'll be installed by the time we reach the current point. This
     * means that merely being installed is not enough, if we'll have our version changed
     * by something in the merge list. */

    result = false;

    PackageDatabaseEntryCollection::ConstPointer matches(d->_imp->env->package_database()->query(
                *a, is_installed_only, qo_whatever));

    for (PackageDatabaseEntryCollection::Iterator m(matches->begin()), m_end(matches->end()) ;
            m != m_end ; ++m)
    {
        /* check that we haven't been replaced by something in the same slot */
        VersionMetadata::ConstPointer vm(d->_imp->env->package_database()->fetch_repository(m->repository)->
                version_metadata(m->name, m->version));
        SlotName slot(vm->slot);
        const VirtualMetadata * const vif(vm->get_virtual_interface());

        std::pair<MergeListIndex::const_iterator, MergeListIndex::const_iterator> p(
                d->_imp->merge_list_index.equal_range(a->package()));

        bool replaced(false);
        PackageDepAtom atom(a->package());
        while (p.second != ((p.first = std::find_if(p.first, p.second,
                            MatchDepListEntryAgainstPackageDepAtom(d->_imp->env, &atom)))))
        {
            if (p.first->second->metadata->slot != slot)
                p.first = next(p.first);
            else if (ignore_current_pde &&
                    p.first->second == d->_imp->current_merge_list_entry)
                p.first = next(p.first);
            else if (ignore_current_pde &&
                    d->_imp->current_merge_list_entry != d->_imp->merge_list.end() &&
                    p.first->second->associated_entry == &*d->_imp->current_merge_list_entry &&
                    (a->version_requirements_ptr() || a->slot_ptr() ||
                     (vif && vif->virtual_for.name != p.first->second->associated_entry->package.name)))
                p.first = next(p.first);
            else
            {
                replaced = true;
                break;
            }
        }

        if (! replaced)
        {
            result = true;
            return;
        }
    }

    /* check the merge list for any new packages that match */
    std::pair<MergeListIndex::const_iterator, MergeListIndex::const_iterator> p(
            d->_imp->merge_list_index.equal_range(a->package()));

    while (p.second != ((p.first = std::find_if(p.first, p.second,
                        MatchDepListEntryAgainstPackageDepAtom(d->_imp->env, a)))))
    {
        if (ignore_current_pde && p.first->second == d->_imp->current_merge_list_entry)
            p.first = next(p.first);
        else if (ignore_current_pde && d->_imp->current_merge_list_entry != d->_imp->merge_list.end()
                && p.first->second->associated_entry == &*d->_imp->current_merge_list_entry)
            p.first = next(p.first);
        else
        {
            result = true;
            return;
        }
    }
}

void
DepList::QueryVisitor::visit(const UseDepAtom * const a)
{
    /* for use? ( ) dep atoms, return true if we're not enabled, so that
     * weird || ( ) cases work. */
    if (d->_imp->env->query_use(a->flag(), d->_imp->current_pde()) ^ a->inverse())
    {
        result = true;
        for (CompositeDepAtom::Iterator c(a->begin()), c_end(a->end()) ; c != c_end ; ++c)
        {
            (*c)->accept(this);
            if (! result)
                return;
        }
    }
    else
        result = true;
}

void
DepList::QueryVisitor::visit(const AnyDepAtom * const a)
{
    /* empty || ( ) must resolve to true */
    std::list<DepAtom::ConstPointer> viable_children;
    std::copy(a->begin(), a->end(), filter_inserter(std::back_inserter(viable_children),
                IsViableAnyDepAtomChild(d->_imp->env, d->_imp->current_pde())));

    RangeRewriter r;
    std::for_each(viable_children.begin(), viable_children.end(), accept_visitor(&r));
    if (r.atom())
    {
        viable_children.clear();
        viable_children.push_back(r.atom());
    }

    result = true;
    for (std::list<DepAtom::ConstPointer>::const_iterator c(viable_children.begin()),
            c_end(viable_children.end()) ; c != c_end ; ++c)
    {
        (*c)->accept(this);
        if (result)
            return;
    }
}

void
DepList::QueryVisitor::visit(const BlockDepAtom * const a)
{
    a->blocked_atom()->accept(this);
    result = !result;
}

void
DepList::QueryVisitor::visit(const AllDepAtom * const a)
{
    for (CompositeDepAtom::Iterator c(a->begin()), c_end(a->end()) ; c != c_end ; ++c)
    {
        (*c)->accept(this);
        if (! result)
            return;
    }
}

struct DepList::AddVisitor :
    DepAtomVisitorTypes::ConstVisitor,
    DepAtomVisitorTypes::ConstVisitor::VisitChildren<AddVisitor, AllDepAtom>
{
    DepList * const d;

    AddVisitor(DepList * const dd) :
        d(dd)
    {
    }

    void visit(const PlainTextDepAtom * const) PALUDIS_ATTRIBUTE((noreturn));
    void visit(const PackageDepAtom * const);
    void visit(const UseDepAtom * const);
    void visit(const AnyDepAtom * const);
    void visit(const BlockDepAtom * const);
    using DepAtomVisitorTypes::ConstVisitor::VisitChildren<AddVisitor, AllDepAtom>::visit;
};

void
DepList::AddVisitor::visit(const PlainTextDepAtom * const)
{
    throw InternalError(PALUDIS_HERE, "Got PlainTextDepAtom?");
}

void
DepList::AddVisitor::visit(const PackageDepAtom * const a)
{
    Context context("When adding PackageDepAtom '" + stringify(*a) + "':");

    /* find already installed things */
    PackageDatabaseEntryCollection::ConstPointer already_installed(d->_imp->env->package_database()->query(
                *a, is_installed_only, qo_order_by_version));

    /* are we already on the merge list? */
    std::pair<MergeListIndex::iterator, MergeListIndex::iterator> q(
            d->_imp->merge_list_index.equal_range(a->package()));
    MergeListIndex::iterator qq(std::find_if(q.first, q.second,
                MatchDepListEntryAgainstPackageDepAtom(d->_imp->env, a)));

    MergeList::iterator existing_merge_list_entry(qq == q.second ? d->_imp->merge_list.end() : qq->second);
    if (existing_merge_list_entry != d->_imp->merge_list.end())
    {
        /* tag it */
        if (a->tag())
            existing_merge_list_entry->tags->insert(DepTagEntry::create()
                    .tag(a->tag())
                    .generation(d->_imp->merge_list_generation));

        if (d->_imp->opts->dependency_tags && d->_imp->current_pde())
            existing_merge_list_entry->tags->insert(DepTagEntry::create()
                    .tag(DepTag::Pointer(new DependencyDepTag(*d->_imp->current_pde())))
                    .generation(d->_imp->merge_list_generation));

        /* have our deps been merged already, or is this a circular dep? */
        if (dle_no_deps == existing_merge_list_entry->state)
        {
            /* is a sufficiently good version installed? */
            if (! already_installed->empty())
                return;

            if (d->_imp->opts->circular == dl_circular_discard)
            {
                Log::get_instance()->message(ll_qa, lc_context, "Dropping circular dependency on '"
                        + stringify(existing_merge_list_entry->package) + "'");
                return;
            }
            else if (d->_imp->opts->circular == dl_circular_discard_silently)
                return;

            throw CircularDependencyError("Atom '" + stringify(*a) + "' matched by merge list entry '" +
                    stringify(existing_merge_list_entry->package) + "', which does not yet have its "
                    "dependencies installed");
        }
        else
            return;
    }

    /* find installable candidates, and find the best visible candidate */
    const PackageDatabaseEntry * best_visible_candidate(0);
    PackageDatabaseEntryCollection::ConstPointer installable_candidates(
            d->_imp->env->package_database()->query(*a, is_installable_only, qo_order_by_version));

    for (PackageDatabaseEntryCollection::ReverseIterator p(installable_candidates->rbegin()),
            p_end(installable_candidates->rend()) ; p != p_end ; ++p)
        if (! d->_imp->env->mask_reasons(*p).any())
        {
            best_visible_candidate = &*p;
            break;
        }

    /* are we allowed to override mask reasons? */
    if (! best_visible_candidate && d->_imp->opts->override_masks.any())
    {
        DepListOverrideMask next(static_cast<DepListOverrideMask>(0));
        DepListOverrideMasks masks_to_override;

        do
        {
            while (next != last_dl_override)
            {
                if (masks_to_override.test(next))
                    next = static_cast<DepListOverrideMask>(static_cast<int>(next) + 1);
                else if (d->_imp->opts->override_masks.test(next))
                {
                    masks_to_override.set(next);
                    break;
                }
                else
                    next = static_cast<DepListOverrideMask>(static_cast<int>(next) + 1);
            }

            if (next == last_dl_override)
                break;

            MaskReasons mask_mask;
            if (masks_to_override.test(dl_override_repository_masks))
                mask_mask.set(mr_repository_mask);
            if (masks_to_override.test(dl_override_profile_masks))
                mask_mask.set(mr_profile_mask);
            if (masks_to_override.test(dl_override_licenses))
                mask_mask.set(mr_license);
            mask_mask.set(mr_by_association);
            mask_mask.flip();

            bool override_tilde_keywords(masks_to_override.test(dl_override_tilde_keywords));
            bool override_unkeyworded(masks_to_override.test(dl_override_unkeyworded));

            for (PackageDatabaseEntryCollection::ReverseIterator p(installable_candidates->rbegin()),
                    p_end(installable_candidates->rend()) ; p != p_end ; ++p)
            {
                if (! (d->_imp->env->mask_reasons(*p, override_tilde_keywords, override_unkeyworded)
                            & mask_mask).any())
                {
                    d->add_error_package(*p, dlk_masked);
                    best_visible_candidate = &*p;
                    break;
                }
            }
        } while (! best_visible_candidate);
    }

    /* no installable candidates. if we're already installed, that's ok (except for top level
     * package targets), otherwise error. */
    if (! best_visible_candidate)
    {
        bool can_fall_back;
        do
        {
            switch (d->_imp->opts->fall_back)
            {
                case dl_fall_back_never:
                    can_fall_back = false;
                    continue;

                case dl_fall_back_as_needed_except_targets:
                    if (! d->_imp->current_pde())
                        can_fall_back = false;
                    else if (already_installed->empty())
                        can_fall_back = true;
                    else
                        can_fall_back = ! d->is_top_level_target(*already_installed->last());

                    continue;

                case dl_fall_back_as_needed:
                    can_fall_back = true;
                    continue;

                case last_dl_fall_back:
                    ;
            }

            throw InternalError(PALUDIS_HERE, "Bad fall_back value '" + stringify(d->_imp->opts->fall_back) + "'");
        } while (false);

        if (already_installed->empty() || ! can_fall_back)
        {
            if (! a->use_requirements_ptr())
                throw AllMaskedError(stringify(*a));

            PackageDatabaseEntryCollection::ConstPointer match_except_reqs(d->_imp->env->package_database()->query(
                        *a->without_use_requirements(), is_any, qo_whatever));

            for (PackageDatabaseEntryCollection::Iterator i(match_except_reqs->begin()),
                    i_end(match_except_reqs->end()) ; i != i_end ; ++i)
                if (! (d->_imp->env->mask_reasons(*i).any()))
                    throw UseRequirementsNotMetError(stringify(*a));

            throw AllMaskedError(stringify(*a));
        }
        else
        {
            Log::get_instance()->message(ll_warning, lc_context, "No visible packages matching '"
                    + stringify(*a) + "', falling back to installed package '"
                    + stringify(*already_installed->last()) + "'");
            d->add_already_installed_package(*already_installed->last(), a->tag());
            return;
        }
    }

    SlotName slot(d->_imp->env->package_database()->fetch_repository(best_visible_candidate->repository)->
            version_metadata(best_visible_candidate->name, best_visible_candidate->version)->slot);
    PackageDatabaseEntryCollection::Pointer already_installed_in_same_slot(
            new PackageDatabaseEntryCollection::Concrete);
    for (PackageDatabaseEntryCollection::Iterator aa(already_installed->begin()),
            aa_end(already_installed->end()) ; aa != aa_end ; ++aa)
        if (d->_imp->env->package_database()->fetch_repository(aa->repository)->
                version_metadata(aa->name, aa->version)->slot == slot)
            already_installed_in_same_slot->push_back(*aa);
    /* no need to sort already_installed_in_same_slot here, although if the above is
     * changed then check that this still holds... */

    /* we have an already installed version. do we want to use it? */
    if (! already_installed_in_same_slot->empty())
    {
        if (d->prefer_installed_over_uninstalled(*already_installed_in_same_slot->last(), *best_visible_candidate))
        {
            Log::get_instance()->message(ll_debug, lc_context, "Taking installed package '"
                    + stringify(*already_installed_in_same_slot->last()) + "' over '" + stringify(*best_visible_candidate) + "'");
            d->add_already_installed_package(*already_installed_in_same_slot->last(), a->tag());
            return;
        }
        else
            Log::get_instance()->message(ll_debug, lc_context, "Not taking installed package '"
                    + stringify(*already_installed_in_same_slot->last()) + "' over '" + stringify(*best_visible_candidate) + "'");
    }
    else if ((! already_installed->empty()) && (dl_new_slots_as_needed == d->_imp->opts->new_slots))
    {
        /* we have an already installed, but not in the same slot, and our options
         * allow us to take this. */
        if (d->prefer_installed_over_uninstalled(*already_installed->last(), *best_visible_candidate))
        {
            Log::get_instance()->message(ll_debug, lc_context, "Taking installed package '"
                    + stringify(*already_installed->last()) + "' over '" + stringify(*best_visible_candidate)
                    + "' (in different slot)");
            d->add_already_installed_package(*already_installed->last(), a->tag());
            return;
        }
        else
            Log::get_instance()->message(ll_debug, lc_context, "Not taking installed package '"
                    + stringify(*already_installed->last()) + "' over '" + stringify(*best_visible_candidate)
                    + "' (in different slot)");
    }
    else
        Log::get_instance()->message(ll_debug, lc_context, "No installed packages in SLOT '"
                + stringify(slot) + "', taking uninstalled package '" + stringify(*best_visible_candidate) + "'");

    /* if this is a downgrade, make sure that that's ok */
    switch (d->_imp->opts->downgrade)
    {
        case dl_downgrade_as_needed:
            break;

        case dl_downgrade_error:
        case dl_downgrade_warning:
            {
                PackageDatabaseEntryCollection::Pointer are_we_downgrading(
                        d->_imp->env->package_database()->query(PackageDepAtom(
                                stringify(a->package()) + ":" + stringify(slot)),
                            is_installed_only, qo_order_by_version));

                if (are_we_downgrading->empty())
                    break;

                if (are_we_downgrading->last()->version <= best_visible_candidate->version)
                    break;

                if (d->_imp->opts->downgrade == dl_downgrade_error)
                    throw DowngradeNotAllowedError(stringify(*best_visible_candidate),
                            stringify(*are_we_downgrading->last()));

                Log::get_instance()->message(ll_warning, lc_context, "Downgrade to '" + stringify(*best_visible_candidate)
                        + "' from '" + stringify(*are_we_downgrading->last()) + "' forced");
            }
            break;

        case last_dl_downgrade:
            ;
    }

    d->add_package(*best_visible_candidate, a->tag());
}

void
DepList::AddVisitor::visit(const UseDepAtom * const a)
{
    if (d->_imp->opts->use == dl_use_deps_standard)
    {
        if (d->_imp->env->query_use(a->flag(), d->_imp->current_pde()) ^ a->inverse())
            std::for_each(a->begin(), a->end(), accept_visitor(this));
    }
    else
    {
        RepositoryUseInterface * u;
        if ((! d->_imp->current_pde()) || (! ((u = d->_imp->env->package_database()->fetch_repository(
                                d->_imp->current_pde()->repository)->use_interface))))
            std::for_each(a->begin(), a->end(), accept_visitor(this));
        else if (a->inverse())
        {
            if (! u->query_use_force(a->flag(), d->_imp->current_pde()))
                std::for_each(a->begin(), a->end(), accept_visitor(this));
        }
        else
        {
            if (! u->query_use_mask(a->flag(), d->_imp->current_pde()))
                std::for_each(a->begin(), a->end(), accept_visitor(this));
        }
    }
}

void
DepList::AddVisitor::visit(const AnyDepAtom * const a)
{
    /* annoying requirement: || ( foo? ( ... ) ) resolves to empty if !foo. */
    std::list<DepAtom::ConstPointer> viable_children;
    std::copy(a->begin(), a->end(), filter_inserter(std::back_inserter(viable_children),
                IsViableAnyDepAtomChild(d->_imp->env, d->_imp->current_pde())));

    if (viable_children.empty())
        return;

    RangeRewriter r;
    std::for_each(viable_children.begin(), viable_children.end(), accept_visitor(&r));
    if (r.atom())
    {
        viable_children.clear();
        viable_children.push_back(r.atom());
    }

    /* see if any of our children is already installed. if any is, add it so that
     * any upgrades kick in */
    for (std::list<DepAtom::ConstPointer>::const_iterator c(viable_children.begin()),
            c_end(viable_children.end()) ; c != c_end ; ++c)
    {
        if (d->already_installed(*c))
        {
            d->add(*c);
            return;
        }
    }

    /* if we have something like || ( a >=b-2 ) and b-1 is installed, try to go for
     * the b-2 bit first */
    std::list<DepAtom::ConstPointer> pda_children;
    std::copy(viable_children.begin(), viable_children.end(),
            filter_inserter(std::back_inserter(pda_children), IsInterestingPDADepAtomChild(d->_imp->env)));

    for (std::list<DepAtom::ConstPointer>::const_iterator c(pda_children.begin()),
            c_end(pda_children.end()) ; c != c_end ; ++c)
    {
        try
        {
            Save<bool> save_t(&d->_imp->throw_on_blocker, true);
            Save<DepListOverrideMasks> save_o(&d->_imp->opts->override_masks, DepListOverrideMasks());
            d->add(*c);
            return;
        }
        catch (const DepListError &)
        {
        }
    }

    /* install first available viable option */
    for (std::list<DepAtom::ConstPointer>::const_iterator c(viable_children.begin()),
            c_end(viable_children.end()) ; c != c_end ; ++c)
    {
        try
        {
            Save<bool> save_t(&d->_imp->throw_on_blocker, true);
            Save<DepListOverrideMasks> save_o(&d->_imp->opts->override_masks, DepListOverrideMasks());
            d->add(*c);
            return;
        }
        catch (const DepListError &)
        {
        }
    }

    Log::get_instance()->message(ll_debug, lc_context, "No resolvable item in || ( ) block. Using "
            "first item for error message");
    {
        Context block_context("Inside || ( ) block with other options:");
        d->add(*viable_children.begin());
    }
}

void
DepList::AddVisitor::visit(const BlockDepAtom * const a)
{
    /* special case: the provider of virtual/blah can DEPEND upon !virtual/blah. */
    /* special case: foo/bar can DEPEND upon !foo/bar. */

    if (d->_imp->current_merge_list_entry != d->_imp->merge_list.end())
        if (d->_imp->current_merge_list_entry->kind == dlk_already_installed)
            return;

    if (! d->already_installed(a->blocked_atom(), true))
        return;

    Context context("When checking BlockDepAtom '!" + stringify(*a->blocked_atom()) + "':");

    switch (d->_imp->opts->blocks)
    {
        case dl_blocks_error:
            throw BlockError(stringify(*a->blocked_atom()));

        case dl_blocks_accumulate:
            if (d->_imp->throw_on_blocker)
                throw BlockError(stringify(*a->blocked_atom()));
            else
            {
                PackageDatabaseEntryCollection::ConstPointer m(d->_imp->env->package_database()->query(
                            *a->blocked_atom(), is_installed_only, qo_order_by_version));
                if (m->empty())
                {
                    /* this happens if we match an already on the list package, so always
                     * throw */
                    throw BlockError(stringify(*a->blocked_atom()));
                }
                else
                    for (PackageDatabaseEntryCollection::Iterator p(m->begin()), p_end(m->end()) ;
                            p != p_end ; ++p)
                        d->add_error_package(*p, dlk_block);
            }
            break;

        case dl_blocks_discard:
            Log::get_instance()->message(ll_warning, lc_context, "Discarding block '!"
                    + stringify(*a->blocked_atom()) + "'");
            break;

        case last_dl_blocks:
            ;
    }
}

struct DepList::ShowSuggestVisitor :
    DepAtomVisitorTypes::ConstVisitor,
    DepAtomVisitorTypes::ConstVisitor::VisitChildren<ShowSuggestVisitor, AllDepAtom>,
    DepAtomVisitorTypes::ConstVisitor::VisitChildren<ShowSuggestVisitor, AnyDepAtom>
{
    DepList * const d;

    ShowSuggestVisitor(DepList * const dd) :
        d(dd)
    {
    }

    void visit(const PlainTextDepAtom * const) PALUDIS_ATTRIBUTE((noreturn));
    void visit(const PackageDepAtom * const);
    void visit(const UseDepAtom * const);
    void visit(const BlockDepAtom * const);
    using DepAtomVisitorTypes::ConstVisitor::VisitChildren<ShowSuggestVisitor, AllDepAtom>::visit;
    using DepAtomVisitorTypes::ConstVisitor::VisitChildren<ShowSuggestVisitor, AnyDepAtom>::visit;
};

void
DepList::ShowSuggestVisitor::visit(const PlainTextDepAtom * const)
{
    throw InternalError(PALUDIS_HERE, "Got PlainTextDepAtom?");
}

void
DepList::ShowSuggestVisitor::visit(const UseDepAtom * const a)
{
    if (d->_imp->env->query_use(a->flag(), d->_imp->current_pde()) ^ a->inverse())
        std::for_each(a->begin(), a->end(), accept_visitor(this));
}

void
DepList::ShowSuggestVisitor::visit(const BlockDepAtom * const)
{
}

void
DepList::ShowSuggestVisitor::visit(const PackageDepAtom * const a)
{
    Context context("When adding suggested dep '" + stringify(*a) + "':");

    PackageDatabaseEntryCollection::ConstPointer matches(d->_imp->env->package_database()->query(
                *a, is_installable_only, qo_order_by_version));
    if (matches->empty())
    {
        Log::get_instance()->message(ll_warning, lc_context, "Nothing found for '" + stringify(*a) + "'");
        return;
    }

    for (PackageDatabaseEntryCollection::Iterator m(matches->begin()), m_end(matches->end()) ;
            m != m_end ; ++m)
    {
        if (d->_imp->env->mask_reasons(*m).any())
            continue;

        d->add_suggested_package(*m);
        return;
    }

    Log::get_instance()->message(ll_warning, lc_context, "Nothing visible found for '" + stringify(*a) + "'");
}

DepList::DepList(const Environment * const e, const DepListOptions & o) :
    PrivateImplementationPattern<DepList>(new Implementation<DepList>(e, o))
{
}

DepList::~DepList()
{
}

CountedPtr<DepListOptions, count_policy::ExternalCountTag>
DepList::options()
{
    return _imp->opts;
}

void
DepList::clear()
{
    DepListOptions o(*options());
    _imp.assign(new Implementation<DepList>(_imp->env, o));
}

void
DepList::add_in_role(DepAtom::ConstPointer atom, const std::string & role)
{
    Context context("When adding " + role + ":");
    add(atom);
}

void
DepList::add(DepAtom::ConstPointer atom)
{
    DepListTransaction transaction(_imp->merge_list, _imp->merge_list_index, _imp->merge_list_generation);

    Save<DepAtom::ConstPointer> save_current_top_level_target(&_imp->current_top_level_target,
            _imp->current_top_level_target ? _imp->current_top_level_target : atom);

    AddVisitor visitor(this);
    atom->accept(&visitor);
    transaction.commit();
}

void
DepList::add_package(const PackageDatabaseEntry & p, DepTag::ConstPointer tag)
{
    Context context("When adding package '" + stringify(p) + "':");

    Save<MergeList::iterator> save_merge_list_insert_position(&_imp->merge_list_insert_position);

    VersionMetadata::ConstPointer metadata(_imp->env->package_database()->fetch_repository(
                p.repository)->version_metadata(p.name, p.version));

    /* create our merge list entry. insert pre deps before ourself in the list. insert
     * post deps after ourself, and after any provides. */
    MergeList::iterator our_merge_entry_position(
            _imp->merge_list.insert(_imp->merge_list_insert_position,
                DepListEntry::create()
                .package(p)
                .metadata(metadata)
                .generation(_imp->merge_list_generation)
                .state(dle_no_deps)
                .tags(DepListEntryTags::Pointer(new DepListEntryTags::Concrete))
                .destinations(RepositoryNameCollection::Pointer(new RepositoryNameCollection::Concrete))
                .associated_entry(0)
                .kind(metadata->get_virtual_interface() ? dlk_virtual : dlk_package))),
        our_merge_entry_post_position(our_merge_entry_position);

    _imp->merge_list_index.insert(std::make_pair(p.name, our_merge_entry_position));

    if (tag)
        our_merge_entry_position->tags->insert(DepTagEntry::create()
                .generation(_imp->merge_list_generation)
                .tag(tag));

    if (_imp->opts->dependency_tags && _imp->current_pde())
        our_merge_entry_position->tags->insert(DepTagEntry::create()
                .tag(DepTag::Pointer(new DependencyDepTag(*_imp->current_pde())))
                .generation(_imp->merge_list_generation));

    Save<MergeList::const_iterator> save_current_merge_list_entry(&_imp->current_merge_list_entry,
            our_merge_entry_position);

    _imp->merge_list_insert_position = our_merge_entry_position;

    /* add provides */
    if (metadata->get_ebuild_interface())
    {
        DepAtomFlattener f(_imp->env, _imp->current_pde(), metadata->get_ebuild_interface()->provide());
        for (DepAtomFlattener::Iterator i(f.begin()), i_end(f.end()) ; i != i_end ; ++i)
        {
            PackageDepAtom::Pointer pp(new PackageDepAtom("=" + (*i)->text() + "-" + stringify(p.version)));

            std::pair<MergeListIndex::iterator, MergeListIndex::iterator> z(
                    _imp->merge_list_index.equal_range(pp->package()));
            MergeListIndex::iterator zz(std::find_if(z.first, z.second,
                MatchDepListEntryAgainstPackageDepAtom(_imp->env, pp.raw_pointer())));

            if (z.first != z.second)
                continue;

            VersionMetadata::ConstPointer m(0);

            if (_imp->env->package_database()->fetch_repository(RepositoryName("virtuals"))->has_version(
                        QualifiedPackageName((*i)->text()), p.version))
                m = _imp->env->package_database()->fetch_repository(RepositoryName("virtuals"))->version_metadata(
                        QualifiedPackageName((*i)->text()), p.version);
            else
            {
                VersionMetadata::Pointer mm(0);
                mm.assign(new VersionMetadata::Virtual(metadata->deps.parser,
                            PackageDatabaseEntry(p.name, p.version, RepositoryName("virtuals"))));
                mm->slot = metadata->slot;
                m = mm;
            }

            our_merge_entry_post_position = _imp->merge_list.insert(next(our_merge_entry_post_position),
                    DepListEntry(DepListEntry::create()
                        .package(PackageDatabaseEntry((*i)->text(), p.version, RepositoryName("virtuals")))
                        .metadata(m)
                        .generation(_imp->merge_list_generation)
                        .state(dle_has_all_deps)
                        .tags(DepListEntryTags::Pointer(new DepListEntryTags::Concrete))
                        .destinations(RepositoryNameCollection::Pointer(new RepositoryNameCollection::Concrete))
                        .associated_entry(&*_imp->current_merge_list_entry)
                        .kind(dlk_provided)));
            _imp->merge_list_index.insert(std::make_pair((*i)->text(), our_merge_entry_post_position));
        }
    }

    /* add suggests */
    if (_imp->opts->suggested == dl_suggested_show)
    {
        Context c("When showing suggestions:");
        Save<MergeList::iterator> suggest_save_merge_list_insert_position(&_imp->merge_list_insert_position,
                next(our_merge_entry_position));
        ShowSuggestVisitor visitor(this);
        metadata->deps.suggested_depend()->accept(&visitor);
    }

    /* add pre dependencies */
    add_predeps(metadata->deps.build_depend(), _imp->opts->uninstalled_deps_pre, "build");
    add_predeps(metadata->deps.run_depend(), _imp->opts->uninstalled_deps_runtime, "run");
    add_predeps(metadata->deps.post_depend(), _imp->opts->uninstalled_deps_post, "post");
    if (_imp->opts->suggested == dl_suggested_install)
        add_predeps(metadata->deps.suggested_depend(), _imp->opts->uninstalled_deps_suggested, "suggest");

    our_merge_entry_position->state = dle_has_pre_deps;
    _imp->merge_list_insert_position = next(our_merge_entry_post_position);

    /* add post dependencies */
    add_postdeps(metadata->deps.build_depend(), _imp->opts->uninstalled_deps_pre, "build");
    add_postdeps(metadata->deps.run_depend(), _imp->opts->uninstalled_deps_runtime, "run");
    add_postdeps(metadata->deps.post_depend(), _imp->opts->uninstalled_deps_post, "post");

    if (_imp->opts->suggested == dl_suggested_install)
        add_postdeps(metadata->deps.suggested_depend(), _imp->opts->uninstalled_deps_suggested, "suggest");

    our_merge_entry_position->state = dle_has_all_deps;
}

void
DepList::add_error_package(const PackageDatabaseEntry & p, const DepListEntryKind kind)
{
    std::pair<MergeListIndex::iterator, MergeListIndex::const_iterator> pp(
            _imp->merge_list_index.equal_range(p.name));

    for ( ; pp.second != pp.first ; ++pp.first)
    {
        if (pp.first->second->kind == kind && pp.first->second->package == p)
        {
            if (_imp->current_pde())
                pp.first->second->tags->insert(DepTagEntry::create()
                        .tag(DepTag::Pointer(new DependencyDepTag(*_imp->current_pde())))
                        .generation(_imp->merge_list_generation));
            return;
        }
    }

    MergeList::iterator our_merge_entry_position(
            _imp->merge_list.insert(_imp->merge_list.begin(),
                DepListEntry::create()
                .package(p)
                .metadata(_imp->env->package_database()->fetch_repository(
                        p.repository)->version_metadata(p.name, p.version))
                .generation(_imp->merge_list_generation)
                .state(dle_has_all_deps)
                .tags(DepListEntryTags::Pointer(new DepListEntryTags::Concrete))
                .destinations(RepositoryNameCollection::Pointer(new RepositoryNameCollection::Concrete))
                .associated_entry(&*_imp->current_merge_list_entry)
                .kind(kind)));

    if (_imp->current_pde())
        our_merge_entry_position->tags->insert(DepTagEntry::create()
                .tag(DepTag::Pointer(new DependencyDepTag(*_imp->current_pde())))
                .generation(_imp->merge_list_generation));

    _imp->merge_list_index.insert(std::make_pair(p.name, our_merge_entry_position));
}

void
DepList::add_suggested_package(const PackageDatabaseEntry & p)
{
    std::pair<MergeListIndex::iterator, MergeListIndex::const_iterator> pp(
            _imp->merge_list_index.equal_range(p.name));

    for ( ; pp.second != pp.first ; ++pp.first)
    {
        if ((pp.first->second->kind == dlk_suggested || pp.first->second->kind == dlk_already_installed
                    || pp.first->second->kind == dlk_package || pp.first->second->kind == dlk_provided
                    || pp.first->second->kind == dlk_subpackage) && pp.first->second->package == p)
            return;
    }

    MergeList::iterator our_merge_entry_position(
            _imp->merge_list.insert(_imp->merge_list_insert_position,
                DepListEntry::create()
                .package(p)
                .metadata(_imp->env->package_database()->fetch_repository(
                        p.repository)->version_metadata(p.name, p.version))
                .generation(_imp->merge_list_generation)
                .state(dle_has_all_deps)
                .tags(DepListEntryTags::Pointer(new DepListEntryTags::Concrete))
                .destinations(RepositoryNameCollection::Pointer(new RepositoryNameCollection::Concrete))
                .associated_entry(&*_imp->current_merge_list_entry)
                .kind(dlk_suggested)));

    if (_imp->current_pde())
        our_merge_entry_position->tags->insert(DepTagEntry::create()
                .tag(DepTag::Pointer(new DependencyDepTag(*_imp->current_pde())))
                .generation(_imp->merge_list_generation));

    _imp->merge_list_index.insert(std::make_pair(p.name, our_merge_entry_position));
}

void
DepList::add_predeps(DepAtom::ConstPointer d, const DepListDepsOption opt, const std::string & s)
{
    if (dl_deps_pre == opt || dl_deps_pre_or_post == opt)
    {
        try
        {
            add_in_role(d, s + " dependencies as pre dependencies");
        }
        catch (const DepListError & e)
        {
            if (dl_deps_pre == opt)
                throw;
            else
                Log::get_instance()->message(ll_warning, lc_context, "Dropping " + s + " dependencies to "
                        "post dependencies because of exception '" + e.message() + "' (" + e.what() + ")");
        }
    }
}

void
DepList::add_postdeps(DepAtom::ConstPointer d, const DepListDepsOption opt, const std::string & s)
{
    if (dl_deps_pre_or_post == opt || dl_deps_post == opt || dl_deps_try_post == opt)
    {
        try
        {
            try
            {
                add_in_role(d, s + " dependencies as post dependencies");
            }
            catch (const CircularDependencyError &)
            {
                Save<DepListCircularOption> save_circular(&_imp->opts->circular,
                        _imp->opts->circular == dl_circular_discard_silently ?
                        dl_circular_discard_silently : dl_circular_discard);
                Save<MergeList::iterator> save_merge_list_insert_position(&_imp->merge_list_insert_position,
                        _imp->merge_list.end());
                add_in_role(d, s + " dependencies as post dependencies with cycle breaking");
            }
        }
        catch (const DepListError & e)
        {
            if (dl_deps_try_post != opt)
                throw;
            else
                Log::get_instance()->message(ll_warning, lc_context, "Ignoring " + s +
                        " dependencies due to exception '" + e.message() + "' (" + e.what() + ")");
        }
    }
}

void
DepList::add_already_installed_package(const PackageDatabaseEntry & p, DepTag::ConstPointer tag)
{
    Context context("When adding installed package '" + stringify(p) + "':");

    Save<MergeList::iterator> save_merge_list_insert_position(&_imp->merge_list_insert_position);
    VersionMetadata::ConstPointer metadata(_imp->env->package_database()->fetch_repository(
                p.repository)->version_metadata(p.name, p.version));

    MergeList::iterator our_merge_entry(_imp->merge_list.insert(_imp->merge_list_insert_position,
                DepListEntry::create()
                .package(p)
                .metadata(metadata)
                .generation(_imp->merge_list_generation)
                .tags(DepListEntryTags::Pointer(new DepListEntryTags::Concrete))
                .state(dle_has_pre_deps)
                .destinations(RepositoryNameCollection::Pointer(new RepositoryNameCollection::Concrete))
                .associated_entry(0)
                .kind(dlk_already_installed)));
    _imp->merge_list_index.insert(std::make_pair(p.name, our_merge_entry));

    if (tag)
        our_merge_entry->tags->insert(DepTagEntry::create()
                .generation(_imp->merge_list_generation)
                .tag(tag));

    if (_imp->opts->dependency_tags && _imp->current_pde())
        our_merge_entry->tags->insert(DepTagEntry::create()
                .tag(DepTag::Pointer(new DependencyDepTag(*_imp->current_pde())))
                .generation(_imp->merge_list_generation));

    Save<MergeList::const_iterator> save_current_merge_list_entry(&_imp->current_merge_list_entry,
            our_merge_entry);

    add_predeps(metadata->deps.build_depend(), _imp->opts->installed_deps_pre, "build");
    add_predeps(metadata->deps.run_depend(), _imp->opts->installed_deps_runtime, "run");
    add_predeps(metadata->deps.post_depend(), _imp->opts->installed_deps_post, "post");

    our_merge_entry->state = dle_has_pre_deps;
    _imp->merge_list_insert_position = next(our_merge_entry);

    add_postdeps(metadata->deps.build_depend(), _imp->opts->installed_deps_pre, "build");
    add_postdeps(metadata->deps.run_depend(), _imp->opts->installed_deps_runtime, "run");
    add_postdeps(metadata->deps.post_depend(), _imp->opts->installed_deps_post, "post");
}

namespace
{
    bool is_scm(const QualifiedPackageName & n)
    {
        std::string pkg(stringify(n.package));
        switch (pkg.length())
        {
            case 0:
            case 1:
            case 2:
            case 3:
                return false;

            default:
                if (0 == pkg.compare(pkg.length() - 6, 6, "-darcs"))
                    return true;

            case 5:
                if (0 == pkg.compare(pkg.length() - 5, 5, "-live"))
                    return true;

            case 4:
                if (0 == pkg.compare(pkg.length() - 4, 4, "-cvs"))
                    return true;
                if (0 == pkg.compare(pkg.length() - 4, 4, "-svn"))
                    return true;
                return false;
        }
    }
}

bool
DepList::prefer_installed_over_uninstalled(const PackageDatabaseEntry & installed,
        const PackageDatabaseEntry & uninstalled)
{
    do
    {
        switch (_imp->opts->target_type)
        {
            case dl_target_package:
                if (! _imp->current_pde())
                    return false;

                if (is_top_level_target(uninstalled))
                    return false;

                continue;

            case dl_target_set:
                continue;

            case last_dl_target:
                ;
        }

        throw InternalError(PALUDIS_HERE, "Bad target_type value '" + stringify(_imp->opts->target_type) + "'");
    } while (false);

    if (dl_reinstall_always == _imp->opts->reinstall)
            return false;

    if (dl_upgrade_as_needed == _imp->opts->upgrade)
        return true;

    if (dl_reinstall_scm_never != _imp->opts->reinstall_scm)
        if (uninstalled.version == installed.version &&
                (installed.version.is_scm() || is_scm(installed.name)))
        {
            static time_t current_time(time(0)); /* static to avoid weirdness */
            time_t installed_time(_imp->env->package_database()->fetch_repository(installed.repository
                        )->installed_interface->installed_time(installed.name, installed.version));
            do
            {
                switch (_imp->opts->reinstall_scm)
                {
                    case dl_reinstall_scm_always:
                        return false;

                    case dl_reinstall_scm_daily:
                        if (current_time - installed_time > (24 * 60 * 60))
                            return false;
                        continue;

                    case dl_reinstall_scm_weekly:
                        if (current_time - installed_time > (24 * 60 * 60 * 7))
                            return false;
                        continue;

                    case dl_reinstall_scm_never:
                        ; /* nothing */

                    case last_dl_reinstall_scm:
                        ;
                }

                throw InternalError(PALUDIS_HERE, "Bad value for opts->reinstall_scm");
            } while (false);
        }

    /* use != rather than > to correctly force a downgrade when packages are
     * removed. */
    if (uninstalled.version != installed.version)
        return false;

    if (dl_reinstall_if_use_changed == _imp->opts->reinstall)
    {
        const EbuildVersionMetadata * const evm_i(_imp->env->package_database()->fetch_repository(
                    installed.repository)->version_metadata(installed.name, installed.version)->get_ebuild_interface());
        const EbuildVersionMetadata * const evm_u(_imp->env->package_database()->fetch_repository(
                    uninstalled.repository)->version_metadata(uninstalled.name, uninstalled.version)->get_ebuild_interface());

        std::set<std::string> use_i, use_u, use_common;
        if (evm_i)
            WhitespaceTokeniser::get_instance()->tokenise(evm_i->iuse, std::inserter(use_i, use_i.end()));
        if (evm_u)
            WhitespaceTokeniser::get_instance()->tokenise(evm_u->iuse, std::inserter(use_u, use_u.end()));

        std::set_intersection(use_i.begin(), use_i.end(), use_u.begin(), use_u.end(),
                std::inserter(use_common, use_common.end()));

        for (std::set<std::string>::const_iterator f(use_common.begin()), f_end(use_common.end()) ;
                f != f_end ; ++f)
            if (_imp->env->query_use(UseFlagName(*f), &installed) != _imp->env->query_use(UseFlagName(*f), &uninstalled))
                return false;
    }

    return true;
}

bool
DepList::already_installed(DepAtom::ConstPointer atom, const bool ignore_current_pde) const
{
    return already_installed(atom.raw_pointer(), ignore_current_pde);
}

bool
DepList::already_installed(const DepAtom * const atom, const bool ignore_current_pde) const
{
    QueryVisitor visitor(this, ignore_current_pde);
    atom->accept(&visitor);
    return visitor.result;
}

DepList::Iterator
DepList::begin() const
{
    return Iterator(_imp->merge_list.begin());
}

DepList::Iterator
DepList::end() const
{
    return Iterator(_imp->merge_list.end());
}

namespace
{
    struct IsTopLevelTarget :
        DepAtomVisitorTypes::ConstVisitor,
        std::unary_function<PackageDatabaseEntry, bool>
    {
        const Environment * const env;
        DepAtom::ConstPointer target;
        const PackageDatabaseEntry * dbe;
        bool matched;

        IsTopLevelTarget(const Environment * const e, DepAtom::ConstPointer t) :
            env(e),
            target(t),
            matched(false)
        {
        }

        bool operator() (const PackageDatabaseEntry & e)
        {
            dbe = &e;
            matched = false;
            target->accept(this);
            return matched;
        }

        void visit(const AllDepAtom * const a)
        {
            if (matched)
                return;

            std::for_each(a->begin(), a->end(), accept_visitor(this));
        }

        void visit(const PackageDepAtom * const a)
        {
            if (matched)
                return;

            if (match_package(env, a, *dbe))
                matched = true;
        }

        void visit(const UseDepAtom * const u)
        {
            if (matched)
                return;

            std::for_each(u->begin(), u->end(), accept_visitor(this));
        }

        void visit(const AnyDepAtom * const a)
        {
            if (matched)
                return;

            std::for_each(a->begin(), a->end(), accept_visitor(this));
        }

        void visit(const BlockDepAtom * const)
        {
        }

        void visit(const PlainTextDepAtom * const) PALUDIS_ATTRIBUTE((noreturn))
        {
            throw InternalError(PALUDIS_HERE, "Got PlainTextDepAtom?");
        }
    };

}

bool
DepList::is_top_level_target(const PackageDatabaseEntry & e) const
{
    if (! _imp->current_top_level_target)
        throw InternalError(PALUDIS_HERE, "current_top_level_target not set?");

    IsTopLevelTarget t(_imp->env, _imp->current_top_level_target);
    return t(e);
}

namespace
{
    struct IsError
    {
        bool operator() (const DepListEntry & e) const
        {
            switch (e.kind)
            {
                case dlk_virtual:
                case dlk_package:
                case dlk_provided:
                case dlk_already_installed:
                case dlk_subpackage:
                case dlk_suggested:
                    return false;

                case dlk_block:
                case dlk_masked:
                    return true;

                case last_dlk:
                    ;
            }

            throw InternalError(PALUDIS_HERE, "Bad e.kind");
        }
    };
}

bool
DepList::has_errors() const
{
    return end() != std::find_if(begin(), end(), IsError());
}

