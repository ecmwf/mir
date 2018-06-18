/*
 * (C) Copyright 1996- ECMWF.
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

#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/tools/MIRTool.h"

#include "atlas/grid/Grid.h"


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
               "\n  % " << tool << " --sizes 1.grib 2.grib"
               "\n  % " << tool << " --atlas 1.grib 2.grib 3.grib"
            << std::endl;
}


void MIRGetData::execute(const eckit::option::CmdArgs& args) {
    using mir::repres::Iterator;


    bool sizes = false;
    args.get("sizes", sizes);


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
                Iterator::point_2d_t bbox_min_mir, bbox_max_mir;

                bool first = true;

                eckit::ScopedPtr< Iterator > it(rep->iterator());
                while (it->next()) {
                    const Iterator::point_2d_t& P(**it);
                    if (first) {
                        bbox_min_mir = bbox_max_mir = P;
                        first = false;
                    } else {
                        bbox_min_mir = Iterator::point_2d_t::componentsMin(bbox_min_mir, P);
                        bbox_max_mir = Iterator::point_2d_t::componentsMax(bbox_max_mir, P);
                    }
                    ++lonlat_size_mir;
                }

                size_t lonlat_size_atlas = 0;
                Iterator::point_2d_t bbox_min_atlas, bbox_max_atlas;

                atlas::Grid grid(rep->atlasGrid());
                first = true;
                for (const atlas::Grid::PointLonLat p: grid.lonlat()) {
                    Iterator::point_2d_t P(p.lat(), p.lon());
                    if (first) {
                        bbox_min_atlas = bbox_max_atlas = P;
                        first = false;
                    } else {
                        bbox_min_atlas = Iterator::point_2d_t::componentsMin(bbox_min_atlas, P);
                        bbox_max_atlas = Iterator::point_2d_t::componentsMax(bbox_max_atlas, P);
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
                        << "\n\t" "MIR   #[(lon, lat)] = " << lonlat_size_mir   << "\t" "bbox(N, W, S, E) = (" << bbox_max_mir[0]   << ", " << bbox_min_mir[1]   << ", " << bbox_min_mir[0]   << ", " << bbox_max_mir[1]   << ")"
                        << "\n\t" "Atlas #[(lon, lat)] = " << lonlat_size_atlas << "\t" "bbox(N, W, S, E) = (" << bbox_max_atlas[0] << ", " << bbox_min_atlas[1] << ", " << bbox_min_atlas[0] << ", " << bbox_max_atlas[1] << ")"
                        << std::endl;

            } else if (atlas) {

                atlas::Grid grid(rep->atlasGrid());

                auto v = field.values(0).begin();
                for (const atlas::Grid::PointLonLat p: grid.lonlat()) {
                    eckit::Log::info() << "\n\t" << p.lat() << '\t' << p.lon() << '\t' << *v;
                    ++v;
                }

                eckit::Log::info() << std::endl;
                ASSERT(v == field.values(0).end());

            } else {

                eckit::ScopedPtr< Iterator > it(rep->iterator());
                for (const double& v: field.values(0)) {
                    ASSERT(it->next());
                    const Iterator::point_2d_t& P(**it);
                    eckit::Log::info() << "\n\t" << P[0] << '\t' << P[1] << '\t' << v;
                }

                eckit::Log::info() << std::endl;
                ASSERT(!it->next());

            }

        }
    }

}


int main(int argc, char **argv) {
    MIRGetData tool(argc, argv);
    return tool.start();
}

