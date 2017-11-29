/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include <cmath>
#include <iostream>
#include "atlas/util/Earth.h"
#include "eckit/log/Log.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"
#include "eckit/option/VectorOption.h"
#include "eckit/types/FloatCompare.h"
#include "mir/param/ConfigurationWrapper.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/Angles.h"
#include "mir/util/Types.h"


class MIRSphericalArea : public mir::tools::MIRTool {
private:
    void execute(const eckit::option::CmdArgs&);
    void usage(const std::string& tool) const;
    int numberOfPositionalArguments() const { return 0; }
public:
    MIRSphericalArea(int argc, char **argv) : mir::tools::MIRTool(argc, argv) {
        options_.push_back(new eckit::option::VectorOption< double >("area", "Specify the cropping area: north/west/south/east", 4));
        options_.push_back(new eckit::option::SimpleOption< double >("radius", "Spherical radius (default Earth's radius, " + std::to_string(atlas::util::Earth::radiusInMeters()) + "m)"));
        options_.push_back(new eckit::option::SimpleOption< size_t >("precision", "Precision of floating point output"));
        options_.push_back(new eckit::option::SimpleOption< bool >("ratio", "Ratio to the Earth's surface (default false)"));
    }
};


void MIRSphericalArea::usage(const std::string &tool) const {
    eckit::Log::info()
            << "\nCalculate the spherical area between parallels and meridians."
               "\n"
               "\nUsage: " << tool << " [--area=N/W/S/E] [--radius=1.] [--ratio=false]"
            << std::endl;
}


void MIRSphericalArea::execute(const eckit::option::CmdArgs& args) {

    // Options

    const mir::param::ConfigurationWrapper args_wrap(args);

    std::vector<double> area = { 90., 0., -90., 360. };
    args_wrap.get("area", area);
    ASSERT(area.size() == 4);

    std::streamsize precision = eckit::Log::info().precision();
    args_wrap.get("precision", precision);
    eckit::Log::info().precision(precision);

    double r = atlas::util::Earth::radiusInMeters();
    args.get("radius", r);
    ASSERT(r >= 0.);

    double a_ref = 1.;
    if (args.has("ratio")) {
        a_ref = atlas::util::Earth::areaInSqMeters();
    }
    ASSERT(a_ref >= 0.);


    // Set longitude fraction
    const mir::Longitude
            W = area[1],
            E = mir::Longitude(area[3]).normalise(W),
            longitude_range(W == E && !eckit::types::is_approximately_equal(area[1], area[3]) ? 360. : E - W);
    ASSERT(0 <= longitude_range);
    ASSERT(longitude_range <= 360.);

    const double longitude_fraction = longitude_range.value() / 360.;


    // Set latitude spherical segment height
    const mir::Latitude
            N = area[0] > area[2] ? area[0] : area[2],
            S = area[0] > area[2] ? area[2] : area[0];
    ASSERT(N >= S);

    const double height = r * (
            std::sin(mir::util::degree_to_radian(N.value())) -
            std::sin(mir::util::degree_to_radian(S.value())) );
    ASSERT(height >= 0.);


    // Calculate area and ratio
    const double a = M_PI * 2. * r * height * longitude_fraction;
    const double ratio = a / a_ref;

    eckit::Log::info() << ratio << std::endl;
}


int main(int argc, char **argv) {
    MIRSphericalArea tool(argc, argv);
    return tool.start();
}

