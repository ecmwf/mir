/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Jun 2017


#include "eckit/log/Log.h"
#include "eckit/memory/ScopedPtr.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"
#include "atlas/grid/Grid.h"
#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/stats/detail/Angle.h"
#include "mir/tools/MIRTool.h"
#include "mir/method/decompose/PolarAngleToCartesian.h"


class MIRGetData : public mir::tools::MIRTool {
private:
    void execute(const eckit::option::CmdArgs&);
    void usage(const std::string& tool) const;
    int minimumPositionalArguments() const {
        return 1;
    }
public:
    MIRGetData(int argc, char **argv) : mir::tools::MIRTool(argc, argv) {
        options_.push_back(new eckit::option::SimpleOption< bool >("atlas", "output Atlas coordinates, default false"));
        options_.push_back(new eckit::option::SimpleOption< bool >("diff", "default false"));
    }
};


void MIRGetData::usage(const std::string &tool) const {
    eckit::Log::info()
            << "\nUsage: " << tool << " [--atlas=[true|false]] file.grib [file.grib [...]]"
               "\nExample:"
               "\n  % " << tool << " --atlas=true file1.grib file2.grib file3.grib"
            << std::endl;
}


void MIRGetData::execute(const eckit::option::CmdArgs& args) {


    bool diff = false;
    args.get("diff", diff);


    bool atlas = false;
    args.get("atlas", atlas);


    for (size_t i = 0; i < args.count(); ++i) {
        mir::input::GribFileInput grib(args(i));
        const mir::input::MIRInput& input = grib;


        size_t count = 0;
        while (grib.next()) {
            eckit::Log::info() << "\n'" << args(i) << "' #" << ++count << std::endl;

            mir::data::MIRField field = input.field();
            ASSERT(field.dimensions() == 1);

            mir::repres::RepresentationHandle rep(field.representation());

            if (diff) {

                mir::method::decompose::PolarAngleToCartesian<mir::data::FieldInfo::CYLINDRICAL_ANGLE_DEGREES_SYMMETRIC>
                        decompose(std::numeric_limits<double>::quiet_NaN());

                mir::stats::detail::Angle
                        stats_lat(decompose, std::numeric_limits<double>::quiet_NaN()),
                        stats_lon(decompose, std::numeric_limits<double>::quiet_NaN());

                eckit::ScopedPtr<atlas::grid::Grid> grid(rep->atlasGrid());
                eckit::ScopedPtr< mir::repres::Iterator > it(rep->rotatedIterator());

                std::vector<double>::const_iterator v = field.values(0).begin();
                std::vector<atlas::grid::Grid::Point> lonlat;
                grid->lonlat(lonlat);

                double lon;
                double lat;
                for (const atlas::grid::Grid::Point& p: lonlat) {
                    ASSERT(it->next(lat, lon));

                    stats_lat(p.lat() - lat);
                    stats_lon(p.lon() - lon);

                    ++v;
                }

                eckit::Log::info()
                        << "\n\t" "Δlat (mean, variance, std) = (" << stats_lat.mean() << ", " << stats_lat.variance() << ", " << stats_lat.standardDeviation() << ")"
                        << "\n\t" "Δlon (mean, variance, std) = (" << stats_lon.mean() << ", " << stats_lon.variance() << ", " << stats_lon.standardDeviation() << ")"
                        << std::endl;

                ASSERT(v == field.values(0).end());
                ASSERT(!it->next(lat, lon));

            } else if (atlas) {

                eckit::ScopedPtr<atlas::grid::Grid> grid(rep->atlasGrid());

                std::vector<double>::const_iterator v = field.values(0).begin();
                std::vector<atlas::grid::Grid::Point> lonlat;
                grid->lonlat(lonlat);

                for (const atlas::grid::Grid::Point& p: lonlat) {
                    eckit::Log::info() << "\n\t" << p.lat() << '\t' << p.lon() << '\t' << *v;
                    ++v;
                }

                eckit::Log::info() << std::endl;
                ASSERT(v == field.values(0).end());

            } else {

                eckit::ScopedPtr< mir::repres::Iterator > it(rep->rotatedIterator());
                double lon;
                double lat;
                for (const double& v: field.values(0)) {
                    ASSERT(it->next(lat, lon));
                    eckit::Log::info() << "\n\t" << lat << '\t' << lon << '\t' << v;
                }

                eckit::Log::info() << std::endl;
                ASSERT(!it->next(lat, lon));

            }

        }
    }

}


int main(int argc, char **argv) {
    MIRGetData tool(argc, argv);
    return tool.start();
}

