/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011, 2012 Ciaran McCreesh
 * Copyright (c) 2015 David Leverton
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

#include <paludis/util/run_test.hh>

#include <paludis/util/visitor_cast.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/generator.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>
#include <paludis/choice.hh>

#include <paludis/repositories/e/dep_parser.hh>
#include <paludis/repositories/e/eapi.hh>

#include <algorithm>

#include "config.h"

#include <paludis/unformatted_pretty_printer.hh>
#include <paludis/formatted_pretty_printer.hh>
#include <paludis/repositories/e/spec_tree_pretty_printer.hh>
#include <paludis/repositories/e/required_use_verifier.hh>

#include <src/clients/cave/colour_pretty_printer.hh>
#include <cmath>

namespace
{
    using namespace paludis;

    class ERepository7 : public Test
    {
    };

    struct UseFlagParams {
        bool a;
        bool b;
        bool foo;
        bool bar;
        bool result;
    };

    struct SelectorsTest : public ERepository7, public testing::WithParamInterface<UseFlagParams>
    {
        bool a;
        bool b;
        bool foo;
        bool bar;
        bool result;

        void SetUp() override
        {
            ERepository7::SetUp();
            a = GetParam().a;
            b = GetParam().b;
            foo = GetParam().foo;
            bar = GetParam().bar;
            result = GetParam().result;
        }
    };

    struct SelectorsAnyOf : public SelectorsTest{};
    struct SelectorsOneOf : public SelectorsTest{};

    TEST_F(ERepository7, assert_in_subshell)
    {
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/assert-in-subshell-7",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("7", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_THROW(id->perform_action(*install_action), ActionFailedError);
    }

    TEST_F(ERepository7, best_version)
    {
        auto repos = env->repositories();
        auto installed_repo(std::static_pointer_cast<FakeInstalledRepository>(*std::find_if(repos.begin(), repos.end(), [](const std::shared_ptr<Repository> cur_repo){return (cur_repo->name().value()=="installed");})));
        installed_repo->add_version("cat", "best-version-pretend-installed", "0");
        installed_repo->add_version("cat", "best-version-pretend-installed", "1");

        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/best-version-7",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("7", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*pretend_action);
        EXPECT_TRUE(! pretend_action->failed());
    }

    TEST_F(ERepository7, die_in_subshell)
    {
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/die-in-subshell-7",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("7", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_THROW(id->perform_action(*install_action), ActionFailedError);
    }

    TEST_F(ERepository7, banned_dohtml)
    {
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/banned-dohtml-7",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("7", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_THROW(id->perform_action(*install_action), ActionFailedError);
    }

    TEST_F(ERepository7, banned_dolib)
    {
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/banned-dolib-7",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("7", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_THROW(id->perform_action(*install_action), ActionFailedError);
    }

    TEST_F(ERepository7, banned_dolib_rep)
    {
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/banned-dolib-rep-7",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("7", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_NO_THROW(id->perform_action(*install_action));
    }

    TEST_F(ERepository7,changed_domo)
    {
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/changed-domo-7",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("7", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_NO_THROW(id->perform_action(*install_action));
    }

    TEST_F(ERepository7,added_dostrip)
    {
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/added-dostrip-7",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("7", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_NO_THROW(id->perform_action(*install_action));
    }

    TEST_F(ERepository7, added_dostrip_strip_restrict)
    {
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/added-dostrip-strip-restrict-7",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("7", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_NO_THROW(id->perform_action(*install_action));
    }

    TEST_F(ERepository7,eapply_git_diff_support)
    {
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/eapply-git-diff-support-7",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("7", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_NO_THROW(id->perform_action(*install_action));
    }

    TEST_F(ERepository7,eapply_user_git_diff_support)
    {
        setenv("PALUDIS_USER_PATCHES", stringify(test_dir / "e_repository_TEST_7_dir/root/var/paludis/user_patches").c_str(), 1);
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/eapply-user-git-diff-support-7",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("7", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_NO_THROW(id->perform_action(*install_action));
        unsetenv("PALUDIS_USER_PATCHES");
    }

    TEST_F(ERepository7,ebegin_not_to_stdout)
    {
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/ebegin-not-to-stdout-7",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("7", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*pretend_action);
        EXPECT_TRUE(! pretend_action->failed());
    }

    TEST_F(ERepository7,econf_added_options)
    {
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                      PackageDepSpec(parse_user_package_dep_spec("=cat/econf-added-options-7",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("7", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_NO_THROW(id->perform_action(*install_action));
    }

    TEST_F(ERepository7,eend_not_to_stdout)
    {
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                      PackageDepSpec(parse_user_package_dep_spec("=cat/eend-not-to-stdout-7",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("7", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*pretend_action);
        EXPECT_TRUE(! pretend_action->failed());
    }

    TEST_F(ERepository7,einfo_not_to_stdout)
      {
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                      PackageDepSpec(parse_user_package_dep_spec("=cat/einfo-not-to-stdout-7",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("7", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*pretend_action);
        EXPECT_TRUE(! pretend_action->failed());
    }

    TEST_F(ERepository7,eqawarn_not_to_stdout)
    {
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                      PackageDepSpec(parse_user_package_dep_spec("=cat/eqawarn-not-to-stdout-7",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("7", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*pretend_action);
        EXPECT_TRUE(! pretend_action->failed());
    }

    TEST_F(ERepository7, has_version)
    {
        auto repos = env->repositories();
        auto installed_repo(std::static_pointer_cast<FakeInstalledRepository>(*std::find_if(repos.begin(), repos.end(), [](const std::shared_ptr<Repository> cur_repo){return (cur_repo->name().value()=="installed");})));
        installed_repo->add_version("cat", "has-version-pretend-installed", "0");
        installed_repo->add_version("cat", "has-version-pretend-installed", "1");

        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/has-version-7",
                                env.get(),{ })),nullptr,{ }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("7", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*pretend_action);
        EXPECT_TRUE(! pretend_action->failed());
    }

    TEST_F(ERepository7, banned_libopts)
    {
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/banned-libopts-7",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("7", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_THROW(id->perform_action(*install_action), ActionFailedError);
    }

    TEST_F(ERepository7, nonfatal_external_and_function)
    {
        env->set_want_choice_enabled(ChoicePrefixName("build_options"), UnprefixedChoiceName("optional_tests"), true);

        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/nonfatal-external-and-function-7",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("7", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_TRUE(!!id->choices_key()->parse_value()->find_by_name_with_prefix(ChoiceNameWithPrefix("build_options:optional_tests")));
        EXPECT_NO_THROW(id->perform_action(*install_action));

        env->set_want_choice_enabled(ChoicePrefixName("build_options"), UnprefixedChoiceName("optional_tests"), false);
    }

    TEST_F(ERepository7, removed_eclassdir)
    {
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/removed-eclassdir-7",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("7", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_NO_THROW(id->perform_action(*install_action));
    }

    TEST_F(ERepository7, vers)
    {
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/vers-7",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("7", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*pretend_action);
        EXPECT_TRUE(! pretend_action->failed());
    }

    TEST_F(ERepository7, no_trail_slash)
    {
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/no-trail-slash-7",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("7", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_NO_THROW(id->perform_action(*install_action));
    }

    TEST_F(ERepository7, prefix)
    {
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/prefix-7",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("7", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_NO_THROW(id->perform_action(*install_action));
    }

    TEST_F(ERepository7, changed_vars)
    {
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/changed-vars-7",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("7", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_NO_THROW(id->perform_action(*install_action));
    }

    TEST_F(ERepository7, env_unset)
    {
        setenv("PARENT_VAR_TO_UNSET", "foo", 1);
        setenv("CHILD_VAR_TO_UNSET", "bar", 1);
        setenv("PARENT_VAR_TO_VANISH", "baz", 1);
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/env-unset-7",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("7", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_NO_THROW(id->perform_action(*install_action));
        unsetenv("PARENT_VAR_TO_UNSET");
        unsetenv("CHILD_VAR_TO_UNSET");
        unsetenv("PARENT_VAR_TO_VANISH");
    }

    TEST_F(ERepository7, selectors_dep)
    {
        using namespace paludis;
        using namespace paludis::erepository;

        for (std::string eapi: {"6", "7"})
        {
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/selectors-dep-"+eapi,
                                env.get(), { })), nullptr, { }))]->last());

        ASSERT_TRUE(bool(id));
        EXPECT_EQ(eapi, visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());

        const auto erepoid(std::static_pointer_cast<const erepository::ERepositoryID>(id));

        // from paludis::resolver::dependent_upon()
        if (id->dependencies_key())
        {
            std::shared_ptr<const DependencySpecTree> depends;
            if (erepoid->eapi()->supported()->dependency_spec_tree_parse_options()[dstpo_disallow_empty_dep_group])
            {
                EXPECT_THROW(depends = id->dependencies_key()->parse_value(), EDepParseError);
                EXPECT_THROW(parse_depend("|| ( )", env.get(), *EAPIData::get_instance()->eapi_from_string(eapi), false), EDepParseError);
            }
            else
            {
                EXPECT_NO_THROW(depends = id->dependencies_key()->parse_value() );
                EXPECT_NO_THROW(parse_depend("|| ( )", env.get(), *EAPIData::get_instance()->eapi_from_string(eapi), false));
            }
        }
        else
        {
            for (auto& fn : { &PackageID::build_dependencies_target_key, &PackageID::build_dependencies_host_key,
                &PackageID::run_dependencies_target_key, &PackageID::run_dependencies_host_key,
                &PackageID::post_dependencies_key })
            {
                auto key(((*id).*fn)());
                if (key)
                {
                    std::shared_ptr<const DependencySpecTree> depends;
                    if (erepoid->eapi()->supported()->dependency_spec_tree_parse_options()[dstpo_disallow_empty_dep_group])
                    {
                        EXPECT_THROW(depends = key->parse_value(), EDepParseError);
                        EXPECT_THROW(parse_depend("|| ( )", env.get(), *EAPIData::get_instance()->eapi_from_string(eapi), false), EDepParseError);
                    }
                    else
                    {
                        EXPECT_NO_THROW(depends = key->parse_value() );
                        EXPECT_NO_THROW(parse_depend("|| ( )", env.get(), *EAPIData::get_instance()->eapi_from_string(eapi), false));
                    }
                }
            }
        }
        }

        for (const std::string& eapi_name: EAPIData::get_instance()->known_eapis())
        {
            auto eapi(EAPIData::get_instance()->eapi_from_string(eapi_name));
            if (eapi->supported()->dependency_spec_tree_parse_options()[dstpo_disallow_empty_dep_group])
                EXPECT_THROW(   parse_depend("|| ( )", env.get(), *eapi, false), EDepParseError);
            else
                EXPECT_NO_THROW(parse_depend("|| ( )", env.get(), *eapi, false));
        }
    }

    TEST_P(SelectorsAnyOf, or)
    {
        env->set_want_choice_enabled(ChoicePrefixName(""), UnprefixedChoiceName("a"), a);
        env->set_want_choice_enabled(ChoicePrefixName(""), UnprefixedChoiceName("b"), b);
        env->set_want_choice_enabled(ChoicePrefixName(""), UnprefixedChoiceName("foo"), foo);
        env->set_want_choice_enabled(ChoicePrefixName(""), UnprefixedChoiceName("bar"), bar);

        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/selectors-or-7",
                                env.get(), { })), nullptr, { }))]->last());

        ASSERT_TRUE(bool(id));
        EXPECT_EQ("7", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_EQ(id->choices_key()->parse_value()->find_by_name_with_prefix(ChoiceNameWithPrefix("a"))->enabled(), a);
        EXPECT_EQ(id->choices_key()->parse_value()->find_by_name_with_prefix(ChoiceNameWithPrefix("b"))->enabled(), b);
        EXPECT_EQ(id->choices_key()->parse_value()->find_by_name_with_prefix(ChoiceNameWithPrefix("foo"))->enabled(), foo);
        EXPECT_EQ(id->choices_key()->parse_value()->find_by_name_with_prefix(ChoiceNameWithPrefix("bar"))->enabled(), bar);
        id->perform_action(*pretend_action);
        EXPECT_EQ(! pretend_action->failed(), ((a&foo)|(b&bar )));

        env->set_want_choice_enabled(ChoicePrefixName(""), UnprefixedChoiceName("a"), false);
        env->set_want_choice_enabled(ChoicePrefixName(""), UnprefixedChoiceName("b"), false);
        env->set_want_choice_enabled(ChoicePrefixName(""), UnprefixedChoiceName("foo"), false);
        env->set_want_choice_enabled(ChoicePrefixName(""), UnprefixedChoiceName("bar"), false);
    }

    TEST_P(SelectorsOneOf, xor)
    {
        env->set_want_choice_enabled(ChoicePrefixName(""), UnprefixedChoiceName("a"), a);
        env->set_want_choice_enabled(ChoicePrefixName(""), UnprefixedChoiceName("b"), b);
        env->set_want_choice_enabled(ChoicePrefixName(""), UnprefixedChoiceName("foo"), foo);
        env->set_want_choice_enabled(ChoicePrefixName(""), UnprefixedChoiceName("bar"), bar);

        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/selectors-xor-7",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("7", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_EQ(id->choices_key()->parse_value()->find_by_name_with_prefix(ChoiceNameWithPrefix("a"))->enabled(), a);
        EXPECT_EQ(id->choices_key()->parse_value()->find_by_name_with_prefix(ChoiceNameWithPrefix("b"))->enabled(), b);
        EXPECT_EQ(id->choices_key()->parse_value()->find_by_name_with_prefix(ChoiceNameWithPrefix("foo"))->enabled(), foo);
        EXPECT_EQ(id->choices_key()->parse_value()->find_by_name_with_prefix(ChoiceNameWithPrefix("bar"))->enabled(), bar);
        EXPECT_EQ("7", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*pretend_action);
        EXPECT_EQ(! pretend_action->failed(), ((a&foo)^(b&bar )) );

        env->set_want_choice_enabled(ChoicePrefixName(""), UnprefixedChoiceName("a"), false);
        env->set_want_choice_enabled(ChoicePrefixName(""), UnprefixedChoiceName("b"), false);
        env->set_want_choice_enabled(ChoicePrefixName(""), UnprefixedChoiceName("foo"), false);
        env->set_want_choice_enabled(ChoicePrefixName(""), UnprefixedChoiceName("bar"), false);
    }

    TEST_F(ERepository7, dep_group_test)
    {
        using namespace paludis::erepository;
        std::vector<bool> choices({true,true,true,true,true,true,true,true});
        FSPath cwd = FSPath::cwd();
        env->set_want_choice_enabled(ChoicePrefixName(""), UnprefixedChoiceName("a"), Tribool(choices[0]));
        env->set_want_choice_enabled(ChoicePrefixName(""), UnprefixedChoiceName("b"), Tribool(choices[1]));
        env->set_want_choice_enabled(ChoicePrefixName(""), UnprefixedChoiceName("c"), Tribool(choices[2]));
        env->set_want_choice_enabled(ChoicePrefixName(""), UnprefixedChoiceName("d"), Tribool(choices[3]));
        env->set_want_choice_enabled(ChoicePrefixName(""), UnprefixedChoiceName("e"), Tribool(choices[4]));
        env->set_want_choice_enabled(ChoicePrefixName(""), UnprefixedChoiceName("f"), Tribool(choices[5]));
        env->set_want_choice_enabled(ChoicePrefixName(""), UnprefixedChoiceName("g"), Tribool(choices[6]));
        env->set_want_choice_enabled(ChoicePrefixName(""), UnprefixedChoiceName("h"), Tribool(choices[7]));

        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
            PackageDepSpec(parse_user_package_dep_spec("=cat/dep-group-test-7",
                                                       env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("7", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());

        EXPECT_EQ(id->choices_key()->parse_value()->find_by_name_with_prefix(ChoiceNameWithPrefix("a"))->enabled(), choices[0]);
        EXPECT_EQ(id->choices_key()->parse_value()->find_by_name_with_prefix(ChoiceNameWithPrefix("b"))->enabled(), choices[1]);
        EXPECT_EQ(id->choices_key()->parse_value()->find_by_name_with_prefix(ChoiceNameWithPrefix("c"))->enabled(), choices[2]);
        EXPECT_EQ(id->choices_key()->parse_value()->find_by_name_with_prefix(ChoiceNameWithPrefix("d"))->enabled(), choices[3]);
        EXPECT_EQ(id->choices_key()->parse_value()->find_by_name_with_prefix(ChoiceNameWithPrefix("e"))->enabled(), choices[4]);
        EXPECT_EQ(id->choices_key()->parse_value()->find_by_name_with_prefix(ChoiceNameWithPrefix("f"))->enabled(), choices[5]);
        EXPECT_EQ(id->choices_key()->parse_value()->find_by_name_with_prefix(ChoiceNameWithPrefix("g"))->enabled(), choices[6]);
        EXPECT_EQ(id->choices_key()->parse_value()->find_by_name_with_prefix(ChoiceNameWithPrefix("h"))->enabled(), choices[7]);

        const auto erepoid(std::static_pointer_cast<const erepository::ERepositoryID>(id));
        const auto ebuildid(std::static_pointer_cast<const erepository::EbuildID>(id));
        std::cout << (erepoid == ebuildid ) << std::endl;
        /*
         * all-of group
         * "( a b )" or "a b" => a & b
         * any-of group
         * "||( a b )" => a | b
         * exactly-one-of group
         * "^^( a b )" => a ^ b
         * at-most-one-of group
         * "??( a b )" => ~a & ~b
         * use-conditional group
         * "a? ( b )" => ~a | b
         */

        struct dep{
            dep(bool _r, const std::string& _s, const std::string& _t = "text"):r(_r),s(_s),t(_t){}
            dep(const dep& o):r(o.r),s(o.s),t(o.t){}
            /** return value */
            bool r;
            /** value */
            std::string s;
            /** type */
            std::string t;
        };

        /** represents a text dependency */
        [[maybe_unused]] auto t = [](const std::string& arg)->std::shared_ptr<dep>{
            if(arg.find_first_not_of(" \t\r\n") != 0)
                throw EDepParseError(arg, "invalid std::shared_ptr<dep>");
            if(arg.at(0) == '!')
                return std::make_shared<dep>(false, arg,"text");
            return std::make_shared<dep>(true, arg, "text");
        };

        /**
         * represents a plain group or top level group
         * "first second third "
         */
        std::function<std::shared_ptr<dep>(const std::list<std::shared_ptr<dep>>&)> p              =  [](const std::list<std::shared_ptr<dep>>& args)->std::shared_ptr<dep> {
            bool ret = true;
            std::string s;

            for(const auto& arg: args)
            {
                ret &= arg->r;
                s += (s.empty()?"":" ");
                if(arg->t=="text" && !arg->r)
                    s += "!";
                s += arg->s;
            }
            return std::make_shared<dep>(ret, s, "plain");
        };

        /**
         * Represents a all-of group
         * Represents a ( first second third ) group of dependency
         */
        std::function<std::shared_ptr<dep>(const std::list<std::shared_ptr<dep>>&)> all_of         =  [](const std::list<std::shared_ptr<dep>>& args)->std::shared_ptr<dep> {
            bool ret = true;
            std::string s="( ";
            for(const auto& arg: args)
            {
                if((arg->t != "use-cond") || arg->s.at(0)!='!')
                    ret &= arg->r;
                s += arg->s + " ";
            }
            return std::make_shared<dep>(ret, s+")", "all-of");
        };

        /**
         * Represents a any-of group
         * "|| ( first second third  )"
         */
        std::function<std::shared_ptr<dep>(const std::list<std::shared_ptr<dep>>&)> any_of         =  [](const std::list<std::shared_ptr<dep>>& args)->std::shared_ptr<dep> {
            bool ret = false;
            std::string s="|| ( ";
            for(const auto& arg: args)
            {
                if((arg->t != "use-cond") || arg->s.at(0)!='!')
                    ret |= arg->r;
                s += arg->s + " ";
            }
            return std::make_shared<dep>(ret, s+")", "any-of");
        };

        /**
         * represents a exactly-one-of group
         * "^^ ( first second third )"
         */
        std::function<std::shared_ptr<dep>(const std::list<std::shared_ptr<dep>>&)> exactly_one_of =  [](const std::list<std::shared_ptr<dep>>& args)->std::shared_ptr<dep> {
            int ret = 0;
            std::string s="^^ ( ";
            for(const auto& arg: args)
            {
                if((arg->t != "use-cond") || arg->s.at(0)!='!')
                    ret += arg->r;
                s += arg->s + " ";
            }
            return std::make_shared<dep>((ret==1), s+")", "exactly-one-of");
        };

        /**
         * Represents a at-most-one-of group
         * "?? ( first second third )"
         */
        std::function<std::shared_ptr<dep>(const std::list<std::shared_ptr<dep>>&)> at_most_one_of =  [](const std::list<std::shared_ptr<dep>>& args)->std::shared_ptr<dep> {
            int ret = 0;
            std::string s="\?\? ( ";
            for(const auto& arg: args)
            {
                if((arg->t != "use-cond") || arg->s.at(0)!='!')
                    ret += arg->r;
                s += arg->s + " ";
            }
            return std::make_shared<dep>(ret<=1, s+")", "at-most-one-of");
        };

        /**
         * Represents a std::shared_ptrdependency spec whose children should only be considered
         * upon a certain condition (for example, a use dependency block).
         * "first? ( second third )"
         */
        std::function<std::shared_ptr<dep>(const std::list<std::shared_ptr<dep>>&)> use_conditional = [](const std::list<std::shared_ptr<dep>>& args)->std::shared_ptr<dep> {
            if( args.size() < 2)
                throw std::runtime_error("wrong amount of args for use cond (got only one)");

            bool ret = true;
            std::string s=args.front()->s+"? ( ";
            for(auto arg = ++args.begin(); arg != args.end();++arg)
            {
                ret &= (*arg)->r;
                s += (*arg)->s + " ";
            }
            ret = ((!args.front()->r) | ret);
            return std::make_shared<dep>(ret, s+")", "use-cond");
        };

        const std::string red="\033[1;31m";
        const std::string green="\033[1;32m";
        std::cout << std::boolalpha;

        std::list<std::shared_ptr<dep>> tests;

        std::vector<std::string> vars({"a","b","c","d","e"});

        auto fn_outer = {
            &p,
            &all_of,
            &any_of,
            &exactly_one_of,
            &at_most_one_of,
            &use_conditional,
        };

        auto fn_inner = {
            &all_of,
            &any_of,
            &exactly_one_of,
            &at_most_one_of,
            &use_conditional,
        };

        for(auto& fn: fn_outer)
        {
            for(std::size_t m = 0; m < (1 << vars.size()); ++m)
            {
                std::list<std::shared_ptr<dep>> plain_vars;

                for(std::size_t n = 0; n < vars.size();++n)
                    plain_vars.push_back(p({  std::make_shared<dep>(!(m>>n&0x1),vars.at(n))}));

                for(auto& fn2: fn_inner)
                {
                    for(std::size_t k=1;k<=(plain_vars.size()-2);++k)
                    {
                        std::list<std::shared_ptr<dep>> outer(plain_vars);
                        std::list<std::shared_ptr<dep>> inner;
                        auto it = outer.begin();
                        std::advance(it,k);
                        inner.splice(inner.begin(), outer, it, outer.end());
                        auto tmp = (*fn2)(inner);
                        outer.push_back(tmp);
                        auto tmp2 = (*fn)(outer);
                        tests.push_back(tmp2);
                    }
                }
            }
        }

        const std::size_t end = 4;
        for( auto& fn: {&any_of,&exactly_one_of})
        {
            for(std::size_t m = 0; m < (1 << end); ++m)
            {
                std::list<std::shared_ptr<dep>> plain_vars;

                for(std::size_t n = 0; n < end;++n)
                    plain_vars.push_back(p({  std::make_shared<dep>(!(m>>n&0x1),vars.at(n))}));

                auto front(plain_vars);
                std::list<std::shared_ptr<dep>> back;
                auto it = front.begin();
                std::advance(it,2);
                back.splice(back.begin(), front, it, front.end());
                tests.push_back((*fn)({use_conditional(front),use_conditional(back)}));
            }
        }

        for(const auto& test: tests)
        {
            auto rqu = std::static_pointer_cast<const RequiredUseSpecTree >( parse_required_use(test->s, env.get(), *ebuildid->eapi(), false));

            RequiredUseVerifier verifier(env.get(), ebuildid);

            rqu->top()->accept(verifier);

            UnformattedPrettyPrinter ff;
            paludis::erepository::SpecTreePrettyPrinter a(ff, {});
            rqu->top()->accept(a);

            if( ( (verifier.unmet_requirements() && verifier.unmet_requirements()->empty())) != test->r)
                std::cout << std::left << std::setw(20) << test->t << std::right << std::setw(5) << test->r << ((verifier.unmet_requirements() && verifier.unmet_requirements()->empty() == test->r)?green:red) << " \"" << stringify(a) << "\"\033[0m" << std::endl;
            EXPECT_EQ((verifier.unmet_requirements() && verifier.unmet_requirements()->empty()), test->r );
        }

        env->set_want_choice_enabled(ChoicePrefixName(""), UnprefixedChoiceName("a"), false);
        env->set_want_choice_enabled(ChoicePrefixName(""), UnprefixedChoiceName("b"), false);
        env->set_want_choice_enabled(ChoicePrefixName(""), UnprefixedChoiceName("c"), false);
        env->set_want_choice_enabled(ChoicePrefixName(""), UnprefixedChoiceName("d"), false);
        env->set_want_choice_enabled(ChoicePrefixName(""), UnprefixedChoiceName("e"), false);
        env->set_want_choice_enabled(ChoicePrefixName(""), UnprefixedChoiceName("f"), false);
        env->set_want_choice_enabled(ChoicePrefixName(""), UnprefixedChoiceName("g"), false);
        env->set_want_choice_enabled(ChoicePrefixName(""), UnprefixedChoiceName("h"), false);
    }

    auto print_test_name = [](const ::testing::TestParamInfo<UseFlagParams>& info) {
        std::string name =
                std::string(info.param.a  ?"a"  :"na")
        + '_' + std::string(info.param.foo?"foo":"nfoo")
        + '_' + std::string(info.param.b  ?"b"  :"nb")
        + '_' + std::string(info.param.bar?"bar":"nbar")
        + '_' + std::string(info.param.result?"true":"false")
        ;
        return name;
    };

    INSTANTIATE_TEST_SUITE_P(ERepository7, SelectorsAnyOf,
        testing::Values(
            // "|| ( a? ( foo ) b? ( bar ) )"
            UseFlagParams{false,false,false,false,false},
            UseFlagParams{false,false,false, true,false},
            UseFlagParams{false,false, true,false,false},
            UseFlagParams{false,false, true, true,false},
            UseFlagParams{false, true,false,false,false},
            UseFlagParams{false, true,false, true, true},
            UseFlagParams{false, true, true,false,false},
            UseFlagParams{false, true, true, true, true},
            UseFlagParams{ true,false,false,false,false},
            UseFlagParams{ true,false,false, true,false},
            UseFlagParams{ true,false, true,false, true},
            UseFlagParams{ true,false, true, true, true},
            UseFlagParams{ true, true,false,false,false},
            UseFlagParams{ true, true,false, true, true},
            UseFlagParams{ true, true, true,false, true},
            UseFlagParams{ true, true, true, true, true}
        ),
        print_test_name
    );

    INSTANTIATE_TEST_SUITE_P(ERepository7, SelectorsOneOf,
        testing::Values(
            // "^^ ( a? ( foo ) b? ( bar ) )"
            UseFlagParams{false,false,false,false,false},
            UseFlagParams{false,false,false, true,false},
            UseFlagParams{false,false, true,false,false},
            UseFlagParams{false,false, true, true,false},
            UseFlagParams{false, true,false,false,false},
            UseFlagParams{false, true,false, true, true},
            UseFlagParams{false, true, true,false,false},
            UseFlagParams{false, true, true, true, true},
            UseFlagParams{ true,false,false,false,false},
            UseFlagParams{ true,false,false, true,false},
            UseFlagParams{ true,false, true,false, true},
            UseFlagParams{ true,false, true, true, true},
            UseFlagParams{ true, true,false,false,false},
            UseFlagParams{ true, true,false, true, true},
            UseFlagParams{ true, true, true,false, true},
            UseFlagParams{ true, true, true, true,false}
        ),
        print_test_name
    );
}
