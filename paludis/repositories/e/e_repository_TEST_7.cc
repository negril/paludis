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
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/selectors-dep-7",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("7", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*pretend_action);
        EXPECT_TRUE(pretend_action->failed());
        // FIXME: doesn't fail, but should.
        //        Reason is known: EAPI=8 behavior for empty OR groups not yet
        //        implemented.
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
