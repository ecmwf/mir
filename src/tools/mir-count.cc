/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include <memory>

#include "eckit/log/JSON.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/FactoryOption.h"
#include "eckit/option/SimpleOption.h"

#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/key/Area.h"
#include "mir/key/grid/Grid.h"
#include "mir/key/grid/GridPattern.h"
#include "mir/param/ConfigurationWrapper.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/repres/latlon/RegularLL.h"
#include "mir/tools/Count.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/Increments.h"
#include "mir/util/Log.h"


namespace mir {
namespace tools {


using prec_t = decltype(Log::info().precision());


struct MIRCount : MIRTool {
    MIRCount(int argc, char** argv) : MIRTool(argc, argv) {
        using namespace eckit::option;

        // options_.push_back(new SimpleOption< bool >("sizes", "compare sizes of coordinates and values vectors,
        // default false"));
        options_.push_back(new FactoryOption<key::Area>("area", "cropping area"));
        options_.push_back(new FactoryOption<key::grid::GridPattern>(
            "grid", "Interpolate to given grid (following a recognizable regular expression)"));
        options_.push_back(new SimpleOption<prec_t>("precision", "Output precision"));
    }

    int minimumPositionalArguments() const override { return 0; }

    void usage(const std::string& tool) const override {
        Log::info() << "\nCount MIR representation number of values, compared to the GRIB numberOfValues."
                       "\n"
                       "\nUsage:"
                       "\n\t"
                    << tool
                    << " [--area=N/W/S/E] file.grib [file.grib [...]]"
                       "\n\t"
                    << tool
                    << " [--area=N/W/S/E] --grid=WE/SN"
                       "\n\t"
                    << tool
                    << " [--area=N/W/S/E] --grid=WE/SN --ni-nj"
                       "\n\t"
                    << tool
                    << " [--area=N/W/S/E] --grid=[fFoO][1-9][0-9]*  # ..."
                       "\n"
                       "\n"
                       "Examples:"
                       "\n\t"
                       "% "
                    << tool
                    << " 1.grib"
                       "\n\t"
                       "% "
                    << tool
                    << " --area=6/0/0/6 1.grib 2.grib"
                       "\n\t"
                       "% "
                    << tool
                    << " --area=6/0/0/6 --gridname=O1280"
                       "\n\t"
                       "% "
                    << tool << " --area=6/0/0/6 --grid=1/1 --ni-nj" << std::endl;
    }

    void execute(const eckit::option::CmdArgs& /*args*/) override;
};


void MIRCount::execute(const eckit::option::CmdArgs& args) {
    auto& log = Log::info();
    eckit::JSON j(log);

    const param::ConfigurationWrapper param(args);


    prec_t precision;
    param.get("precision", precision) ? log.precision(precision) : log.precision();

    std::vector<double> area;
    param.get("area", area);


    // setup a regular lat/lon representation and perform count
    std::string grid;
    if (key::grid::Grid::get("grid", grid, param)) {
        const auto& g = key::grid::Grid::lookup(grid, param);
        repres::RepresentationHandle rep(g.representation());

        tools::Count counter(area);
        counter.countOnRepresentation(*rep);
        counter.json(j);
        return;
    }


    // count each file(s) message(s)
    tools::Count counter(area);

    j.startObject();
    j << "files";
    j.startList();

    for (size_t i = 0, k = 0; i < args.count(); ++i, k = 0) {

        input::GribFileInput grib(args(i));
        while (grib.next()) {
            ++k;

            auto field = static_cast<const input::MIRInput&>(grib).field();
            repres::RepresentationHandle rep(field.representation());

            counter.countOnRepresentation(*rep);

            j.startObject();
            j << "file" << args(i);
            j << "fileMessage" << k;
            counter.json(j, false);
            j.endObject();
        }
    }

    j.endList();
    j.endObject();
}


}  // namespace tools
}  // namespace mir


int main(int argc, char** argv) {
    mir::tools::MIRCount tool(argc, argv);
    return tool.start();
}
