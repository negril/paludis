/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
 * Copyright (c) 2007 Piotr Jaroszyński <peper@gentoo.org>
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

#include "vdb_unmerger.hh"

using namespace paludis;

#include <paludis/environment.hh>
#include <paludis/repositories/gentoo/vdb_unmerger-sr.cc>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/log.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/digests/md5.hh>
#include <paludis/hook.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/package_database.hh>
#include <paludis/util/join.hh>
#include <list>
#include <map>
#include <fstream>
#include <iostream>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace paludis
{
    template<>
    struct Implementation<VDBUnmerger>
    {
        VDBUnmergerOptions options;

        std::list<std::string> config_protect;
        std::list<std::string> config_protect_mask;

        std::multimap<std::string, std::string> extra_info;

        Implementation(const VDBUnmergerOptions & o) :
            options(o)
        {
            WhitespaceTokeniser::get_instance()->tokenise(o.config_protect,
                    std::back_inserter(config_protect));
            WhitespaceTokeniser::get_instance()->tokenise(o.config_protect_mask,
                    std::back_inserter(config_protect_mask));
        }
    };

    typedef std::multimap<std::string, std::string>::iterator ExtraInfoIterator;
}

VDBUnmerger::VDBUnmerger(const VDBUnmergerOptions & o) :
    Unmerger(UnmergerOptions::create()
            .environment(o.environment)
            .root(o.root)),
    PrivateImplementationPattern<VDBUnmerger>(new Implementation<VDBUnmerger>(o)),
    _imp(PrivateImplementationPattern<VDBUnmerger>::_imp.operator-> ())
{
}

VDBUnmerger::~VDBUnmerger()
{
}

Hook
VDBUnmerger::extend_hook(const Hook & h) const
{
    std::string cat(stringify(_imp->options.package_name.category));
    std::string pn(stringify(_imp->options.package_name.package));
    std::string pvr(stringify(_imp->options.version));
    std::string pv(stringify(_imp->options.version.remove_revision()));

    tr1::shared_ptr<const FSEntryCollection> bashrc_files(_imp->options.environment->bashrc_files());

    Hook result(Unmerger::extend_hook(h)
        ("P", pn + "-" + pv)
        ("PN", pn)
        ("CATEGORY", cat)
        ("PR", _imp->options.version.revision_only())
        ("PV", pv)
        ("PVR", pvr)
        ("PF", pn + "-" + pvr)
        ("CONFIG_PROTECT", _imp->options.config_protect)
        ("CONFIG_PROTECT_MASK", _imp->options.config_protect_mask)
        ("PALUDIS_BASHRC_FILES", join(bashrc_files->begin(), bashrc_files->end(), " ")));

    if (0 != _imp->options.repository)
    {
        std::string slot(stringify(_imp->options.repository->version_metadata(
                        _imp->options.package_name, _imp->options.version)->slot));
        return result("SLOT", slot);
    }
    else
        return result;
}

bool
VDBUnmerger::config_protected(const FSEntry & f) const
{
    std::string tidy(make_tidy(f));

    bool result(false);
    for (std::list<std::string>::const_iterator c(_imp->config_protect.begin()),
            c_end(_imp->config_protect.end()) ; c != c_end && ! result ; ++c)
        if (0 == tidy.compare(0, c->length(), *c))
            result = true;
    if (result)
        for (std::list<std::string>::const_iterator c(_imp->config_protect_mask.begin()),
                c_end(_imp->config_protect_mask.end()) ; c != c_end && result ; ++c)
            if (0 == tidy.compare(0, c->length(), *c))
                result = false;

    return result;
}

std::string
VDBUnmerger::make_tidy(const FSEntry & f) const
{
    std::string root_str(stringify(_imp->options.root)), f_str(stringify(f));
    if (root_str == "/")
        root_str.clear();
    if (0 != f_str.compare(0, root_str.length(), root_str))
        throw VDBUnmergerError("Can't work out tidy name for '" + f_str + "' with root '" + root_str + "'");
    return f_str.substr(root_str.length());
}

void
VDBUnmerger::populate_unmerge_set()
{
    std::ifstream c(stringify(_imp->options.contents_file).c_str());
    if (! c)
        throw VDBUnmergerError("Cannot read '" + stringify(_imp->options.contents_file) + "'");

    std::string line;
    while (std::getline(c, line))
    {
        std::vector<std::string> tokens;
        WhitespaceTokeniser::get_instance()->tokenise(line, std::back_inserter(tokens));
        if (tokens.empty())
            continue;

        if ("obj" == tokens.at(0))
        {
            while (tokens.size() > 4)
            {
                if (std::string::npos != tokens.at(4).find('='))
                    break;

                tokens.at(1).append(" " + tokens.at(2));
                tokens.erase(next(tokens.begin(), 2));
            }

            if (tokens.size() != 4)
                Log::get_instance()->message(ll_warning, lc_no_context, "Malformed VDB entry '" + line + "'");
            else
            {
                add_unmerge_entry(tokens.at(1), et_file);

                for (std::vector<std::string>::iterator i(next(tokens.begin(), 2)), i_end(tokens.end()) ;
                        i != i_end ; ++i)
                    _imp->extra_info.insert(std::make_pair(tokens.at(1), *i));
            }

        }
        else if ("sym" == tokens.at(0))
        {
            while (tokens.size() > 5)
            {
                if (tokens.at(2) == "->")
                    break;

                tokens.at(1).append(" " + tokens.at(2));
                tokens.erase(next(tokens.begin(), 2));
            }

            while (tokens.size() > 5)
            {
                if (std::string::npos != tokens.at(5).find('='))
                    break;

                tokens.at(3).append(" " + tokens.at(4));
                tokens.erase(next(tokens.begin(), 4));
            }

            if (tokens.size() != 5 || tokens.at(2) != "->")
                Log::get_instance()->message(ll_warning, lc_no_context, "Malformed VDB entry '" + line + "'");
            else
            {
                add_unmerge_entry(tokens.at(1), et_sym);

                for (std::vector<std::string>::iterator i(next(tokens.begin(), 3)), i_end(tokens.end()) ;
                        i != i_end ; ++i)
                    _imp->extra_info.insert(std::make_pair(tokens.at(1), *i));
            }
        }
        else if ("misc" == tokens.at(0))
        {
        }
        else if ("fif" == tokens.at(0) || "dev" == tokens.at(0))
        {
            while (tokens.size() > 2)
            {
                if (std::string::npos != tokens.at(2).find('='))
                    break;

                tokens.at(1).append(" " + tokens.at(2));
                tokens.erase(next(tokens.begin(), 2));
            }

            if (tokens.size() != 2)
                Log::get_instance()->message(ll_warning, lc_no_context, "Malformed VDB entry '" + line + "'");
            else
            {
                add_unmerge_entry(tokens.at(1), et_misc);

                _imp->extra_info.insert(std::make_pair(tokens.at(1), tokens.at(0)));
                for (std::vector<std::string>::iterator i(next(tokens.begin(), 2)), i_end(tokens.end()) ;
                        i != i_end ; ++i)
                    _imp->extra_info.insert(std::make_pair(tokens.at(1), *i));
            }
        }
        else if ("dir" == tokens.at(0))
        {
            while (tokens.size() > 2)
            {
                if (std::string::npos != tokens.at(2).find('='))
                    break;

                tokens.at(1).append(" " + tokens.at(2));
                tokens.erase(next(tokens.begin(), 2));
            }

            if (tokens.size() != 2)
                Log::get_instance()->message(ll_warning, lc_no_context, "Malformed VDB entry '" + line + "'");
            else
            {
                add_unmerge_entry(tokens.at(1), et_dir);

                for (std::vector<std::string>::iterator i(next(tokens.begin(), 2)), i_end(tokens.end()) ;
                        i != i_end ; ++i)
                    _imp->extra_info.insert(std::make_pair(tokens.at(1), *i));
            }
        }
        else
            Log::get_instance()->message(ll_warning, lc_no_context, "Malformed VDB entry '" + line + "'");
    }
}

bool
VDBUnmerger::check_file(const FSEntry & f) const
{
    ExtraInfoIterator i(_imp->extra_info.lower_bound(stringify(f)));

    if (! (_imp->options.root / f).is_regular_file())
        display("--- [!type] " + stringify(f));
    else if (stringify((_imp->options.root / f).mtime()) != next(i)->second)
        display("--- [!time] " + stringify(f));
    else
    {
        std::ifstream md5_file(stringify(_imp->options.root / f).c_str());
        if (! md5_file)
        {
            Log::get_instance()->message(ll_warning, lc_no_context, "Cannot get md5 for '" +
                    stringify(_imp->options.root / f) + "'");
            display("--- [!md5?] " + stringify(f));
        }
        else if (MD5(md5_file).hexsum() != i->second)
            display("--- [!md5 ] " + stringify(f));
        else if (config_protected(_imp->options.root / f))
            display("--- [cfgpr] " + stringify(f));
        else
            return true;
    }

    return false;
}

bool
VDBUnmerger::check_sym(const FSEntry & f) const
{
    ExtraInfoIterator i(_imp->extra_info.lower_bound(stringify(f)));

    if (! (_imp->options.root / f).is_symbolic_link())
        display("--- [!type] " + stringify(f));
    else if (stringify((_imp->options.root / f).mtime()) != next(i)->second)
        display("--- [!time] " + stringify(f));
    else if ((_imp->options.root / f).readlink() !=  i->second)
        display("--- [!dest] " + stringify(f));
    else
        return true;

    return false;
}

bool
VDBUnmerger::check_misc(const FSEntry & f) const
{
    ExtraInfoIterator i(_imp->extra_info.lower_bound(stringify(f)));

    if ("fif" == i->second && ! (_imp->options.root / f).is_fifo())
        display("--- [!type] " + stringify(f));
    else if ("dev" == i->second && ! (_imp->options.root / f).is_device())
        display("--- [!type] " + stringify(f));
    else
        return true;

    return false;
}

bool
VDBUnmerger::check_dir(const FSEntry & f) const
{
    if (! (_imp->options.root / f).is_directory())
        display("--- [!type] " + stringify(f));
    else if (DirIterator(_imp->options.root / f, false) != DirIterator())
        display("--- [!empt] " + stringify(f));
    else
        return true;

    return false;
}

void
VDBUnmerger::display(const std::string & message) const
{
    std::cout << message << std::endl;
}

VDBUnmergerError::VDBUnmergerError(const std::string & s) throw () :
    UnmergerError(s)
{
}

