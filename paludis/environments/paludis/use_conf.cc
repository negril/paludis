/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009 Ciaran McCreesh
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

#include <paludis/environments/paludis/use_conf.hh>
#include <paludis/environments/paludis/paludis_environment.hh>
#include <paludis/environments/paludis/bashable_conf.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/paludislike_options_conf.hh>
#include <paludis/choice.hh>

using namespace paludis;
using namespace paludis::paludis_environment;

namespace paludis
{
    template<>
    struct Implementation<UseConf>
    {
        const PaludisEnvironment * const env;
        const std::tr1::shared_ptr<PaludisLikeOptionsConf> handler;

        Implementation(const PaludisEnvironment * const e) :
            env(e),
            handler(new PaludisLikeOptionsConf(make_named_values<PaludisLikeOptionsConfParams>(
                            value_for<n::allow_locking>(false),
                            value_for<n::environment>(e),
                            value_for<n::make_config_file>(&make_bashable_conf)
                            )))
        {
        }
    };
}

UseConf::UseConf(const PaludisEnvironment * const e) :
    PrivateImplementationPattern<UseConf>(new Implementation<UseConf>(e))
{
}

UseConf::~UseConf()
{
}

void
UseConf::add(const FSEntry & filename)
{
    Context context("When adding source '" + stringify(filename) + "' as a use file:");

    _imp->handler->add_file(filename);
}

const Tribool
UseConf::want_choice_enabled(
        const std::tr1::shared_ptr<const PackageID> & id,
        const std::tr1::shared_ptr<const Choice> & choice,
        const UnprefixedChoiceName & f
        ) const
{
    Context context("When checking state of flag prefix '" + stringify(choice->prefix()) +
            "' name '" + stringify(f) + "' for '" + stringify(*id) + "':");

    return _imp->handler->want_choice_enabled_locked(id, choice, f).first;
}

const std::string
UseConf::value_for_choice_parameter(
        const std::tr1::shared_ptr<const PackageID> & id,
        const std::tr1::shared_ptr<const Choice> & choice,
        const UnprefixedChoiceName & f
        ) const
{
    Context context("When checking parameter of flag prefix '" + stringify(choice->prefix()) +
            "' name '" + stringify(f) + "' for '" + stringify(*id) + "':");

    return _imp->handler->value_for_choice_parameter(id, choice, f);
}


std::tr1::shared_ptr<const Set<UnprefixedChoiceName> >
UseConf::known_choice_value_names(
        const std::tr1::shared_ptr<const PackageID> & id,
        const std::tr1::shared_ptr<const Choice> & choice
        ) const
{
    Context context("When loading known use expand names for prefix '" + stringify(choice->prefix()) + "':");

    return _imp->handler->known_choice_value_names(id, choice);
}

