/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include <memory>

#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/VectorOption.h"

#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/method/Method.h"
#include "mir/namedgrids/NamedGrid.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/ConfigurationWrapper.h"
#include "mir/param/DefaultParametrisation.h"
#include "mir/param/RuntimeParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/repres/latlon/RegularLL.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Increments.h"
#include "mir/action/context/Context.h"
#include "mir/util/MIRStatistics.h"
#include "mir/data/MIRField.h"


class MIRConservative : public mir::tools::MIRTool {
private:
    void execute(const eckit::option::CmdArgs&);
    void usage(const std::string& tool) const;
    int minimumPositionalArguments() const {
        return 0;
    }
public:
    MIRConservative(int argc, char **argv) : mir::tools::MIRTool(argc, argv) {
        using eckit::option::SimpleOption;
        using eckit::option::VectorOption;

        //options_.push_back(new SimpleOption< bool >("sizes", "compare sizes of coordinates and values vectors, default false"));
        //options_.push_back(new VectorOption<double>("area", "cropping area (North/West/South/East)", 4));
        //options_.push_back(new SimpleOption<std::string>("gridname", "grid name: [FNOfno][1-9][0-9]*"));
        //options_.push_back(new VectorOption<double>("grid", "regular grid increments (West-East/South-North)", 2));
        //options_.push_back(new SimpleOption<bool>("ni-nj", "output number of increments in longitude/latitude (Ni:Nj)"));
    }
};


void MIRConservative::usage(const std::string &tool) const {
    eckit::Log::info() <<
            "\nConservative method debugging."
            "\n"
            "\nUsage:"
            "\n\t" << tool << " [--area=N/W/S/E] file.grib [file.grib [...]]"
            "\n\t" << tool << " [--area=N/W/S/E] --grid=WE/SN"
            "\n"
            "\n" "Examples:"
            "\n\t" "% " << tool << " 1.grib"
            "\n\t" "% " << tool << " --area=6/0/0/6 --grid=1/1 --ni-nj"
            << std::endl;
}


void MIRConservative::execute(const eckit::option::CmdArgs& args) {
    using namespace mir;

    static const param::DefaultParametrisation defaults;
    static const param::ConfigurationWrapper commandLine(args);
    param::RuntimeParametrisation user(commandLine);
    user.set("caching", false);

    std::unique_ptr<param::MIRParametrisation> param(new param::CombinedParametrisation(user, defaults, defaults));
    ASSERT(param);


    std::unique_ptr<method::Method> method(method::MethodFactory::build("conservative", *param));
    ASSERT(method);

    util::BoundingBox bbox{2, 0, 0, 2};
    util::Increments inc11{1, 1};
    util::Increments inc22{2, 2};

    repres::RepresentationHandle in(new repres::latlon::RegularLL(inc11, bbox));
    repres::RepresentationHandle out(new repres::latlon::RegularLL(inc22, bbox));

    auto& log = eckit::Log::info();


    data::MIRField field(in);
    {
        data::MIRValuesVector v(in->numberOfPoints(), 42.);
        field.update(v, 0);
    }

    util::MIRStatistics statistics;
    context::Context ctx(field, statistics);


    log << "before: " << ctx.field().values(0).size() << std::endl;
    method->execute(ctx, *in, *out);
    ctx.field().representation(out);
    log << "after: " << ctx.field().values(0).size() << std::endl;


    for (auto& v : ctx.field().values(0)) {
        log << v << std::endl;
    }

}


int main(int argc, char **argv) {
    MIRConservative tool(argc, argv);
    return tool.start();
}

