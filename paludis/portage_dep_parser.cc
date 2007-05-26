/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/dep_spec.hh>
#include <paludis/portage_dep_lexer.hh>
#include <paludis/portage_dep_parser.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/visitor-impl.hh>
#include <stack>

/** \file
 * Implementation for dep_parser.hh things.
 *
 * \ingroup grpdepparser
 */

using namespace paludis;

DepStringParseError::DepStringParseError(const std::string & d,
        const std::string & m) throw () :
    DepStringError(d, "in parse phase: " + m)
{
}

DepStringNestingError::DepStringNestingError(const std::string & dep_string) throw () :
    DepStringParseError(dep_string, "improperly balanced parentheses")
{
}

namespace
{
    enum PortageDepParserState
    {
        dps_initial,
        dps_had_double_bar,
        dps_had_double_bar_space,
        dps_had_paren,
        dps_had_use_flag,
        dps_had_use_flag_space
    };

    template <typename H_>
    struct Composite
    {
        virtual tr1::shared_ptr<typename H_::ConstItem> as_const_item() const = 0;
        virtual void add(tr1::shared_ptr<ConstAcceptInterface<H_> >) = 0;

        virtual ~Composite()
        {
        }
    };

    template <typename H_, typename E_>
    struct RealComposite :
        Composite<H_>
    {
        tr1::shared_ptr<ConstTreeSequence<H_, E_> > _i;

        virtual tr1::shared_ptr<typename H_::ConstItem> as_const_item() const
        {
            return tr1::static_pointer_cast<typename H_::ConstItem>(_i);
        }

        virtual void add(tr1::shared_ptr<ConstAcceptInterface<H_> > i)
        {
            _i->add(i);
        }

        RealComposite(tr1::shared_ptr<ConstTreeSequence<H_, E_> > e) :
            _i(e)
        {
        }
    };

    struct ParsePackageDepSpec
    {
        PackageDepSpecParseMode _parse_mode;

        ParsePackageDepSpec(PackageDepSpecParseMode m) :
            _parse_mode(m)
        {
        }

        template <typename H_>
        void
        add(const std::string & s, tr1::shared_ptr<Composite<H_> > & p) const
        {
            p->add(tr1::shared_ptr<TreeLeaf<H_, PackageDepSpec> >(new TreeLeaf<H_, PackageDepSpec>(
                            tr1::shared_ptr<PackageDepSpec>(new PackageDepSpec(s, _parse_mode)))));
        }
    };

    struct ParsePackageOrBlockDepSpec
    {
        PackageDepSpecParseMode _parse_mode;

        ParsePackageOrBlockDepSpec(PackageDepSpecParseMode m) :
            _parse_mode(m)
        {
        }

        template <typename H_>
        void
        add(const std::string & s, tr1::shared_ptr<Composite<H_> > & p) const
        {
            if (s.empty() || '!' != s.at(0))
                p->add(tr1::shared_ptr<TreeLeaf<H_, PackageDepSpec> >(new TreeLeaf<H_, PackageDepSpec>(
                                tr1::shared_ptr<PackageDepSpec>(new PackageDepSpec(s, _parse_mode)))));
            else
                p->add(tr1::shared_ptr<TreeLeaf<H_, BlockDepSpec> >(new TreeLeaf<H_, BlockDepSpec>(
                                tr1::shared_ptr<BlockDepSpec>(new BlockDepSpec(
                                        tr1::shared_ptr<PackageDepSpec>(new PackageDepSpec(s.substr(1), _parse_mode)))))));
        }
    };

    struct ParseTextDepSpec
    {
        template <typename H_>
        void
        add(const std::string & s, tr1::shared_ptr<Composite<H_> > & p) const
        {
            p->add(tr1::shared_ptr<TreeLeaf<H_, PlainTextDepSpec> >(new TreeLeaf<H_, PlainTextDepSpec>(
                            tr1::shared_ptr<PlainTextDepSpec>(new PlainTextDepSpec(s)))));
        }
    };

    template <typename H_, bool>
    struct HandleUse
    {
        static void handle(const std::string & s, const std::string & i, std::stack<tr1::shared_ptr<Composite<H_> > > & stack)
        {
            std::string f(i);
            bool inv(f.length() && ('!' == f.at(0)));
            if (inv)
                f.erase(0, 1);

            if (f.empty())
                throw DepStringParseError(s,
                        "Bad use flag name '" + i + "'");
            if ('?' != f.at(f.length() - 1))
                throw DepStringParseError(s,
                        "Use flag name '" + i + "' missing '?'");

            f.erase(f.length() - 1);
            tr1::shared_ptr<ConstTreeSequence<H_, UseDepSpec> > a(
                    new ConstTreeSequence<H_, UseDepSpec>(tr1::shared_ptr<UseDepSpec>(
                            new UseDepSpec(UseFlagName(f), inv))));
            stack.top()->add(a);
            stack.push(tr1::shared_ptr<Composite<H_> >(new RealComposite<H_, UseDepSpec>(a)));
        }
    };

    template <typename H_>
    struct HandleUse<H_, false>
    {
        static void handle(const std::string & s, const std::string &, std::stack<tr1::shared_ptr<Composite<H_> > > &)
        {
            throw DepStringParseError(s, "use? group is not allowed here");
        }
    };

    template <typename H_, bool>
    struct HandleAny
    {
        static void handle(const std::string &, std::stack<tr1::shared_ptr<Composite<H_> > > & stack)
        {
             tr1::shared_ptr<ConstTreeSequence<H_, AnyDepSpec> > a(new ConstTreeSequence<H_, AnyDepSpec>(
                         tr1::shared_ptr<AnyDepSpec>(new AnyDepSpec)));
             stack.top()->add(a);
             stack.push(tr1::shared_ptr<Composite<H_> >(new RealComposite<H_, AnyDepSpec>(a)));
        }
    };

    template <typename H_>
    struct HandleAny<H_, false>
    {
        static void handle(const std::string & s, std::stack<tr1::shared_ptr<Composite<H_> > > &)
        {
             throw DepStringParseError(s, "|| is not allowed here");
        }
    };
}

template <typename H_, typename I_, bool any_, bool use_>
tr1::shared_ptr<typename H_::ConstItem>
PortageDepParser::_parse(const std::string & s, const I_ & p)
{
    Context context("When parsing dependency string '" + s + "':");

    std::stack<tr1::shared_ptr<Composite<H_> > > stack;
    stack.push(tr1::shared_ptr<RealComposite<H_, AllDepSpec> >(new RealComposite<H_, AllDepSpec>(
                    tr1::shared_ptr<ConstTreeSequence<H_, AllDepSpec> >(new ConstTreeSequence<H_, AllDepSpec>(
                            tr1::shared_ptr<AllDepSpec>(new AllDepSpec))))));

    PortageDepParserState state(dps_initial);
    PortageDepLexer lexer(s);
    PortageDepLexer::Iterator i(lexer.begin()), i_end(lexer.end());

    for ( ; i != i_end ; ++i)
    {
        Context local_context("When handling lexer token '" + i->second +
                "' (" + stringify(i->first) + "):");
        do
        {
            switch (state)
            {
                case dps_initial:
                    do
                    {
                        switch (i->first)
                        {
                            case dpl_whitespace:
                                 continue;

                            case dpl_text:
                                 {
                                     if (i->second.empty())
                                         throw DepStringParseError(i->second, "Empty text entry");
                                     p.template add<H_>(i->second, stack.top());
                                 }
                                 continue;

                            case dpl_open_paren:
                                 {
                                     tr1::shared_ptr<ConstTreeSequence<H_, AllDepSpec> > a(new ConstTreeSequence<H_, AllDepSpec>(
                                                 tr1::shared_ptr<AllDepSpec>(new AllDepSpec)));
                                     stack.top()->add(a);
                                     stack.push(tr1::shared_ptr<Composite<H_> >(new RealComposite<H_, AllDepSpec>(a)));
                                     state = dps_had_paren;
                                 }
                                 continue;

                            case dpl_close_paren:
                                 if (stack.empty())
                                     throw DepStringNestingError(s);
                                 stack.pop();
                                 if (stack.empty())
                                     throw DepStringNestingError(s);
                                 state = dps_had_paren;
                                 continue;

                            case dpl_double_bar:
                                 HandleAny<H_, any_>::handle(s, stack);
                                 state = dps_had_double_bar;
                                 continue;

                            case dpl_use_flag:
                                 HandleUse<H_, use_>::handle(s, i->second, stack);
                                 state = dps_had_use_flag;
                                 continue;

                        }
                        throw InternalError(PALUDIS_HERE,
                                "dps_initial: i->first is " + stringify(i->first));

                    } while (0);
                    continue;

                case dps_had_double_bar:
                    do
                    {
                        switch (i->first)
                        {
                            case dpl_whitespace:
                                state = dps_had_double_bar_space;
                                continue;

                            case dpl_text:
                            case dpl_use_flag:
                            case dpl_double_bar:
                            case dpl_open_paren:
                            case dpl_close_paren:
                                throw DepStringParseError(s, "Expected space after '||'");
                        }
                        throw InternalError(PALUDIS_HERE,
                                "dps_had_double_bar: i->first is " + stringify(i->first));

                    } while (0);
                    continue;

                case dps_had_double_bar_space:
                    do
                    {
                        switch (i->first)
                        {
                            case dpl_open_paren:
                                state = dps_initial;
                                continue;

                            case dpl_whitespace:
                            case dpl_text:
                            case dpl_use_flag:
                            case dpl_double_bar:
                            case dpl_close_paren:
                                throw DepStringParseError(s, "Expected '(' after '|| '");
                        }
                        throw InternalError(PALUDIS_HERE,
                                "dps_had_double_bar_space: i->first is " + stringify(i->first));
                    } while (0);
                    continue;

                case dps_had_paren:
                    do
                    {
                        switch (i->first)
                        {
                            case dpl_whitespace:
                                state = dps_initial;
                                continue;

                            case dpl_text:
                            case dpl_use_flag:
                            case dpl_double_bar:
                            case dpl_open_paren:
                            case dpl_close_paren:
                                throw DepStringParseError(s, "Expected space after '(' or ')'");
                        }
                        throw InternalError(PALUDIS_HERE,
                                "dps_had_paren: i->first is " + stringify(i->first));
                    } while (0);
                    continue;

                case dps_had_use_flag:
                    do
                    {
                        switch (i->first)
                        {
                            case dpl_whitespace:
                                state = dps_had_use_flag_space;
                                continue;

                            case dpl_text:
                            case dpl_use_flag:
                            case dpl_double_bar:
                            case dpl_open_paren:
                            case dpl_close_paren:
                                throw DepStringParseError(s, "Expected space after use flag");
                        }
                        throw InternalError(PALUDIS_HERE,
                                "dps_had_use_flag: i->first is " + stringify(i->first));
                    } while (0);
                    continue;

                case dps_had_use_flag_space:
                    do
                    {
                        switch (i->first)
                        {
                            case dpl_open_paren:
                                state = dps_had_paren;
                                continue;

                            case dpl_whitespace:
                            case dpl_text:
                            case dpl_use_flag:
                            case dpl_double_bar:
                            case dpl_close_paren:
                                throw DepStringParseError(s, "Expected '(' after use flag");
                        }
                        throw InternalError(PALUDIS_HERE,
                                "dps_had_use_flag_space: i->first is " + stringify(i->first));
                    } while (0);
                    continue;
            }
            throw InternalError(PALUDIS_HERE,
                    "state is " + stringify(state));

        } while (0);
    }

    if (stack.empty())
        throw DepStringNestingError(s);
    tr1::shared_ptr<Composite<H_> > result(stack.top());
    stack.pop();
    if (! stack.empty())
        throw DepStringNestingError(s);
    return result->as_const_item();
}

tr1::shared_ptr<DependencySpecTree::ConstItem>
PortageDepParser::parse_depend(const std::string & s, const PackageDepSpecParseMode m)
{
    return _parse<DependencySpecTree, ParsePackageOrBlockDepSpec, true, true>(s, ParsePackageOrBlockDepSpec(m));
}

tr1::shared_ptr<ProvideSpecTree::ConstItem>
PortageDepParser::parse_provide(const std::string & s)
{
    return _parse<ProvideSpecTree, ParsePackageDepSpec, false, true>(s, ParsePackageDepSpec(pds_pm_eapi_0));
}

tr1::shared_ptr<RestrictSpecTree::ConstItem>
PortageDepParser::parse_restrict(const std::string & s)
{
    return _parse<RestrictSpecTree, ParseTextDepSpec, false, true>(s);
}

tr1::shared_ptr<URISpecTree::ConstItem>
PortageDepParser::parse_uri(const std::string & s)
{
    return _parse<URISpecTree, ParseTextDepSpec, false, true>(s);
}

tr1::shared_ptr<LicenseSpecTree::ConstItem>
PortageDepParser::parse_license(const std::string & s)
{
    return _parse<LicenseSpecTree, ParseTextDepSpec, true, true>(s);
}

