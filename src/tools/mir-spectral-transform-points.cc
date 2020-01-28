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
#include <string>
#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"
#include "eckit/utils/StringTools.h"
#include "eckit/utils/Translator.h"

#include "mir/api/Atlas.h"
#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/repres/Representation.h"
#include "mir/tools/MIRTool.h"


class MIRSpectralTransformPoints : public mir::tools::MIRTool {
private:
    void execute(const eckit::option::CmdArgs&);

    int minimumPositionalArguments() const { return 1; }

    void usage(const std::string& tool) const {
        eckit::Log::info() << "\n"
                              "Usage: "
                           << tool
                           << " --point=N/W input1.grib [input2.grib [...]]"
                              "\n"
                              "Examples: "
                              "\n"
                              "  % "
                           << tool
                           << " --point=1/1 input.grib"
                              "\n"
                              "  % "
                           << tool << " --pont=\"1/1 2/2\" input.grib" << std::endl;
    }

public:
    MIRSpectralTransformPoints(int argc, char** argv) : MIRTool(argc, argv) {
        options_.push_back(
            new eckit::option::SimpleOption<std::string>("point", "lat/lon coordinate pair(s), space-separated"));
    }
};


void MIRSpectralTransformPoints::execute(const eckit::option::CmdArgs& args) {
    auto& log = eckit::Log::info();
    eckit::Translator<std::string, double> to_double;

    std::string point = args.getString("point");

    auto pts = new std::vector<atlas::PointXY>;  // pointer, to transfer ownership to UnstructuredGrid
    for (auto& pt : eckit::StringTools::split(" ", point)) {
        auto ll = eckit::StringTools::split("/", pt);
        if (ll.size() != 2) {
            throw eckit::UserError("Expecting lat/lon, got '" + pt + "'");
        }
        pts->push_back({to_double(ll[1]), to_double(ll[0])});
    }

    ASSERT(!pts->empty());
    auto grid = atlas::UnstructuredGrid(pts);


    // loop over each file(s) message(s)
    for (size_t a = 0; a < args.count(); ++a) {
        for (std::unique_ptr<mir::input::MIRInput> input(new mir::input::GribFileInput(args(a))); input->next();) {

            auto field = input->field();
            ASSERT(field.dimensions() == 1);

            size_t T = field.representation()->truncation();
            ASSERT(T > 0);

            mir::MIRValuesVector out(size_t(grid.size()), 0.);

            atlas::trans::Trans trans(grid, int(T), atlas::util::Config("type", "local"));
            trans.invtrans(1, field.values(0).data(), out.data(), atlas::option::global());

            for (atlas::idx_t i = 0; i < grid.size(); ++i) {
                auto p = grid.lonlat(i);
                log << p.lat() << '\t' << p.lon() << '\t' << out[size_t(i)] << std::endl;
            }
        }
    }
}


int main(int argc, char** argv) {
    MIRSpectralTransformPoints tool(argc, argv);
    return tool.start();
}
