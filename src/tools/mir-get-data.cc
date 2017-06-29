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


#include "eckit/geometry/Point2.h"
#include "eckit/log/Log.h"
#include "eckit/memory/ScopedPtr.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"
#include "atlas/grid.h"
#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/stats/detail/Angle.h"
#include "mir/tools/MIRTool.h"
#include "mir/method/decompose/PolarAngleToCartesian.h"
#include "mir/util/MIRGrid.h"


class MIRGetData : public mir::tools::MIRTool {
private:
    void execute(const eckit::option::CmdArgs&);
    void usage(const std::string& tool) const;
    int minimumPositionalArguments() const {
        return 1;
    }
public:
    MIRGetData(int argc, char **argv) : mir::tools::MIRTool(argc, argv) {
        using eckit::option::SimpleOption;
        options_.push_back(new SimpleOption< bool >("sizes", "compare sizes of coordinates and values vectors, default false"));
        options_.push_back(new SimpleOption< bool >("atlas-diff", "compare Atlas/MIR coordinates, default false"));
        options_.push_back(new SimpleOption< bool >("atlas", "output Atlas coordinates instead of MIR coordinates, default false"));
    }
};


void MIRGetData::usage(const std::string &tool) const {
    eckit::Log::info()
            << "\nPrint a latitude, longitude, data values list."
               "\n"
               "\nUsage: " << tool << " [--sizes=[true|false]] [--diff=[true|false]] [--atlas=[true|false]] file.grib [file.grib [...]]"
               "\nExamples:"
               "\n  % " << tool << " 1.grib"
               "\n  % " << tool << " --sizes 1.grib"
               "\n  % " << tool << " --atlas-diff 1.grib 2.grib 3.grib"
            << std::endl;
}


void MIRGetData::execute(const eckit::option::CmdArgs& args) {
    typedef eckit::geometry::Point2 point_t;


    bool sizes = false;
    args.get("sizes", sizes);


    bool diff = false;
    args.get("atlas-diff", diff);


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

            if (sizes) {

                size_t lonlat_size_mir = 0;
                point_t bbox_min_mir, bbox_max_mir;

                bool first = true;

                mir::Longitude lon;
                mir::Latitude lat;

                eckit::ScopedPtr< mir::repres::Iterator > it(rep->iterator());
                while (it->next(lat, lon)) {
                    point_t P(lon.value(), lat.value());
                    if (first) {
                        bbox_min_mir = bbox_max_mir = P;
                        first = false;
                    } else {
                        bbox_min_mir = point_t::componentsMin(bbox_min_mir, P);
                        bbox_max_mir = point_t::componentsMax(bbox_max_mir, P);
                    }
                    ++lonlat_size_mir;
                }

                size_t lonlat_size_atlas = 0;
                point_t bbox_min_atlas, bbox_max_atlas;

                atlas::Grid grid(rep->grid());
                first = true;
                for (const atlas::Grid::PointLonLat p: grid.lonlat()) {
                    point_t P(p.lon(), p.lat());
                    if (first) {
                        bbox_min_atlas = bbox_max_atlas = P;
                        first = false;
                    } else {
                        bbox_min_atlas = point_t::componentsMin(bbox_min_atlas, P);
                        bbox_max_atlas = point_t::componentsMax(bbox_max_atlas, P);
                    }
                    ++lonlat_size_atlas;
                }

                bool validates = false;
                try {
                    rep->validate(field.values(0));
                    validates = true;
                } catch (...) {
                }

                eckit::Log::info()
                        <<   "\t" "MIR   #values       = " << field.values(0).size() << "\t" "validates? " << (validates? "yes":"no")
                        << "\n\t" "MIR   #[(lon, lat)] = " << lonlat_size_mir   << "\t" "bbox(N, W, S, E) = (" << bbox_max_mir[1]   << ", " << bbox_min_mir[0]   << ", " << bbox_min_mir[1]   << ", " << bbox_max_mir[0]   << ")"
                        << "\n\t" "Atlas #[(lon, lat)] = " << lonlat_size_atlas << "\t" "bbox(N, W, S, E) = (" << bbox_max_atlas[1] << ", " << bbox_min_atlas[0] << ", " << bbox_min_atlas[1] << ", " << bbox_max_atlas[0] << ")"
                        << std::endl;

            } else if (diff) {

                mir::method::decompose::PolarAngleToCartesian<mir::data::FieldInfo::CYLINDRICAL_ANGLE_DEGREES_SYMMETRIC>
                        decompose(std::numeric_limits<double>::quiet_NaN());

                mir::stats::detail::Angle
                        stats_lat(decompose, std::numeric_limits<double>::quiet_NaN()),
                        stats_lon(decompose, std::numeric_limits<double>::quiet_NaN());

                atlas::Grid grid(rep->grid());
                eckit::ScopedPtr< mir::repres::Iterator > it(rep->iterator());

                std::vector<double>::const_iterator v = field.values(0).begin();

                mir::Longitude lon;
                mir::Latitude lat;
                for (const atlas::Grid::PointLonLat p: grid.lonlat()) {
                    ASSERT(it->next(lat, lon));

                    stats_lat(p.lat() - lat.value());
                    stats_lon(p.lon() - lon.value());

                    ++v;
                }

                eckit::Log::info()
                        << "\n\t" "Δlat (mean, variance, std) = (" << stats_lat.mean() << ", " << stats_lat.variance() << ", " << stats_lat.standardDeviation() << ")"
                        << "\n\t" "Δlon (mean, variance, std) = (" << stats_lon.mean() << ", " << stats_lon.variance() << ", " << stats_lon.standardDeviation() << ")"
                        << std::endl;

                ASSERT(v == field.values(0).end());
                ASSERT(!it->next(lat, lon));

            } else if (atlas) {

                atlas::Grid grid(rep->grid());

                std::vector<double>::const_iterator v = field.values(0).begin();
                for (const atlas::Grid::PointLonLat p: grid.lonlat()) {
                    eckit::Log::info() << "\n\t" << p.lat() << '\t' << p.lon() << '\t' << *v;
                    ++v;
                }

                eckit::Log::info() << std::endl;
                ASSERT(v == field.values(0).end());

            } else {

                eckit::ScopedPtr< mir::repres::Iterator > it(rep->iterator());
                mir::Longitude lon;
                mir::Latitude lat;
                for (const double& v: field.values(0)) {
                    ASSERT(it->next(lat, lon));
                    eckit::Log::info() << "\n\t" << lat << '\t' << lon.value() << '\t' << v;
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

