#ifndef PALUDIS_GUARD_PALUDIS_PRINTER_HH
#define PALUDIS_GUARD_PALUDIS_PRINTER_HH 1

#include <paludis/resolver/constraint-fwd.hh>
#include <paludis/resolver/change_by_resolvent-fwd.hh>
#include <paludis/resolver/destination-fwd.hh>
#include <paludis/resolver/decision-fwd.hh>
#include <paludis/resolver/decisions-fwd.hh>
#include <paludis/resolver/job_list-fwd.hh>
#include <paludis/resolver/nag-fwd.hh>
#include <paludis/resolver/resolutions_by_resolvent-fwd.hh>
#include <paludis/resolver/resolution-fwd.hh>
#include <paludis/resolver/resolved-fwd.hh>
#include <paludis/resolver/sanitised_dependencies-fwd.hh>

#include <iostream>
namespace paludis {
std::string stringify_if_not_null(const std::shared_ptr<const paludis::resolver::Destination> & d);
std::string stringify_change_by_resolvent(const paludis::resolver::ChangeByResolvent & r);
std::string stringify_constraint(const paludis::resolver::Constraint & c);
std::ostream& operator<< (std::ostream & s, const paludis::resolver::Destination & d);
std::ostream& operator<< (std::ostream & s, const paludis::resolver::Decision & d);
std::ostream& operator<< (std::ostream & s, const paludis::resolver::SanitisedDependency & d);
std::ostream& operator<< (std::ostream & s, const paludis::resolver::Constraint & c);
std::ostream& operator<< (std::ostream & s, const paludis::resolver::Resolution & r);
template <typename Decision_, typename Notes_/* = paludis::NoType<0u> **/>
std::ostream& operator<< (std::ostream & s, const std::shared_ptr<paludis::resolver::Decisions<Decision_, Notes_ >> & d);
// template <typename Decision_>
// std::ostream& operator<< (std::ostream & s, const std::shared_ptr<paludis::resolver::Decisions<Decision_>> & d);
template <typename Job_>
std::ostream& operator<< (std::ostream & s, const std::shared_ptr<paludis::resolver::JobList<Job_>> & j) ;
std::ostream& operator<< (std::ostream & s, const std::shared_ptr<paludis::resolver::ResolutionsByResolvent> & r) ;
std::ostream& operator<< (std::ostream & s, const std::shared_ptr<paludis::resolver::NAG> & nag) ;
std::ostream& operator<< (std::ostream & s, const std::shared_ptr<paludis::resolver::Resolved> & resolved);
};
#endif
