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

#include "config.h"

namespace
{
    using namespace paludis;

    class ERepository8 : public Test
    {
    };

    TEST_F(ERepository8, bash_compat)
    {
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/bash-compat-8",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("8", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*pretend_action);
        EXPECT_TRUE(! pretend_action->failed());
    }

    TEST_F(ERepository8, econf_added_options)
    {
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/econf-added-options-8",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("8", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_NO_THROW(id->perform_action(*install_action));
    }

    TEST_F(ERepository8, changed_opts)
    {
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/changed-opts-8",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("8", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_NO_THROW(id->perform_action(*install_action));
    }

    TEST_F(ERepository8, dosym_rel)
    {
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/dosym-rel-8",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("8", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_NO_THROW(id->perform_action(*install_action));
    }

    TEST_F(ERepository8, restrict_none)
    {
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/restrict-none-8",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("8", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_NO_THROW(id->perform_action(*fetch_action));
    }

    TEST_F(ERepository8, restrict_mirror)
    {
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/restrict-mirror-8",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("8", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_NO_THROW(id->perform_action(*fetch_action));
    }

    TEST_F(ERepository8, restrict_fetch_nolabels)
    {
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/restrict-fetch-nolabels-8",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("8", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_THROW(id->perform_action(*fetch_action), ActionFailedError);
    }

    TEST_F(ERepository8, restrict_fetch_nolabel_alllabels)
    {
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/restrict-fetch-nolabel-alllabels-8",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("8", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_THROW(id->perform_action(*fetch_action), ActionFailedError);
    }

    TEST_F(ERepository8, restrict_fetch_alllabels)
    {
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/restrict-fetch-alllabels-8",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("8", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_NO_THROW(id->perform_action(*fetch_action));
    }

    TEST_F(ERepository8, banned_functions_hasq)
    {
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/banned-functions-hasq-8",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("8", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*pretend_action);
        EXPECT_TRUE(pretend_action->failed());
    }

    TEST_F(ERepository8, banned_functions_hasv)
    {
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/banned-functions-hasv-8",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("8", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*pretend_action);
        EXPECT_TRUE(pretend_action->failed());
    }

    TEST_F(ERepository8, banned_functions_useq)
    {
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/banned-functions-useq-8",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("8", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*pretend_action);
        EXPECT_TRUE(pretend_action->failed());
    }

    TEST_F(ERepository8, changed_vars)
    {
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/changed-vars-8",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("8", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*pretend_action);
        EXPECT_TRUE(! pretend_action->failed());
    }

    TEST_F(ERepository8, patches_no_opts)
    {
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/patches-no-opts-8",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("8", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_THROW(id->perform_action(*install_action), ActionFailedError);
    }

    TEST_F(ERepository8, accumulated_vars)
    {
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/accumulated-vars-8",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("8", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*pretend_action);
        EXPECT_TRUE(! pretend_action->failed());
    }

    TEST_F(ERepository8, test_network)
    {
        env->set_want_choice_enabled(ChoicePrefixName("build_options"), UnprefixedChoiceName("optional_tests"), true);

        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/test-network-8",
                                  env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("8", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_TRUE(!!id->choices_key()->parse_value()->find_by_name_with_prefix(ChoiceNameWithPrefix("build_options:optional_tests")));
        EXPECT_NO_THROW(id->perform_action(*install_action));

        env->set_want_choice_enabled(ChoicePrefixName("build_options"), UnprefixedChoiceName("optional_tests"), false);
    }

    TEST_F(ERepository8, unpack_formats_removed)
    {
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/unpack-formats-removed-8",
                                  env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("8", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_NO_THROW(id->perform_action(*install_action));
    }

    TEST_F(ERepository8, usev_second_arg)
    {
        env->set_want_choice_enabled(ChoicePrefixName(""), UnprefixedChoiceName("flag"), true);

        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/usev-second-arg-8",
                                  env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("8", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*pretend_action);
        EXPECT_TRUE(! pretend_action->failed());

        env->set_want_choice_enabled(ChoicePrefixName(""), UnprefixedChoiceName("flag"), false);
    }

    TEST_F(ERepository8, pkg_empty_dir)
    {
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/pkg-empty-dir-8",
                                  env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("8", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*pretend_action);
        EXPECT_TRUE(! pretend_action->failed());
    }
}
