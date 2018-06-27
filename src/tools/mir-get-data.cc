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


#include <vector>

#include "eckit/log/Log.h"
#include "eckit/memory/ScopedPtr.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"

#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/stats/detail/ScalarMinMaxFn.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/Grib.h"

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
        options_.push_back(new SimpleOption< bool >("diff-atlas", "compare to Atlas coordinates, default false"));
        options_.push_back(new SimpleOption< bool >("diff-ecc", "compare to ecCodes coordinates, default false"));
    }
};


void MIRGetData::usage(const std::string &tool) const {
    eckit::Log::info()
            << "\nPrint a latitude, longitude, data values list."
               "\n"
               "\nUsage: " << tool << " [--diff-atlas=[true|false]] [--diff-ecc=[true|false]] file.grib [file.grib [...]]"
               "\nExamples:"
               "\n  % " << tool << " 1.grib"
               "\n  % " << tool << " --diff-atlas 1.grib 2.grib 3.grib"
            << std::endl;
}


double longitude_diff(double lon1, double lon2) {
    double a = std::abs(lon1 - lon2);
    while (a > 180.) {
        a = std::abs(a - 360.);
    }
    return a;
};


void MIRGetData::execute(const eckit::option::CmdArgs& args) {
    using mir::repres::Iterator;
    using minmax_t = mir::stats::detail::ScalarMinMaxFn<double>;

    auto& log = eckit::Log::info();


    bool ecc = false;
    args.get("diff-ecc", ecc);


    bool atlas = false;
    args.get("diff-atlas", atlas);


    for (size_t i = 0; i < args.count(); ++i) {
        mir::input::GribFileInput grib(args(i));
        const mir::input::MIRInput& input = grib;


        size_t count = 0;
        while (grib.next()) {
            log << "\n'" << args(i) << "' #" << ++count << std::endl;

            mir::data::MIRField field = input.field();
            ASSERT(field.dimensions() == 1);

            mir::repres::RepresentationHandle rep(field.representation());

            if (atlas) {
                eckit::ScopedPtr< Iterator > it(rep->iterator());
                minmax_t stats[2];

                atlas::Grid grid(rep->atlasGrid());

                auto v = field.values(0).begin();
                for (const atlas::Grid::PointLonLat p: grid.lonlat()) {
                    ASSERT(it->next());
                    const Iterator::point_2d_t& P(**it);
                    stats[0](std::abs(P[0] - p.lat()));
                    stats[1](longitude_diff(P[1], p.lon()));
                    ++v;
                }
                ASSERT(v == field.values(0).end());

                log << "\nCompare |MIR - Atlas|:"
                    << "\n\tcount = " << field.values(0).size()
                    << "\n\tmax(|Δlat|) [°] = " << stats[0].max() << "\tindex [1] = " << stats[0].maxIndex()
                    << "\n\tmax(|Δlon|) [°] = " << stats[1].max() << "\tindex [1] = " << stats[1].maxIndex()
                    << std::endl;

                std::vector<size_t> look;
                for (int i : { int(stats[0].maxIndex()), int(stats[1].maxIndex()) }) {
                    for (int j : { -3, -2, -1, 0, 1, 2, 3 }) {
                        auto k = i + j;
                        if (0 < k && k <= int(field.values(0).size())) {
                            look.push_back(size_t(k));
                        }
                    }
                }
                std::sort(look.begin(), look.end());
                look.erase(std::unique(look.begin(), look.end() ), look.end());

                auto old = log.precision(32);
                size_t pos = 0;
                it.reset(rep->iterator());
                for (const atlas::Grid::PointLonLat p: grid.lonlat()) {
                    ASSERT(it->next());
                    if (std::find(look.begin(), look.end(), ++pos) != look.end()) {
                        const Iterator::point_2d_t& P(**it);
                        log << "\n\t" << P[0]    << '\t' << P[1]    << '\t' << *v << "\t(index [1] = " << pos << ", MIR)"
                            << "\n\t" << p.lat() << '\t' << p.lon() << '\t' << *v << "\t(index [1] = " << pos << ", Atlas)";
                        if (pos == look.back()) {
                            break;
                        }
                    }
                }
                log << std::endl;
                log.precision(old);
            }

            if (ecc) {
                eckit::ScopedPtr< Iterator > it(rep->iterator());
                minmax_t stats[2];

                int err = 0;
                grib_iterator* iter = grib_iterator_new(input.gribHandle(), 0, &err);
                if (err != GRIB_SUCCESS) {
                    GRIB_CHECK(err, nullptr);
                }

                auto v = field.values(0).begin();
                long n = 0;
                for (double lat, lon, value; grib_iterator_next(iter, &lat, &lon, &value); ++n) {
                    ASSERT(it->next());
                    const Iterator::point_2d_t& P(**it);
                    stats[0](std::abs(P[0] - lat));
                    stats[1](longitude_diff(P[1], lon));
                    ++v;
                }
                ASSERT(v == field.values(0).end());

                grib_iterator_delete(iter);

                log << "\nCompare |MIR - ecCodes|:"
                    << "\n\tcount = " << field.values(0).size()
                    << "\n\tmax(|Δlat|) [°] = " << stats[0].max() << "\tindex [1] = " << stats[0].maxIndex()
                    << "\n\tmax(|Δlon|) [°] = " << stats[1].max() << "\tindex [1] = " << stats[1].maxIndex()
                    << std::endl;

                std::vector<size_t> look;
                for (int i : { int(stats[0].maxIndex()), int(stats[1].maxIndex()) }) {
                    for (int j : { -3, -2, -1, 0, 1, 2, 3 }) {
                        auto k = i + j;
                        if (0 < k && k <= int(field.values(0).size())) {
                            look.push_back(size_t(k));
                        }
                    }
                }
                std::sort(look.begin(), look.end());
                look.erase(std::unique(look.begin(), look.end() ), look.end());

                auto old = log.precision(32);
                size_t pos = 0;
                it.reset(rep->iterator());

                err = 0;
                iter = grib_iterator_new(input.gribHandle(), 0, &err);
                if (err != GRIB_SUCCESS) {
                    GRIB_CHECK(err, nullptr);
                }

                for (double lat, lon, value; grib_iterator_next(iter, &lat, &lon, &value); ++n) {
                    ASSERT(it->next());
                    if (std::find(look.begin(), look.end(), ++pos) != look.end()) {
                        const Iterator::point_2d_t& P(**it);
                        log << "\n\t" << P[0] << '\t' << P[1] << '\t' << *v << "\t(index [1] = " << pos << ", MIR)"
                            << "\n\t" << lat  << '\t' << lon  << '\t' << *v << "\t(index [1] = " << pos << ", ecCodes)";
                        if (pos == look.back()) {
                            break;
                        }
                    }
                }

                grib_iterator_delete(iter);

                log << std::endl;
                log.precision(old);
            }

            if (!atlas && !ecc) {
                eckit::ScopedPtr< Iterator > it(rep->iterator());
                for (const double& v: field.values(0)) {
                    ASSERT(it->next());
                    const Iterator::point_2d_t& P(**it);
                    log << "\n\t" << P[0] << '\t' << P[1] << '\t' << v;
                }

                log << std::endl;
                ASSERT(!it->next());

            }

        }
    }

}


int main(int argc, char **argv) {
    MIRGetData tool(argc, argv);
    return tool.start();
}

