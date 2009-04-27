#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006, 2007, 2008, 2009 Ciaran McCreesh
#
# Based in part upon ebuild.sh from Portage, which is Copyright 1995-2005
# Gentoo Foundation and distributed under the terms of the GNU General
# Public License v2.
#
# This file is part of the Paludis package manager. Paludis is free software;
# you can redistribute it and/or modify it under the terms of the GNU General
# Public License, version 2, as published by the Free Software Foundation.
#
# Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 59 Temple
# Place, Suite 330, Boston, MA  02111-1307  USA

default_src_test_slow()
{
    :
}

src_test_slow()
{
    default "$@"
}

exheres_internal_test_slow()
{
    local old_sandbox_predict="${SANDBOX_PREDICT}"
    [[ -z "${PALUDIS_DO_NOTHING_SANDBOXY}" ]] && SANDBOX_PREDICT="${SANDBOX_PREDICT+${SANDBOX_PREDICT}:}/"

    local save_PALUDIS_EXTRA_DIE_MESSAGE="${PALUDIS_EXTRA_DIE_MESSAGE}"

    if [[ -d "${WORK}" ]] ; then
        cd "${WORK}" || die "cd to \${WORK} (\"${WORK}\") failed"
    elif [[ -d "${WORKBASE}" ]] ; then
        cd "${WORKBASE}" || die "cd to \${WORKBASE} (\"${WORKBASE}\") failed"
    fi

    if hasq "test_slow" ${SKIP_FUNCTIONS} ; then
        ebuild_section "Skipping src_test_slow (SKIP_FUNCTIONS)"
    else
        ebuild_section "Starting src_test_slow"
        src_test_slow
        ebuild_section "Done src_test_slow"
    fi

    export PALUDIS_EXTRA_DIE_MESSAGE="${save_PALUDIS_EXTRA_DIE_MESSAGE}"

    [[ -z "${PALUDIS_DO_NOTHING_SANDBOXY}" ]] && SANDBOX_PREDICT="${old_sandbox_predict}"
    true
}
