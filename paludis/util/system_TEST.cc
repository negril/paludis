/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include <paludis/util/system.hh>
#include <paludis/util/pstream.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace test;
using namespace paludis;

namespace test_cases
{

    /**
     * \test Test getenv_with_default.
     *
     * \ingroup Test
     */
    struct GetenvWithDefaultTest : TestCase
    {
        GetenvWithDefaultTest() : TestCase("getenv_with_default") { }

        void run()
        {
            TEST_CHECK(! getenv_with_default("HOME", "!").empty());
            TEST_CHECK_EQUAL(getenv_with_default("HOME", "!").at(0), '/');
            TEST_CHECK_EQUAL(getenv_with_default("THEREISNOSUCHVARIABLE", "moo"), "moo");
        }
    } test_getenv_with_default;

    /**
     * \test Test getenv_or_error.
     *
     * \ingroup Test
     */
    struct GetenvOrErrorTest : TestCase
    {
        GetenvOrErrorTest() : TestCase("getenv_or_error") { }

        void run()
        {
            TEST_CHECK(! getenv_or_error("HOME").empty());
            TEST_CHECK_THROWS(getenv_or_error("THEREISNOSUCHVARIABLE"), GetenvError);
        }
    } test_getenv_or_error;

    /**
     * \test Test kernel_version.
     *
     * \ingroup Test
     */
    struct KernelVersionTest : TestCase
    {
        KernelVersionTest() : TestCase("kernel version") { }

        void run()
        {
            TEST_CHECK(! kernel_version().empty());
#ifdef linux
            TEST_CHECK('2' == kernel_version().at(0));
            TEST_CHECK('.' == kernel_version().at(1));
#else
#  error You need to write a sanity test for kernel_version() for your platform.
#endif
        }
    } test_kernel_version;

    /**
     * \test Test run_command.
     *
     * \ingroup Test
     */
    struct RunCommandTest : TestCase
    {
        RunCommandTest() : TestCase("run_command") { }

        void run()
        {
            TEST_CHECK(0 == run_command("true"));
            TEST_CHECK(0 != run_command("false"));
        }
    } test_run_command;

    /**
     * \test Test make_env_command.
     *
     * \ingroup Test
     */
    struct MakeEnvCommandTest : TestCase
    {
        MakeEnvCommandTest() : TestCase("make_env_command") { }

        void run()
        {
            TEST_CHECK(0 != run_command(make_env_command("printenv PALUDUS_TEST_ENV_VAR")));
            TEST_CHECK(0 == run_command(make_env_command("bash -c '[[ -z $PALUDUS_TEST_ENV_VAR ]]'")));
            TEST_CHECK(0 == run_command(make_env_command("bash -c '[[ -z $PALUDUS_TEST_ENV_VAR ]]'")(
                            "PALUDUS_TEST_ENV_VAR", "")));
            TEST_CHECK(0 == run_command(make_env_command("bash -c '[[ -n $PALUDUS_TEST_ENV_VAR ]]'")(
                            "PALUDUS_TEST_ENV_VAR", "foo")));
            TEST_CHECK(0 != run_command(make_env_command("bash -c '[[ -z $PALUDUS_TEST_ENV_VAR ]]'")(
                            "PALUDUS_TEST_ENV_VAR", "foo")));
            TEST_CHECK(0 != run_command(make_env_command("bash -c '[[ -n $PALUDUS_TEST_ENV_VAR ]]'")));
            TEST_CHECK(0 != run_command(make_env_command("bash -c '[[ -n $PALUDUS_TEST_ENV_VAR ]]'")(
                            "PALUDUS_TEST_ENV_VAR", "")));
            TEST_CHECK(0 == run_command(make_env_command("bash -c '[[ $PALUDUS_TEST_ENV_VAR == foo ]]'")(
                            "PALUDUS_TEST_ENV_VAR", "foo")));
            TEST_CHECK(0 != run_command(make_env_command("bash -c '[[ $PALUDUS_TEST_ENV_VAR == foo ]]'")));
            TEST_CHECK(0 != run_command(make_env_command("bash -c '[[ $PALUDUS_TEST_ENV_VAR == foo ]]'")(
                            "PALUDUS_TEST_ENV_VAR", "")));
            TEST_CHECK(0 != run_command(make_env_command("bash -c '[[ $PALUDUS_TEST_ENV_VAR == foo ]]'")(
                            "PALUDUS_TEST_ENV_VAR", "bar")));
        }
    } test_make_env_command;

    /**
     * \test Test make_env_command with quotes.
     *
     * \ingroup Test
     */
    struct MakeEnvCommandQuoteTest : TestCase
    {
        MakeEnvCommandQuoteTest() : TestCase("make_env_command quotes") { }

        void run()
        {
            TEST_CHECK(0 == run_command(make_env_command(
                            "bash -c '[[ x$PALUDUS_TEST_ENV_VAR == \"x....\" ]]'")
                        ("PALUDUS_TEST_ENV_VAR", "....")));
            TEST_CHECK(0 == run_command(make_env_command(
                            "bash -c '[[ x$PALUDUS_TEST_ENV_VAR == \"x..'\"'\"'..\" ]]'")
                        ("PALUDUS_TEST_ENV_VAR", "..'..")));
        }
    } test_make_env_command_quotes;
}
