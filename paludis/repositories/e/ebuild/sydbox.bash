#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2009 Ali Polatel <polatel@gmail.com>
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

sydboxcheck()
{
    if [[ -z "${1}" ]]; then
        [[ -e /dev/sydbox ]]
    else
        [[ -e /dev/sydbox/${1} ]]
    fi
}

sydboxcmd()
{
    if sydboxcheck ${1}; then
        if [[ -n "${2}" ]]; then
            [[ "/" != "${2:0:1}" ]] && die "${FUNCNAME} ${1}: non-absolute path"
            : > /dev/sydbox/${1}/"${2}"
        else
            : > /dev/sydbox/${1}
        fi
    fi
}

addread()
{
    die_unless_nonfatal "${FUNCNAME} not implemented for sydbox yet"
}

addwrite()
{
    sydboxcmd write "${1}"
}

adddeny()
{
    die_unless_nonfatal "${FUNCNAME} not implemented for sydbox yet"
}

addpredict()
{
    sydboxcmd predict "${1}"
}

rmwrite()
{
    sydboxcmd unwrite "${1}"
}

rmpredict()
{
    sydboxcmd unpredict "${1}"
}

addfilter()
{
    sydboxcmd addfilter "${1}"
}

rmfilter()
{
    sydboxcmd rmfilter "${1}"
}

