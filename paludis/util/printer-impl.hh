#ifndef PALUDIS_GUARD_PALUDIS_PRINTER_IMPL_HH
#define PALUDIS_GUARD_PALUDIS_PRINTER_IMPL_HH 1


#include <paludis/util/printer.hh>

#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/change_by_resolvent.hh>
#include <paludis/resolver/destination.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/resolver/decisions.hh>
#include <paludis/resolver/job.hh>
#include <paludis/resolver/job_list.hh>
#include <paludis/resolver/job_lists.hh>
#include <paludis/resolver/nag.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/resolutions_by_resolvent.hh>
#include <paludis/resolver/resolved.hh>
#include <paludis/resolver/collect_depped_upon.hh>

#include <paludis/util/stringify.hh>
#include <paludis/util/enum_iterator.hh>
#include <paludis/util/join.hh>
#include <paludis/util/indirect_iterator.hh>
#include <paludis/serialise.hh>

#include <sstream>
#include <type_traits>

namespace paludis {
using namespace paludis::resolver;

std::ostream &
operator<< (std::ostream & s, const Destination & d)
{
  std::stringstream ss;
  ss << "Destination(" << d.repository();
  if (! d.replacing()->empty())
    ss << " replacing " << join(indirect_iterator(d.replacing()->begin()),
                                indirect_iterator(d.replacing()->end()), ", ");
  ss << ")";

  s << ss.str();
  return s;
}

std::string stringify_if_not_null(const std::shared_ptr<const Destination> & d)
{
  if (d)
  {
    std::stringstream ss;
    ss << *d;
    return ss.str();
  }
  else
    return "null";
}

std::string stringify_change_by_resolvent(const ChangeByResolvent & r)
{
  return stringify(*r.package_id());
}

struct DecisionStringifier
{
  const std::string visit(const UnableToMakeDecision & d) const
  {
    return "UnableToMakeDecision(taken: " + stringify(d.taken()) + ")";
  }

  const std::string visit(const RemoveDecision & d) const
  {
    return "RemoveDecision(ids: " + join(indirect_iterator(d.ids()->begin()),
                                          indirect_iterator(d.ids()->end()), ", ") + " taken: " + stringify(d.taken()) + ")";
  }

  const std::string visit(const NothingNoChangeDecision & d) const
  {
    return "NothingNoChangeDecision(taken: " + stringify(d.taken()) + ")";
  }

  const std::string visit(const ExistingNoChangeDecision & d) const
  {
    std::string attrs;
    for (EnumIterator<ExistingPackageIDAttribute> t, t_end(last_epia) ; t != t_end ; ++t)
      if (d.attributes()[*t])
      {
        if (! attrs.empty())
          attrs += ", ";
        attrs += stringify(*t);
      }

    return "ExistingNoChangeDecision(" + stringify(*d.existing_id()) + " " +
      attrs + " taken: " + stringify(d.taken()) + ")";
  }

  const std::string visit(const ChangesToMakeDecision & d) const
  {
    return "ChangesToMakeDecision(" + stringify(*d.origin_id()) + " best: "
    + stringify(d.best()) + " taken: " + stringify(d.taken())
    + " destination: " + stringify_if_not_null(d.destination())
    + ")";
  }

  const std::string visit(const BreakDecision & d) const
  {
    return "BreakDecision(taken: " + stringify(d.taken()) + " existing_id: " + stringify(*d.existing_id()) + ")";
  }
};

std::ostream &
operator<< (std::ostream & s, const Decision & d)
{
  s << d.accept_returning<std::string>(DecisionStringifier());
  return s;
}

std::ostream &
operator<< (std::ostream & s, const SanitisedDependency & d)
{
  std::stringstream ss;
  ss << "Dep(";
  if (! d.metadata_key_raw_name().empty())
    ss << d.metadata_key_raw_name() << " ";
  ss << d.spec();

  ss << " { " << join(indirect_iterator(d.active_dependency_labels()->begin()),
                      indirect_iterator(d.active_dependency_labels()->end()), ", ") << " }";

                      ss << ")";

                      s << ss.str();
                      return s;
}

struct ReasonFinder
{
  std::string str;

  ReasonFinder() :
  str("none")
  {
  }

  void visit(const TargetReason & r)
  {
    str = "Target(" + r.extra_information() + ")";
  }

  void visit(const PresetReason &)
  {
    str = "Preset";
  }

  void visit(const SetReason & r)
  {
    ReasonFinder f;
    if (r.reason_for_set())
      r.reason_for_set()->accept(f);
    str = "Set(" + stringify(r.set_name()) + " " + f.str + ")";
  }

  void visit(const LikeOtherDestinationTypeReason & r)
  {
    ReasonFinder f;
    if (r.reason_for_other())
      r.reason_for_other()->accept(f);
    str = "LikeOtherDestinationTypeReason(" + stringify(r.other_resolvent()) + " " + f.str + ")";
  }

  void visit(const ViaBinaryReason & r)
  {
    str = "ViaBinaryReason(" + stringify(r.other_resolvent()) + ")";
  }

  void visit(const DependentReason & r)
  {
    str = "Dependent(" + stringify(*r.dependent_upon().package_id()) + ", " +
    r.dependent_upon().active_dependency_labels_as_string() + ")";
  }

  void visit(const WasUsedByReason & r)
  {
    str = "WasUsedBy(" + join(r.ids_and_resolvents_being_removed()->begin(),
                              r.ids_and_resolvents_being_removed()->end(), ", ", stringify_change_by_resolvent) + ")";
  }

  void visit(const DependencyReason & r)
  {
    std::stringstream s;
    s << r.sanitised_dependency();
    str = "Dependency(" + s.str() + " from " + stringify(r.from_resolvent()) + ")";
  }
};

std::ostream &
operator<< (std::ostream & s, const Constraint & c)
{
  std::stringstream ss;
  ss << "Constraint(spec: " << c.spec();
  if (c.nothing_is_fine_too())
    ss << "; nothing is fine too";
  ss
  << "; untaken: " << stringify(c.untaken())
  << "; use_existing: " << stringify(c.use_existing())
  << "; reason: ";

  ReasonFinder r;
  if (c.reason())
    c.reason()->accept(r);
  ss
  << r.str << ")";
  s << ss.str();

  return s;
}

std::string stringify_constraint(const Constraint & c)
{
  std::stringstream s;
  s << c;
  return s.str();
}

std::ostream &
operator<< (std::ostream & s, const Resolution & r)
{
  std::stringstream ss;
  ss <<  "Resolution("
     << "constraints: " << join(indirect_iterator(r.constraints()->begin()),
                                indirect_iterator(r.constraints()->end()), ", ", stringify_constraint)
     << "; decision: ";
  if (r.decision())
    ss << *r.decision();
  else
    ss << "none";
  ss << ")";
  s << ss.str();
  return s;
}

template <typename Decision_, typename Notes_>
std::ostream &
operator<< (std::ostream & s, const std::shared_ptr<Decisions<Decision_, Notes_ >> & d)
{
  for (const auto& c: *d)
  {
    Serialiser ser(s);
    if constexpr (std::is_same<Notes_,paludis::NoType<0u> * >::value)
      c->serialise(ser);
    else
      c.first->serialise(ser);

    s << '\n';
  }
  return s;
}

template <typename Job_>
std::ostream &
operator<< (std::ostream & s, const std::shared_ptr<JobList<Job_>> & j)
{
  s << " (size=" << j->length() << ")\n";
  for (auto& c: *j)
  {
    Serialiser ser(s);
    c->serialise(ser);
    s << '\n';
  }
  return s;
}

std::ostream &
operator<< (std::ostream & s, const std::shared_ptr<ResolutionsByResolvent> & r)
{
  for (auto& c: *r)
  {
    Serialiser ser(s);
    c->serialise(ser);
    s << '\n';
  }
  return s;
}

std::ostream&
operator<< (std::ostream & s, const std::shared_ptr<NAG> & nag)
{
  Serialiser ser(s);
  nag->serialise(ser);
  s << '\n';
  return s;
}

std::ostream&
operator<< (std::ostream & s, const std::shared_ptr<Resolved> & resolved)
{
  s << '\n'
  << "#####" << '\n'
  << "job_lists:\n"
  << "  execute_job_list" << resolved->job_lists()->execute_job_list()
  << "  pretend_job_list" << resolved->job_lists()->pretend_job_list()

  << "decisions:\n"
  << "taken_change_or_remove_decisions:\n"
  << resolved->taken_change_or_remove_decisions() << '\n'
  << "taken_unable_to_make_decisions:\n"
  << resolved->taken_unable_to_make_decisions() << '\n'
  << "taken_unconfirmed_decisions:\n"
  << resolved->taken_unconfirmed_decisions() << '\n'
  << "taken_unorderable_decisions:\n"
  << resolved->taken_unorderable_decisions() << '\n'
  << "untaken_change_or_remove_decisions:\n"
  << resolved->untaken_change_or_remove_decisions() << '\n'
  << "untaken_unable_to_make_decisions:\n"
  << resolved->untaken_unable_to_make_decisions() << '\n'

  << "nag:\n"
  << resolved->nag() << '\n'
  << "resolutions_by_resolvent:\n"
  << resolved->resolutions_by_resolvent() << '\n'
  << "#####"<< std::endl
;
//     auto f = resolved->taken_change_or_remove_decisions();
//     foo::operator<<(std::cout, f);
//     std::cout << std::endl;
  return s;
}
};

#endif
