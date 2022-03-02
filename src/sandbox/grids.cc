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


#include "eckit/runtime/Tool.h"
#include "eckit/utils/RLE.h"

#include "mir/util/Atlas.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Types.h"


namespace mir {
namespace tools {


struct Grids : eckit::Tool {
    using Tool::Tool;
    void run() override;
    void grid(const atlas::StructuredGrid& /*grid*/);
};


void Grids::grid(const atlas::StructuredGrid& grid) {

    const auto& pl = grid.nx();
    ASSERT(pl.size());

    std::vector<int> points_per_latitudes(pl.size());
    ASSERT(pl.size() == points_per_latitudes.size());

    size_t half = points_per_latitudes.size() / 2;
    ASSERT(half > 0);

    std::vector<int> diff;
    diff.reserve(half);
    eckit::DIFFencode(points_per_latitudes.begin(), points_per_latitudes.begin() + long(half),
                      std::back_inserter(diff));

    std::vector<int> rle;
    eckit::RLEencode2(diff.begin(), diff.end(), std::back_inserter(rle), 1000);

    const atlas::Grid& g = grid;

    Log::info() << "uid " << g.uid() << " rle ";
    eckit::RLEprint(Log::info(), rle.begin(), rle.end());
    Log::info() << std::endl;
}


void Grids::run() {
    for (const std::string& name :
         {"N16",  "N24",  "N32",  "N48",  "N64",  "N80",   "N96",   "N128",  "N160",  "N200",  "N256", "N320",
          "N400", "N512", "N576", "N640", "N800", "N1024", "N1280", "N1600", "N2000", "N4000", "N8000"}) {
        grid(atlas::ReducedGaussianGrid(name));
    }

    for (const std::string& name :
         {"O16",  "O24",  "O32",  "O48",  "O64",  "O80",   "O96",   "O128",  "O160",  "O200",  "O256", "O320",
          "O400", "O512", "O576", "O640", "O800", "O1024", "O1280", "O1600", "O2000", "O4000", "O8000"}) {
        grid(atlas::ReducedGaussianGrid(name));
    }
}


}  // namespace tools
}  // namespace mir


int main(int argc, char** argv) {
    mir::tools::Grids tool(argc, argv);
    return tool.start();
}
