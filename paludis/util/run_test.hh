/*
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

#ifndef PALUDIS_GUARD_PALUDIS_RUN_TEST_HH
#define PALUDIS_GUARD_PALUDIS_RUN_TEST_HH 1

#include <paludis/environments/test/test_environment.hh>

#include <paludis/repositories/e/e_repository.hh>
#include <paludis/repositories/fake/fake_installed_repository.hh>

#include <paludis/util/fs_stat.hh>
#include <paludis/util/map.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/process.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/system.hh>

#include <paludis/standard_output_manager.hh>
#include <paludis/action.hh>

#include <gtest/gtest.h>

/** \file
 * Declarations for the Test base class.
 *
 * \ingroup g_digests
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{

    /**
     * Test base class.
     *
     * \ingroup g_digests
     */

    int run_script(const std::string& suffix)
    {
        if(suffix.empty())
            throw std::runtime_error("Missing script suffix");

        std::string TEST_NAME(getenv_or_error("TEST_NAME"));
        FSPath TEST_SCRIPT_DIR(getenv_or_error("TEST_SCRIPT_DIR"));
        FSPath script(TEST_SCRIPT_DIR / (TEST_NAME + "_" + suffix + ".sh"));

        if( script.stat().is_regular_file() )
        {
            std::cout << ">>> " << suffix << " for test " << TEST_NAME << std::endl;
            Process process(ProcessCommand({ "bash", stringify(script) }));
            int exit_status(0);
            exit_status = process.run().wait();
            if (0 != exit_status)
            {
                std::cout << ">>> exiting with error for test " << TEST_NAME << " " << exit_status << std::endl;
                return exit_status;
            }
        }
        else
        {
            std::cerr << ">>> No " << stringify(script) << " to run" << std::endl;
        }
        return 0;
    }

    int run_setup(const FSPath& path)
    {
        if(! path.stat().is_directory() )
            path.mkdir(0755, {fspmkdo_ok_if_exists} );

        chdir(stringify(path).c_str());
        return run_script("setup");
    }

    int run_cleanup(const FSPath& path)
    {
        FSPath cwd = FSPath::cwd();
        chdir(stringify(path).c_str());

        run_script("cleanup");

        chdir(stringify(cwd).c_str());

        if( path.stat().is_directory())
            return path.rmdir();
        return true;
    }

    void print_env(){
        char **s = environ;

        for (; *s; s++) {
            std::cout << *s << std::endl;
        }
    }

    void cannot_uninstall(const std::shared_ptr<const PackageID> & id, const UninstallActionOptions &)
    {
        if (id)
            throw InternalError(PALUDIS_HERE, "cannot uninstall");
    }

    std::shared_ptr<OutputManager> make_standard_output_manager(const Action &)
    {
        return std::make_shared<StandardOutputManager>();
    }

    std::string from_keys(const std::shared_ptr<const Map<std::string, std::string> > & m,
                          const std::string & k)
    {
        Map<std::string, std::string>::ConstIterator mm(m->find(k));
        if (m->end() == mm)
            return "";
        else
            return mm->second;
    }

    WantPhase want_all_phases(const std::string &)
    {
        return wp_yes;
    }

    class Test : public ::testing::Test {
    public:
        Test():cwd(""),test_dir(""){}
    protected:

        void SetUp() override {
            cwd = FSPath::cwd();

            std::string current = (std::string(::testing::UnitTest::GetInstance()->current_test_case()->name()) + '.' + std::string(::testing::UnitTest::GetInstance()->current_test_info()->name()) +"_dir");

            // we need to replace invalid chars so we can use it as pathname
            std::size_t i = 0;
            while(i <= current.size())
            {
                i=current.find('/',i);
                if(i == std::string::npos)
                    break;
                current.replace(i,1,".");
                i+=1;
            }

            test_dir = cwd / current;
            if (auto instance = ::testing::UnitTest::GetInstance())
            {
                if(auto current_test_case =instance->current_test_case())
                    setenv("CURRENT_TEST_CASE", current_test_case->name(), 1);
                if(auto current_test_info =instance->current_test_info())
                    setenv("CURRENT_TEST_INFO", current_test_info->name(), 1);
                if(auto current_test_suite =instance->current_test_case())
                    setenv("CURRENT_TEST_SUITE", current_test_suite->name(), 1);
            }

            std::string test_name = getenv_or_error("TEST_NAME");

            run_cleanup(test_dir);
            run_setup(test_dir);

            env = std::make_shared<TestEnvironment>(new TestEnvironment);

            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify( test_dir / (test_name + "_dir") / "repo" ).c_str());
            keys->insert("profiles", stringify( test_dir / (test_name + "_dir") / "repo/profiles/profile" ).c_str());
            keys->insert("layout", "traditional");
            keys->insert("eapi_when_unknown", "0");
            keys->insert("eapi_when_unspecified", "0");
            keys->insert("profile_eapi", "0");
            keys->insert("distdir", stringify( test_dir / (test_name + "_dir") / "distdir" ).c_str());
            keys->insert("builddir", stringify( test_dir / (test_name + "_dir") / "build" ).c_str());
            std::shared_ptr<Repository> repo(ERepository::repository_factory_create(env.get(),
                std::bind(from_keys, keys, std::placeholders::_1)));
            env->add_repository(1, repo);

            std::shared_ptr<FakeInstalledRepository> installed_repo(std::make_shared<FakeInstalledRepository>(
                make_named_values<FakeInstalledRepositoryParams>(
                    n::environment() = env.get(),
                                                                 n::name() = RepositoryName("installed"),
                                                                 n::suitable_destination() = true,
                                                                 n::supports_uninstall() = true
                )));
            env->add_repository(2, installed_repo);

            install_action = std::make_shared<InstallAction>(make_named_values<InstallActionOptions>(
                n::destination() = installed_repo,
                n::make_output_manager() = &make_standard_output_manager,
                n::perform_uninstall() = &cannot_uninstall,
                n::replacing() = std::make_shared<PackageIDSequence>(),
                n::want_phase() = &want_all_phases
            ));

            pretend_action = std::make_shared<PretendAction> (make_named_values<PretendActionOptions>(
                n::destination() = installed_repo,
                n::make_output_manager() = &make_standard_output_manager,
                n::replacing() = std::make_shared<PackageIDSequence>()
            ));

            fetch_action = std::make_shared<FetchAction>(make_named_values<FetchActionOptions>(
                // TODO(compnerd) provide a cross-compile-host
                n::cross_compile_host() = "",
                n::errors() = std::make_shared<Sequence<FetchActionFailure>>(),
                n::exclude_unmirrorable() = false,
                n::fetch_parts() = FetchParts() + fp_regulars + fp_extras,
                n::ignore_not_in_manifest() = false,
                n::ignore_unfetched() = false,
                n::make_output_manager() = &make_standard_output_manager,
                n::safe_resume() = true,
                // TODO(compnerd) provide a tool-prefix
                n::tool_prefix() = "",
                n::want_phase() = &want_all_phases
            ));

        }

        void TearDown() override {
            fetch_action = nullptr;
            pretend_action = nullptr;
            install_action = nullptr;
            env = nullptr;
            chdir(stringify(cwd).c_str());
            run_cleanup(test_dir);
        }

        std::shared_ptr<TestEnvironment> env;
        std::shared_ptr<InstallAction> install_action;
        std::shared_ptr<PretendAction> pretend_action;
        std::shared_ptr<FetchAction> fetch_action;

        FSPath cwd;
        FSPath test_dir;
    };
}

#endif
