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
#include <paludis/resolver/resolver_functions.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/resolver/resolutions.hh>
#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/qpn_s.hh>
#include <paludis/resolver/suggest_restart.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/options.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/map.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/repository_factory.hh>
#include <paludis/package_database.hh>
#include <test/test_runner.hh>
#include <test/test_framework.hh>
#include <list>
#include <tr1/functional>
#include <algorithm>
#include <map>

using namespace paludis;
using namespace paludis::resolver;
using namespace test;

namespace
{
    std::string from_keys(const std::tr1::shared_ptr<const Map<std::string, std::string> > & m,
            const std::string & k)
    {
        Map<std::string, std::string>::ConstIterator mm(m->find(k));
        if (m->end() == mm)
            return "";
        else
            return mm->second;
    }

    typedef std::map<QPN_S, std::tr1::shared_ptr<Constraints> > InitialConstraints;

    bool care_about_dep_fn(const QPN_S &, const std::tr1::shared_ptr<const Resolution> &, const SanitisedDependency &)
    {
        return true;
    }

    const std::tr1::shared_ptr<Constraints> initial_constraints_for_fn(
            const InitialConstraints & initial_constraints,
            const QPN_S & qpn_s)
    {
        InitialConstraints::const_iterator i(initial_constraints.find(qpn_s));
        if (i == initial_constraints.end())
            return make_shared_ptr(new Constraints);
        else
            return i->second;
    }

    std::tr1::shared_ptr<QPN_S_Sequence> get_qpn_s_s_for_fn(const PackageDepSpec & spec,
            const std::tr1::shared_ptr<const Reason> &)
    {
        std::tr1::shared_ptr<QPN_S_Sequence> result(new QPN_S_Sequence);
        result->push_back(QPN_S(spec, make_shared_ptr(new SlotName("0"))));
        return result;
    }

    struct IsSuggestionVisitor
    {
        bool is_suggestion;

        IsSuggestionVisitor() :
            is_suggestion(true)
        {
        }

        void visit(const DependencyRequiredLabel &)
        {
            is_suggestion = false;
        }

        void visit(const DependencyRecommendedLabel &)
        {
            is_suggestion = false;
        }

        void visit(const DependencySuggestedLabel &)
        {
        }
    };

    bool is_suggestion(const SanitisedDependency & dep)
    {
        if (dep.active_dependency_labels()->suggest_labels()->empty())
            return false;

        IsSuggestionVisitor v;
        std::for_each(indirect_iterator(dep.active_dependency_labels()->suggest_labels()->begin()),
                indirect_iterator(dep.active_dependency_labels()->suggest_labels()->end()),
                accept_visitor(v));
        return v.is_suggestion;
    }

    bool take_dependency_fn(
            const QPN_S &,
            const SanitisedDependency & dep,
            const std::tr1::shared_ptr<const Reason> &)
    {
        return ! is_suggestion(dep);
    }

    UseInstalled get_use_installed_fn(
            const QPN_S &,
            const PackageDepSpec &,
            const std::tr1::shared_ptr<const Reason> &)
    {
        return ui_never;
    }

    struct ResolverSuggestionsTestCase : TestCase
    {
        TestEnvironment env;
        std::tr1::shared_ptr<Repository> repo, inst_repo;

        ResolverSuggestionsTestCase(const std::string & s) :
            TestCase(s)
        {
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "exheres");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "resolver_TEST_suggestions_dir" / "repo"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "resolver_TEST_suggestions_dir" / "repo/profiles/profile"));
            keys->insert("layout", "exheres");
            keys->insert("eapi_when_unknown", "exheres-0");
            keys->insert("eapi_when_unspecified", "exheres-0");
            keys->insert("profile_eapi", "exheres-0");
            keys->insert("distdir", stringify(FSEntry::cwd() / "resolver_TEST_suggestions_dir" / "distdir"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "resolver_TEST_suggestions_dir" / "build"));
            repo = RepositoryFactory::get_instance()->create(&env,
                    std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1));
            env.package_database()->add_repository(1, repo);

            keys.reset(new Map<std::string, std::string>);
            keys->insert("format", "exndbam");
            keys->insert("names_cache", "/var/empty");
            keys->insert("provides_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "resolver_TEST_suggestions_dir" / "installed"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "resolver_TEST_suggestions_dir" / "build"));
            inst_repo = RepositoryFactory::get_instance()->create(&env,
                    std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1));
            env.package_database()->add_repository(1, inst_repo);
        }

        const std::tr1::shared_ptr<const ResolutionLists> get_resolutions(const PackageDepSpec & target)
        {
            InitialConstraints initial_constraints;

            while (true)
            {
                try
                {
                    Resolver resolver(&env, make_named_values<ResolverFunctions>(
                                value_for<n::care_about_dep_fn>(&care_about_dep_fn),
                                value_for<n::get_initial_constraints_for_fn>(
                                    std::tr1::bind(&initial_constraints_for_fn, std::tr1::ref(initial_constraints),
                                        std::tr1::placeholders::_1)),
                                value_for<n::get_qpn_s_s_for_fn>(&get_qpn_s_s_for_fn),
                                value_for<n::get_use_installed_fn>(&get_use_installed_fn),
                                value_for<n::take_dependency_fn>(&take_dependency_fn)
                                ));
                    resolver.add_target(target);
                    resolver.resolve();
                    return resolver.resolution_lists();
                }
                catch (const SuggestRestart & e)
                {
                    initial_constraints.insert(std::make_pair(e.qpn_s(), make_shared_ptr(new Constraints))).first->second->add(
                            e.suggested_preset());
                }
            }
        }

        const std::tr1::shared_ptr<const ResolutionLists> get_resolutions(const std::string & target)
        {
            PackageDepSpec target_spec(parse_user_package_dep_spec(target, &env, UserPackageDepSpecOptions()));
            return get_resolutions(target_spec);
        }

        struct ResolutionListChecks
        {
            typedef std::tr1::function<bool (const std::tr1::shared_ptr<const Resolution> &) > CheckFunction;
            typedef std::tr1::function<std::string (const std::tr1::shared_ptr<const Resolution> &) > MessageFunction;
            typedef std::list<std::pair<CheckFunction, MessageFunction> > List;
            List checks;

            static bool check_qpn(const QualifiedPackageName & q, const std::tr1::shared_ptr<const Resolution> & r)
            {
                if ((! r) || (! r->decision()) || (! r->decision()->if_package_id()))
                    return false;

                return r->decision()->if_package_id()->name() == q;
            }

            static std::string check_generic_msg(const std::string & q, const std::tr1::shared_ptr<const Resolution> & r)
            {
                if (! r)
                    return "Expected " + stringify(q) + " but got finished";
                else if (! r->decision())
                    return "Expected " + stringify(q) + " but got undecided for " + stringify(r->qpn_s());
                else if (! r->decision()->if_package_id())
                    return "Expected " + stringify(q) + " but got decided nothing (kind " + stringify(r->decision()->kind()) + ") for "
                        + stringify(r->qpn_s());
                else
                    return "Expected " + stringify(q) + " but got " + stringify(r->decision()->if_package_id()->name()) + " for "
                        + stringify(r->qpn_s());
            }

            static std::string check_qpn_msg(const QualifiedPackageName & q, const std::tr1::shared_ptr<const Resolution> & r)
            {
                return check_generic_msg(stringify(q), r);
            }

            static bool check_finished(const std::tr1::shared_ptr<const Resolution> & r)
            {
                return ! r;
            }

            static std::string check_finished_msg(const std::tr1::shared_ptr<const Resolution> & r)
            {
                return check_generic_msg("finished", r);
            }

            ResolutionListChecks & qpn(const QualifiedPackageName & q)
            {
                checks.push_back(std::make_pair(
                            std::tr1::bind(&check_qpn, q, std::tr1::placeholders::_1),
                            std::tr1::bind(&check_qpn_msg, q, std::tr1::placeholders::_1)
                            ));
                return *this;
            }

            ResolutionListChecks & finished()
            {
                checks.push_back(std::make_pair(
                            &check_finished,
                            &check_finished_msg
                            ));
                return *this;
            }
        };

        void check_resolution_list(const std::tr1::shared_ptr<const Resolutions> & list, const ResolutionListChecks & checks)
        {
            Resolutions::ConstIterator r(list->begin()), r_end(list->end());
            for (ResolutionListChecks::List::const_iterator c(checks.checks.begin()), c_end(checks.checks.end()) ;
                    c != c_end ; ++c)
            {
                if (r == r_end)
                    TEST_CHECK_MESSAGE(c->first(make_null_shared_ptr()), c->second(make_null_shared_ptr()));
                else
                    TEST_CHECK_MESSAGE(c->first(*r), c->second(*r));
                ++r;
            }
        }
    };
}

namespace test_cases
{
    struct TestSuggestionThenDependency : ResolverSuggestionsTestCase
    {
        TestSuggestionThenDependency() : ResolverSuggestionsTestCase("suggestion then dependency") { }

        void run()
        {
            std::tr1::shared_ptr<const ResolutionLists> resolutions(get_resolutions("suggestion-then-dependency/target"));

            {
                TestMessageSuffix s("errors");
                check_resolution_list(resolutions->errors(), ResolutionListChecks()
                        .finished()
                        );
            }

            {
                TestMessageSuffix s("ordered");
                check_resolution_list(resolutions->ordered(), ResolutionListChecks()
                        .qpn(QualifiedPackageName("suggestion-then-dependency/a-suggested-dep"))
                        .qpn(QualifiedPackageName("suggestion-then-dependency/hard-dep"))
                        .qpn(QualifiedPackageName("suggestion-then-dependency/target"))
                        .finished()
                        );
            }

            {
                TestMessageSuffix s("untaken");
                check_resolution_list(resolutions->untaken(), ResolutionListChecks()
                        .finished()
                        );
            }
        }
    } test_suggestion_then_dependency;
}

