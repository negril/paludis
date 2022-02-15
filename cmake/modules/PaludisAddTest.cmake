
include(CMakeParseArguments)

function(paludis_add_test test_name)
  set(options BASH GTEST PYTHON RUBY DISCOVER)
  set(single_value_args EBUILD_MODULE_SUFFIXES TEST_RUNNER)
  set(multiple_value_args LINK_LIBRARIES)

  cmake_parse_arguments(PAT "${options}" "${single_value_args}" "${multiple_value_args}" ${ARGN})

  string(REGEX MATCH "_TEST" has_TEST ${test_name})
  if(NOT has_TEST)
    set(test_name ${test_name}_TEST)
  endif()

  if(PAT_GTEST AND NOT ENABLE_GTEST)
    return()
  endif()

  if(NOT PAT_BASH AND NOT PAT_PYTHON AND NOT PAT_RUBY)
    add_executable(${test_name}
                     "${CMAKE_CURRENT_SOURCE_DIR}/${test_name}.cc")
  endif()
  if(PAT_GTEST)
    target_link_libraries(${test_name}
                          PRIVATE
                            libpaludis
                            libpaludisutil
                            GTest::GTest
                            GTest::Main
                            GTest::gmock
                            ${PAT_LINK_LIBRARIES})
  endif()

  set(pat_test_runner "${PROJECT_SOURCE_DIR}/paludis/util/run_test.sh")
  if(PAT_TEST_RUNNER)
    set(pat_test_runner "${PAT_TEST_RUNNER}")
  endif()

  set(pat_environment_variables)
  if(NOT "${PAT_EBUILD_MODULE_SUFFIXES}" STREQUAL "")
    set(pat_environment_variables PALUDIS_EBUILD_MODULE_SUFFIXES=${PAT_EBUILD_MODULE_SUFFIXES})
  endif()

  string(REGEX REPLACE "_TEST" "" pat_display_name ${test_name})

  set(pat_test_extension "")
  if(PAT_PYTHON)
    set(pat_test_extension ".py")
    set(pat_display_name "python_${pat_display_name}")
  elseif(PAT_RUBY)
    set(pat_test_extension ".rb")
    set(pat_display_name "ruby_${pat_display_name}")
  endif()

  if(PAT_BASH OR PAT_PYTHON OR PAT_RUBY)
    set(pat_test_binary_parent_directory "${CMAKE_CURRENT_SOURCE_DIR}")
  else()
    set(pat_test_binary_parent_directory "${CMAKE_CURRENT_BINARY_DIR}")
  endif()

  set(pat_test_binary
      "${pat_test_binary_parent_directory}/${test_name}${pat_test_extension}")

  math(EXPR version_major "${PROJECT_VERSION_MAJOR} * 100 + ${PROJECT_VERSION_MINOR}")

  # NOTE(compnerd) the trailing slash on the TEST_SCRIPT_DIR is important as
  # the harness will not add that for us
  if(NOT PAT_DISCOVER AND NOT (PAT_GTEST AND NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${test_name}_setup.sh" AND NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${test_name}_cleanup.sh"))
  add_test(NAME
             ${pat_display_name}
           COMMAND
             env -i HOME=${CMAKE_CURRENT_BINARY_DIR}
                    LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}/paludis
                    PALUDIS_BYPASS_USERPRIV_CHECKS=YES
                    PALUDIS_DEFAULT_OUTPUT_CONF=${PROJECT_SOURCE_DIR}/paludis/environments/paludis/tests_output.conf
                    PALUDIS_DISTRIBUTION=gentoo
                    PALUDIS_DISTRIBUTIONS_DIR=${PROJECT_SOURCE_DIR}/paludis/distributions
                    PALUDIS_EAPIS_DIR=${PROJECT_SOURCE_DIR}/paludis/repositories/e/eapis
                    PALUDIS_EBUILD_DIR=${PROJECT_SOURCE_DIR}/paludis/repositories/e/ebuild
                    PALUDIS_EBUILD_DIR_FALLBACK=${CMAKE_BINARY_DIR}/paludis/repositories/e/ebuild
                    PALUDIS_ECHO_FUNCTIONS_DIR=${CMAKE_BINARY_DIR}/paludis/util
                    PALUDIS_EXTRA_EBUILD_MODULES_DIRS=${CMAKE_BINARY_DIR}/paludis/util
                    PALUDIS_FETCHERS_DIR=${PROJECT_SOURCE_DIR}/paludis/fetchers
                    PALUDIS_HOOKER_DIR=${PROJECT_SOURCE_DIR}/paludis
                    PALUDIS_NO_CHOWN=YES
                    PALUDIS_NO_GLOBAL_HOOKS=YES
                    PALUDIS_NO_GLOBAL_SETS=YES
                    PALUDIS_NO_XTERM_TITLES=YES
                    PALUDIS_PC_SLOT=${PALUDIS_PKG_CONFIG_SLOT}
                    PALUDIS_PYTHON_DIR=${PROJECT_SOURCE_DIR}/python
                    PALUDIS_SUFFIXES_FILE=${PROJECT_SOURCE_DIR}/paludis/repositories/e/ebuild_entries_suffixes.conf
                    PALUDIS_TMPDIR=${CMAKE_CURRENT_BINARY_DIR}
                    PYTHON=${Python3_EXECUTABLE}
                    PYTHONPATH=${CMAKE_BINARY_DIR}/python
                    RUBY=${RUBY_EXECUTABLE}
                    RUBYLIB=${CMAKE_BINARY_DIR}/ruby
                    SO_SUFFIX=${version_major}
                    SYSCONFDIR=${CMAKE_INSTALL_FULL_SYSCONFDIR}
                    TEST_SCRIPT_DIR=${CMAKE_CURRENT_SOURCE_DIR}/
                    TOP_BUILDDIR=${CMAKE_BINARY_DIR}
                    SYDBOX_ACTIVE=\${SYDBOX_ACTIVE}
                    ${pat_environment_variables}
             "${BASH_EXECUTABLE}" ${pat_test_runner} "${pat_test_binary}")
    else()
    gtest_discover_tests(
       ${test_name}
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      PROPERTIES
        ENVIRONMENT LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}/paludis
        ENVIRONMENT PALUDIS_BYPASS_USERPRIV_CHECKS=YES
        ENVIRONMENT PALUDIS_DEFAULT_OUTPUT_CONF=${PROJECT_SOURCE_DIR}/paludis/environments/paludis/tests_output.conf
        ENVIRONMENT PALUDIS_DISTRIBUTION=gentoo
        ENVIRONMENT PALUDIS_DISTRIBUTIONS_DIR=${PROJECT_SOURCE_DIR}/paludis/distributions
        ENVIRONMENT PALUDIS_EAPIS_DIR=${PROJECT_SOURCE_DIR}/paludis/repositories/e/eapis
        ENVIRONMENT PALUDIS_EBUILD_DIR=${PROJECT_SOURCE_DIR}/paludis/repositories/e/ebuild
        ENVIRONMENT PALUDIS_EBUILD_DIR_FALLBACK=${CMAKE_BINARY_DIR}/paludis/repositories/e/ebuild
        ENVIRONMENT PALUDIS_ECHO_FUNCTIONS_DIR=${CMAKE_BINARY_DIR}/paludis/util
        ENVIRONMENT PALUDIS_EXTRA_EBUILD_MODULES_DIRS=${CMAKE_BINARY_DIR}/paludis/util
        ENVIRONMENT PALUDIS_FETCHERS_DIR=${PROJECT_SOURCE_DIR}/paludis/fetchers
        ENVIRONMENT PALUDIS_HOOKER_DIR=${PROJECT_SOURCE_DIR}/paludis
        ENVIRONMENT PALUDIS_NO_CHOWN=YES
        ENVIRONMENT PALUDIS_NO_GLOBAL_HOOKS=YES
        ENVIRONMENT PALUDIS_NO_GLOBAL_SETS=YES
        ENVIRONMENT PALUDIS_NO_XTERM_TITLES=YES
        ENVIRONMENT PALUDIS_PC_SLOT=${PALUDIS_PKG_CONFIG_SLOT}
        ENVIRONMENT PALUDIS_PYTHON_DIR=${PROJECT_SOURCE_DIR}/python
        ENVIRONMENT PALUDIS_SUFFIXES_FILE=${PROJECT_SOURCE_DIR}/paludis/repositories/e/ebuild_entries_suffixes.conf
        ENVIRONMENT PALUDIS_TMPDIR=${CMAKE_CURRENT_BINARY_DIR}
        ENVIRONMENT PYTHON=${PYTHON_EXECUTABLE}
        ENVIRONMENT PYTHONPATH=${CMAKE_BINARY_DIR}/python
        ENVIRONMENT RUBY=${RUBY_EXECUTABLE}
        ENVIRONMENT RUBYLIB=${CMAKE_BINARY_DIR}/ruby
        ENVIRONMENT SO_SUFFIX=${version_major}
        ENVIRONMENT SYSCONFDIR=${CMAKE_INSTALL_FULL_SYSCONFDIR}
        ENVIRONMENT TEST_SCRIPT_DIR=${CMAKE_CURRENT_SOURCE_DIR}
        ENVIRONMENT TOP_BUILDDIR=${CMAKE_BINARY_DIR}
        ENVIRONMENT TEST_NAME=${test_name}
        ${pat_environment_variables}
    )
    endif()
endfunction()

