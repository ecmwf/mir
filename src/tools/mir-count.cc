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
#include "eckit/option/SimpleOption.h"
#include "eckit/option/VectorOption.h"

#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/key/grid/NamedGrid.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/repres/latlon/RegularLL.h"
#include "mir/tools/Count.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/Increments.h"


using prec_t = decltype(std::cout.precision());


class MIRCount : public mir::tools::MIRTool {
private:
    void execute(const eckit::option::CmdArgs&);

    int minimumPositionalArguments() const { return 0; }

    void usage(const std::string& tool) const {
        eckit::Log::info() << "\nCount MIR representation number of values, compared to the GRIB numberOfValues."
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
                           << " [--area=N/W/S/E] --gridname=[FNOfno][1-9][0-9]*  # ..."
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

public:
    MIRCount(int argc, char** argv) : MIRTool(argc, argv) {
        using eckit::option::SimpleOption;
        using eckit::option::VectorOption;

        // options_.push_back(new SimpleOption< bool >("sizes", "compare sizes of coordinates and values vectors,
        // default false"));
        options_.push_back(new VectorOption<double>("area", "cropping area (North/West/South/East)", 4));
        options_.push_back(new SimpleOption<std::string>("gridname", "grid name: [FNOfno][1-9][0-9]*"));
        options_.push_back(new VectorOption<double>("grid", "regular grid increments (West-East/South-North)", 2));
        options_.push_back(new SimpleOption<prec_t>("precision", "Output precision"));
    }
};


void MIRCount::execute(const eckit::option::CmdArgs& args) {
    using mir::util::BoundingBox;

    auto& log = eckit::Log::info();
    eckit::JSON j(log);

    prec_t precision;
    args.get("precision", precision) ? log.precision(precision) : log.precision();

    std::vector<double> area;
    args.get("area", area);


    // setup a regular lat/lon representation and perfom count
    std::vector<double> grid;
    if (args.get("grid", grid)) {
        ASSERT(!args.has("gridname"));

        mir::tools::Count counter(area);
        counter.countOnGridIncrements(grid);

        counter.json(j);
        return;
    }


    // setup a representation from gridname and perfom count
    std::string gridname;
    if (args.get("gridname", gridname)) {
        ASSERT(!args.has("grid"));

        mir::tools::Count counter(area);
        counter.countOnNamedGrid(gridname);

        counter.json(j);
        return;
    }


    // count each file(s) message(s)
    mir::tools::Count counter(area);

    j.startObject();
    j << "files";
    j.startList();

    for (size_t i = 0, k = 0; i < args.count(); ++i, k = 0) {

        mir::input::GribFileInput grib(args(i));
        while (grib.next()) {
            ++k;

            auto field = static_cast<const mir::input::MIRInput&>(grib).field();
            mir::repres::RepresentationHandle rep(field.representation());

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


int main(int argc, char** argv) {
    MIRCount tool(argc, argv);
    return tool.start();
}
