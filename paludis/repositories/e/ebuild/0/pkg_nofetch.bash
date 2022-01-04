#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006, 2007 Ciaran McCreesh
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

pkg_nofetch()
{
    [[ -z "${A}" ]] && return

    local f g=
    for f in ${A} ; do
        [[ -f "${DISTDIR}/${A}" ]] && continue
        if [[ -z "${g}" ]] ; then
            echo "The following files could not be fetched automatically for ${PN}:"
            g=no
        fi
        echo "* ${f}"
    done
}

ebuild_f_nofetch()
{
    local old_sandbox_write="${SANDBOX_WRITE}"
    [[ -z "${PALUDIS_DO_NOTHING_SANDBOXY}" ]] && SANDBOX_WRITE="${SANDBOX_WRITE+${SANDBOX_WRITE}:}${DISTDIR}"

    # Try to use empty cwd opportunistically, since it doesn't hurt, but
    # accept if it's impossible to do so if the EAPI doesn't explicitly
    # require it.
    local need_empty="${PALUDIS_PKG_PHASES_NEED_EMPTY_CWD}"
    if [[ -n "${PALUDIS_EMPTYDIR}" ]]; then
        # Check if it looks actually somewhat valid.
        if [[ '/' != "${PALUDIS_EMPTYDIR}" ]]; then
            if [[ -e "${PALUDIS_EMPTYDIR}" ]]; then
                if ! rm -fr "${PALUDIS_EMPTYDIR}"; then
                    [[ -n "${need_empty}" ]] && die "unable to remove \${PALUDIS_EMPTYDIR} (\"${PALUDIS_EMPTYDIR}\")"
                fi
            fi

            if ! mkdir "${PALUDIS_EMPTYDIR}"; then
                [[ -n "${need_empty}" ]] && die "unable to create \${PALUDIS_EMPTYDIR} (\"${PALUDIS_EMPTYDIR}\")"
            fi

            if ! cd "${PALUDIS_EMPTYDIR}"; then
                [[ -n "${need_empty}" ]] && die "unable to change working directory to \${PALUDIS_EMPTYDIR} (\"${PALUDIS_EMPTYDIR}\")"
            fi

            [[ -z "${PALUDIS_DO_NOTHING_SANDBOXY}" ]] && SANDBOX_WRITE="${SANDBOX_WRITE+${SANDBOX_WRITE}:}${PALUDIS_EMPTYDIR%/}/"
        else
            [[ -n "${need_empty}" ]] && die "\${PALUDIS_EMPTYDIR} (\"${PALUDIS_EMPTYDIR}\") is not valid"
        fi
    else
        [[ -n "${need_empty}" ]] && die "\${PALUDIS_EMPTYDIR} (\"${PALUDIS_EMPTYDIR}\") is not defined"
    fi

    if hasq "nofetch" ${SKIP_FUNCTIONS} ; then
        ebuild_section "Skipping pkg_nofetch (SKIP_FUNCTIONS)"
    else
        ebuild_section "Starting pkg_nofetch"
        pkg_nofetch
        ebuild_section "Done pkg_nofetch"
    fi
    [[ -z "${PALUDIS_DO_NOTHING_SANDBOXY}" ]] && SANDBOX_WRITE="${old_sandbox_write}"
    true
}
