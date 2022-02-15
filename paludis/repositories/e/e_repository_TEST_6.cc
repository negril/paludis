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

#include <algorithm>

#include "config.h"

namespace
{
    using namespace paludis;

    class ERepository6 : public Test
    {
    };

    TEST_F(ERepository6, global_failglob_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/global-failglob-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("UNKNOWN", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
    }

    TEST_F(ERepository6, nonglobal_no_failglob_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/nonglobal-no-failglob-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*install_action);
    }

    TEST_F(ERepository6, unpack_bare_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/unpack-bare-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*install_action);
    }

    TEST_F(ERepository6, unpack_dotslash_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/unpack-dotslash-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*install_action);
    }

    TEST_F(ERepository6, unpack_absolute_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/unpack-absolute-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*install_action);
    }

    TEST_F(ERepository6, unpack_relative_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/unpack-relative-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*install_action);
    }

    TEST_F(ERepository6, unpack_case_insensitive_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/unpack-case-insensitive-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*install_action);
    }

    TEST_F(ERepository6, econf_no_docdir_htmldir_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/econf-no-docdir-htmldir-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*install_action);
    }

    TEST_F(ERepository6, econf_docdir_only_6_r6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/econf-docdir-only-6-r6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*install_action);
    }

    TEST_F(ERepository6, econf_htmldir_only_6_r6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/econf-htmldir-only-6-r6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*install_action);
    }

    TEST_F(ERepository6, econf_docdir_htmldir_6_r6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/econf-docdir-htmldir-6-r6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*install_action);
    }

    TEST_F(ERepository6, plain_die_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/plain-die-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_THROW(id->perform_action(*install_action), ActionFailedError);
    }

    TEST_F(ERepository6, plain_assert_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/plain-assert-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_THROW(id->perform_action(*install_action), ActionFailedError);
    }

    TEST_F(ERepository6, nonfatal_die_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/nonfatal-die-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_THROW(id->perform_action(*install_action), ActionFailedError);
    }

    TEST_F(ERepository6, nonfatal_assert_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/nonfatal-assert-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_THROW(id->perform_action(*install_action), ActionFailedError);
    }

    TEST_F(ERepository6, die_n_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/die-n-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_THROW(id->perform_action(*install_action), ActionFailedError);
    }

    TEST_F(ERepository6, assert_n_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/assert-n-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_THROW(id->perform_action(*install_action), ActionFailedError);
    }

    TEST_F(ERepository6, nonfatal_die_n_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/nonfatal-die-n-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*install_action);
    }

    TEST_F(ERepository6, nonfatal_assert_n_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/nonfatal-assert-n-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*install_action);
    }

    TEST_F(ERepository6, get_libdir_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/get_libdir-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*install_action);
    }

    TEST_F(ERepository6, no_einstall_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/no-einstall-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_THROW(id->perform_action(*install_action), ActionFailedError);
    }

    TEST_F(ERepository6, in_iuse_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/in_iuse-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*install_action);
    }

    TEST_F(ERepository6, in_iuse_global_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/in_iuse-global-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("UNKNOWN", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
    }

    TEST_F(ERepository6, in_iuse_global_notmetadata_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/in_iuse-global-notmetadata-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_THROW(id->perform_action(*install_action), ActionFailedError);
    }

    TEST_F(ERepository6, einstalldocs_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/einstalldocs-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*install_action);
    }

    TEST_F(ERepository6, einstalldocs_nothing_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/einstalldocs-nothing-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*install_action);
    }

    TEST_F(ERepository6, einstalldocs_DOCS_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/einstalldocs-DOCS-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*install_action);
    }

    TEST_F(ERepository6, einstalldocs_DOCS_array_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/einstalldocs-DOCS-array-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*install_action);
    }

    TEST_F(ERepository6, einstalldocs_empty_DOCS_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/einstalldocs-empty-DOCS-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*install_action);
    }

    TEST_F(ERepository6, einstalldocs_empty_DOCS_array_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/einstalldocs-empty-DOCS-array-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*install_action);
    }

    TEST_F(ERepository6, einstalldocs_HTML_DOCS_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/einstalldocs-HTML_DOCS-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*install_action);
    }

    TEST_F(ERepository6, einstalldocs_HTML_DOCS_array_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/einstalldocs-HTML_DOCS-array-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*install_action);
    }

    TEST_F(ERepository6, einstalldocs_empty_HTML_DOCS_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/einstalldocs-empty-HTML_DOCS-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*install_action);
    }

    TEST_F(ERepository6, einstalldocs_empty_HTML_DOCS_array_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/einstalldocs-empty-HTML_DOCS-array-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*install_action);
    }

    TEST_F(ERepository6, einstalldocs_DOCS_HTML_DOCS_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/einstalldocs-DOCS-HTML_DOCS-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*install_action);
    }

    TEST_F(ERepository6, einstalldocs_failure_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/einstalldocs-failure-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_THROW(id->perform_action(*install_action), ActionFailedError);
    }

    TEST_F(ERepository6, einstalldocs_nonfatal_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/einstalldocs-nonfatal-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*install_action);
    }

    TEST_F(ERepository6, einstalldocs_DOCS_failure_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/einstalldocs-DOCS-failure-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_THROW(id->perform_action(*install_action), ActionFailedError);
    }

    TEST_F(ERepository6, einstalldocs_DOCS_nonfatal_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/einstalldocs-DOCS-nonfatal-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*install_action);
    }

    TEST_F(ERepository6, einstalldocs_DOCS_array_failure_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/einstalldocs-DOCS-array-failure-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_THROW(id->perform_action(*install_action), ActionFailedError);
    }

    TEST_F(ERepository6, einstalldocs_DOCS_array_nonfatal_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/einstalldocs-DOCS-array-nonfatal-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*install_action);
    }

    TEST_F(ERepository6, einstalldocs_HTML_DOCS_failure_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/einstalldocs-HTML_DOCS-failure-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_THROW(id->perform_action(*install_action), ActionFailedError);
    }

    TEST_F(ERepository6, einstalldocs_HTML_DOCS_nonfatal_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/einstalldocs-HTML_DOCS-nonfatal-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*install_action);
    }

    TEST_F(ERepository6, einstalldocs_HTML_DOCS_array_failure_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/einstalldocs-HTML_DOCS-array-failure-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_THROW(id->perform_action(*install_action), ActionFailedError);
    }

    TEST_F(ERepository6, einstalldocs_HTML_DOCS_array_nonfatal_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/einstalldocs-HTML_DOCS-array-nonfatal-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*install_action);
    }

    TEST_F(ERepository6, default_src_install_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/default_src_install-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*install_action);
    }

    TEST_F(ERepository6, eapply_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/eapply-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*install_action);
    }

    TEST_F(ERepository6, eapply_options_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/eapply-options-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*install_action);
    }

    TEST_F(ERepository6, eapply_dashdash_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/eapply-dashdash-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*install_action);
    }

    TEST_F(ERepository6, eapply_missing_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/eapply-missing-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_THROW(id->perform_action(*install_action), ActionFailedError);
    }

    TEST_F(ERepository6, eapply_failure_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/eapply-failure-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_THROW(id->perform_action(*install_action), ActionFailedError);
    }

    TEST_F(ERepository6, eapply_nonfatal_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/eapply-nonfatal-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*install_action);
    }

    TEST_F(ERepository6, eapply_dir_failure_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/eapply-dir-failure-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_THROW(id->perform_action(*install_action), ActionFailedError);
    }

    TEST_F(ERepository6, eapply_dir_nonfatal_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/eapply-dir-nonfatal-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*install_action);
    }

    TEST_F(ERepository6, eapply_badmix_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/eapply-badmix-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_THROW(id->perform_action(*install_action), ActionFailedError);
    }

    TEST_F(ERepository6, eapply_nopatches_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/eapply-nopatches-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_THROW(id->perform_action(*install_action), ActionFailedError);
    }

    TEST_F(ERepository6, eapply_dir_nopatches_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/eapply-dir-nopatches-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_THROW(id->perform_action(*install_action), ActionFailedError);
    }

    TEST_F(ERepository6, eapply_user_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/eapply_user-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*install_action);
    }

    TEST_F(ERepository6, eapply_user2_6){
        setenv("PALUDIS_USER_PATCHES", stringify(test_dir / "e_repository_TEST_6_dir" /
                                                 "root" / "var" / "paludis" / "user_patches").c_str(), 1);
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/eapply_user2-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*install_action);
        unsetenv("PALUDIS_USER_PATCHES");
    }

    TEST_F(ERepository6, eapply_user3_6_r1){
        setenv("PALUDIS_USER_PATCHES", stringify(test_dir / "e_repository_TEST_6_dir" /
                                                 "root" / "var" / "paludis" / "user_patches").c_str(), 1);
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/eapply_user3-6-r1",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*install_action);
        unsetenv("PALUDIS_USER_PATCHES");
    }

    TEST_F(ERepository6, eapply_user4_6_r1_1){
        setenv("PALUDIS_USER_PATCHES", stringify(test_dir / "e_repository_TEST_6_dir" /
                                                 "root" / "var" / "paludis" / "user_patches").c_str(), 1);
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/eapply_user4-6-r1:1",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*install_action);
        unsetenv("PALUDIS_USER_PATCHES");
    }

    TEST_F(ERepository6, eapply_user5_6){
        setenv("PALUDIS_USER_PATCHES", stringify(test_dir / "e_repository_TEST_6_dir" /
                                                 "root" / "var" / "paludis" / "user_patches").c_str(), 1);
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/eapply_user5-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*install_action);
        unsetenv("PALUDIS_USER_PATCHES");
    }

    TEST_F(ERepository6, eapply_user6_6){
        setenv("PALUDIS_USER_PATCHES", stringify(test_dir / "e_repository_TEST_6_dir" /
                                                 "root" / "var" / "paludis" / "user_patches").c_str(), 1);
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/eapply_user6-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*install_action);
        unsetenv("PALUDIS_USER_PATCHES");
    }

    TEST_F(ERepository6, default_src_prepare_nothing_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/default_src_prepare-nothing-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*install_action);
    }

    TEST_F(ERepository6, default_src_prepare_PATCHES_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/default_src_prepare-PATCHES-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*install_action);
    }

    TEST_F(ERepository6, default_src_prepare_empty_PATCHES_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/default_src_prepare-empty-PATCHES-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*install_action);
    }

    TEST_F(ERepository6, default_src_prepare_PATCHES_array_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/default_src_prepare-PATCHES-array-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*install_action);
    }

    TEST_F(ERepository6, default_src_prepare_empty_PATCHES_array_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/default_src_prepare-empty-PATCHES-array-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*install_action);
    }

    TEST_F(ERepository6, bash_compat_6){
        const std::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/bash-compat-6",
                                env.get(), { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("6", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(*install_action);
    }
}
