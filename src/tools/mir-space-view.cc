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


#include <functional>
#include <memory>
#include <random>

#include "eckit/option/CmdArgs.h"
#include "eckit/types/FloatCompare.h"

#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir {
namespace tools {


double bisection_method(double x_min, double x_max, std::function<double(double)> f, double f0 = 0.,
                        double f_eps = 1.e-9) {
    static std::random_device rd;
    static const auto entropy = rd();
    std::mt19937 gen(entropy);
    std::uniform_real_distribution<double> dis(x_min, x_max);

    auto xp = dis(gen);
    while (f(xp) < f0) {
        xp = dis(gen);
    }

    auto xm = dis(gen);
    while (f(xm) > f0) {
        xm = dis(gen);
    }

    auto x = (xp + xm) / 2.;
    for (auto fx = f(x) - f0; !eckit::types::is_approximately_equal(fx, 0., f_eps);
         x = (xp + xm) / 2., fx = f(x) - f0) {
        (fx >= 0. ? xp : xm) = x;
    }

    return x;
}


struct MIRSpaceView : MIRTool {
    using MIRTool::MIRTool;

    int numberOfPositionalArguments() const override { return 1; }

    void usage(const std::string& tool) const override {
        Log::info() << "\nUsage: " << tool << " file.grib" << std::endl;
    }

    void execute(const eckit::option::CmdArgs&) override;
};


void MIRSpaceView::execute(const eckit::option::CmdArgs& args) {
    ASSERT(args.count() == 1);

    std::unique_ptr<input::MIRInput> input(new input::GribFileInput(args(0)));
    ASSERT(input->next());

    auto field = input->field();
    // auto missingValue  = field.missingValue();
    // const auto& values = field.values(0);

    repres::RepresentationHandle rep(field.representation());
    // for (std::unique_ptr<repres::Iterator> it(rep->iterator()); it->next();) {
    // }

    auto grid  = rep->atlasGrid();
    auto& proj = grid.projection();

    auto& log = Log::info();
    log.precision(32);

    log << proj.spec() << std::endl;

    auto lat = [&proj](const double x) { return proj.lonlat({0, x}).lat(); };
    auto lon = [&proj](const double x) { return proj.lonlat({x, 0}).lon(); };
    auto x   = [&proj](const double x) { return proj.xy({x, 0}).x(); };
    auto y   = [&proj](const double x) { return proj.xy({0, x}).y(); };

    auto r = bisection_method(0, 1e7, lat, 81.3, 1e-6);
    log << "f(" << r << ")=" << lat(r) << " [N] r=" << y(lat(r)) << std::endl;

    r = bisection_method(-1e7, 0, lat, -81.3, 1e-6);
    log << "f(" << r << ")=" << lat(r) << " [S] r=" << y(lat(r)) << std::endl;

    r = bisection_method(-1e7, 0, lon, -81.242, 1e-6);
    log << "f(" << r << ")=" << lon(r) << " [W] r=" << x(lon(r)) << std::endl;

    r = bisection_method(0, 1e7, lon, 81.242, 1e-6);
    log << "f(" << r << ")=" << lon(r) << " [E] r=" << x(lon(r)) << std::endl;
}


}  // namespace tools
}  // namespace mir


int main(int argc, char** argv) {
    mir::tools::MIRSpaceView tool(argc, argv);
    return tool.start();
}
