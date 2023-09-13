#ifndef PALUDIS_GUARD_PALUDIS_SERIALISE_PP_IMPL_HH
#define PALUDIS_GUARD_PALUDIS_SERIALISE_PP_IMPL_HH 1

#include <paludis/serialise_pp.hh>
#include <paludis/util/simple_parser.hh>

namespace{
    const int indent_size = 2;
    using namespace paludis;

    void
    indent(std::string& out, const int& level)
    {
        for(int m = 0; m < level; ++m)
            for(int n = 0; n < indent_size; ++n)
                out+=" ";
    }
    void
    parse(SimpleParser & parser, std::string& out)
    {
        int level = 0;
        while (true)
        {
            Context context("When parsing from offset '" + stringify(parser.offset()) + "':");
            std::string word;

            if (parser.eof())
            {
                    return;
            }
            else if (parser.consume(simple_parser::exact("(")))
            {
                out += "(\n";
                ++level;
            }
            else if (parser.consume(simple_parser::exact(");")))
            {
                indent(out, --level);
                out += ");\n";
            }
            else if (parser.consume(simple_parser::exact(";")))
            {
                out += ";\n";
            }
            else if (parser.consume(+simple_parser::any_except("();") >> word))
            {
                if(word.at(word.length()-1) == '\\')
                    if (! parser.consume(+simple_parser::any_except(";") >> word))
                        throw InternalError(PALUDIS_HERE, "Got");

                indent(out, level);
                out += word;
            }
            else
                throw InternalError(PALUDIS_HERE, "Got error 'Unexpected trailing text' parsing '" + parser.text() + "' " + std::to_string(parser.eof()) + " " + std::to_string(parser.offset()) + " " + std::to_string(parser.text().length()) + " "  + parser.text().substr(parser.offset(),parser.offset()+10) +
                "', but the error function returned");
                // error(parser, callbacks, "Unexpected trailing text");
        }
    }
};

std::string paludis::SerialiserPrettyPrinter::operator()(const std::string& s )
{
    Context context("When parsing '" + s + "':");
    SimpleParser parser(s);
    std::string out;
    parse(parser, out);
    return out;

}

std::string a(R"(Resolved(job_lists=JobLists(execute_job_list=JobList(items=c(count="0";););pretend_job_list=JobList(items=c(count="0";);););nag=NAG(nodes=c(count="0";);edge.count="0";);resolutions_by_resolvent=ResolutionsByResolvent(items=c(1=Resolution(constraints=Constraints(items=c(1=Constraint(destination_type="install_to_slash";force_unable="false";from_id=null;nothing_is_fine_too="false";reason=TargetReason(extra_information="";);spec=PackageOrBlockDepSpec(block="false";spec="=cat/test-11";annotations_count="0";);untaken="false";use_existing="never";);count="1";););decision=ChangesToMakeDecision(resolvent=Resolvent(destination_type="install_to_slash";package="cat/test";slot=SlotNameOrNull(name_or_null="0";null_means_unknown="false";););origin_id="=cat/test-11:0::test-repo";if_changed_choices=null;best="true";change_type="reinstall";destination=Destination(replacing=c(1="=cat/test-11:0::installed";count="1";);repository="installed";);if_via_new_binary_in=null;taken="true";required_confirmations_if_any=null;);resolvent=Resolvent(destination_type="install_to_slash";package="cat/test";slot=SlotNameOrNull(name_or_null="0";null_means_unknown="false";);););count="1";););taken_change_or_remove_decisions=Decisions(count="0";);taken_unable_to_make_decisions=Decisions(count="0";);taken_unconfirmed_decisions=Decisions(count="0";);taken_unorderable_decisions=Decisions(count="0";);untaken_change_or_remove_decisions=Decisions(count="0";);untaken_unable_to_make_decisions=Decisions(count="0";););)");
std::string b(R"(Resolved(job_lists=JobLists(execute_job_list=JobList(items=c(1=FetchJob(requirements=c(count="0";);origin_id_spec="=cat/test-11:0::test-repo";state=null;was_target="true";);2=InstallJob(requirements=c(1=JobRequirement(job_number="0";required_if="require_for_satisfied,require_for_independent,require_always,fetching";);count="1";);origin_id_spec="=cat/test-11:0::test-repo";destination_repository_name="installed";destination_type="install_to_slash";replacing_specs=c(1="=cat/test-11:0::installed";count="1";);state=null;was_target="true";);count="2";););pretend_job_list=JobList(items=c(1=PretendJob(origin_id_spec="=cat/test-11:0::test-repo";destination_type="install_to_slash";destination_repository_name="installed";);count="1";);););nag=NAG(nodes=c(1=NAGIndex(resolvent=Resolvent(destination_type="install_to_slash";package="cat/test";slot=SlotNameOrNull(name_or_null="0";null_means_unknown="false";););role="done";);2=NAGIndex(resolvent=Resolvent(destination_type="install_to_slash";package="cat/test";slot=SlotNameOrNull(name_or_null="0";null_means_unknown="false";););role="fetched";);count="2";);edge.1.f=NAGIndex(resolvent=Resolvent(destination_type="install_to_slash";package="cat/test";slot=SlotNameOrNull(name_or_null="0";null_means_unknown="false";););role="done";);edge.1.t=NAGIndex(resolvent=Resolvent(destination_type="install_to_slash";package="cat/test";slot=SlotNameOrNull(name_or_null="0";null_means_unknown="false";););role="fetched";);edge.1.p=NAGEdgeProperties(always="false";build="true";build_all_met="false";run="false";run_all_met="true";);edge.count="1";);resolutions_by_resolvent=ResolutionsByResolvent(items=c(1=Resolution(constraints=Constraints(items=c(1=Constraint(destination_type="install_to_slash";force_unable="false";from_id=null;nothing_is_fine_too="false";reason=TargetReason(extra_information="";);spec=PackageOrBlockDepSpec(block="false";spec="=cat/test-11";annotations_count="0";);untaken="false";use_existing="never";);count="1";););decision=ChangesToMakeDecision(resolvent=Resolvent(destination_type="install_to_slash";package="cat/test";slot=SlotNameOrNull(name_or_null="0";null_means_unknown="false";););origin_id="=cat/test-11:0::test-repo";if_changed_choices=null;best="true";change_type="reinstall";destination=Destination(replacing=c(1="=cat/test-11:0::installed";count="1";);repository="installed";);if_via_new_binary_in=null;taken="true";required_confirmations_if_any=null;);resolvent=Resolvent(destination_type="install_to_slash";package="cat/test";slot=SlotNameOrNull(name_or_null="0";null_means_unknown="false";);););count="1";););taken_change_or_remove_decisions=Decisions(1.1=ChangesToMakeDecision(resolvent=Resolvent(destination_type="install_to_slash";package="cat/test";slot=SlotNameOrNull(name_or_null="0";null_means_unknown="false";););origin_id="=cat/test-11:0::test-repo";if_changed_choices=null;best="true";change_type="reinstall";destination=Destination(replacing=c(1="=cat/test-11:0::installed";count="1";);repository="installed";);if_via_new_binary_in=null;taken="true";required_confirmations_if_any=null;);1.2=OrdererNotes(cycle_breaking="";);count="1";);taken_unable_to_make_decisions=Decisions(count="0";);taken_unconfirmed_decisions=Decisions(count="0";);taken_unorderable_decisions=Decisions(count="0";);untaken_change_or_remove_decisions=Decisions(count="0";);untaken_unable_to_make_decisions=Decisions(count="0";););)");

#endif
