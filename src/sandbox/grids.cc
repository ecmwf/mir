/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#include "eckit/runtime/Tool.h"
#include "eckit/utils/MD5.h"
#include "eckit/utils/RLE.h"
#include "atlas/grid.h"


class Grids : public eckit::Tool {

    virtual void run();

    void usage(const std::string &tool);
    void grid(const atlas::grid::StructuredGrid&);

  public:
    Grids(int argc, char **argv) :
        eckit::Tool(argc, argv) {
    }

};


void Grids::grid(const atlas::grid::StructuredGrid& grid) {

    const std::vector<long>& pl = grid.nx();
    ASSERT(pl.size());

    std::vector<int> points_per_latitudes(pl.size());
    ASSERT(pl.size()==points_per_latitudes.size());

    size_t half = points_per_latitudes.size() / 2;
    ASSERT(half>0);

    std::vector<int> diff;
    diff.reserve(half);
    eckit::DIFFencode(points_per_latitudes.begin(), points_per_latitudes.begin() + half, std::back_inserter(diff));

    std::vector<int> rle;
    eckit::RLEencode2(diff.begin(), diff.end(), std::back_inserter(rle), 1000);

    const atlas::Grid& g = grid;

    eckit::Log::info() << "uid " << g.uid() << " rle ";
    eckit::RLEprint(eckit::Log::info(), rle.begin(), rle.end());
    eckit::Log::info() << std::endl;
}


void Grids::run() {
    using atlas::grid::ReducedGaussianGrid;

    grid(ReducedGaussianGrid("N16"));
    grid(ReducedGaussianGrid("N24"));
    grid(ReducedGaussianGrid("N32"));
    grid(ReducedGaussianGrid("N48"));
    grid(ReducedGaussianGrid("N64"));
    grid(ReducedGaussianGrid("N80"));
    grid(ReducedGaussianGrid("N96"));
    grid(ReducedGaussianGrid("N128"));
    grid(ReducedGaussianGrid("N160"));
    grid(ReducedGaussianGrid("N200"));
    grid(ReducedGaussianGrid("N256"));
    grid(ReducedGaussianGrid("N320"));
    grid(ReducedGaussianGrid("N400"));
    grid(ReducedGaussianGrid("N512"));
    grid(ReducedGaussianGrid("N576"));
    grid(ReducedGaussianGrid("N640"));
    grid(ReducedGaussianGrid("N800"));
    grid(ReducedGaussianGrid("N1024"));
    grid(ReducedGaussianGrid("N1280"));
    grid(ReducedGaussianGrid("N1600"));
    grid(ReducedGaussianGrid("N2000"));
    grid(ReducedGaussianGrid("N4000"));
    grid(ReducedGaussianGrid("N8000"));

    grid(ReducedGaussianGrid("O16"));
    grid(ReducedGaussianGrid("O24"));
    grid(ReducedGaussianGrid("O32"));
    grid(ReducedGaussianGrid("O48"));
    grid(ReducedGaussianGrid("O64"));
    grid(ReducedGaussianGrid("O80"));
    grid(ReducedGaussianGrid("O96"));
    grid(ReducedGaussianGrid("O128"));
    grid(ReducedGaussianGrid("O160"));
    grid(ReducedGaussianGrid("O200"));
    grid(ReducedGaussianGrid("O256"));
    grid(ReducedGaussianGrid("O320"));
    grid(ReducedGaussianGrid("O400"));
    grid(ReducedGaussianGrid("O512"));
    grid(ReducedGaussianGrid("O576"));
    grid(ReducedGaussianGrid("O640"));
    grid(ReducedGaussianGrid("O800"));
    grid(ReducedGaussianGrid("O1024"));
    grid(ReducedGaussianGrid("O1280"));
    grid(ReducedGaussianGrid("O1600"));
    grid(ReducedGaussianGrid("O2000"));
    grid(ReducedGaussianGrid("O4000"));
    grid(ReducedGaussianGrid("O8000"));
}


int main( int argc, char **argv ) {
    Grids tool(argc, argv);
#if (ECKIT_MAJOR_VERSION == 0) && (ECKIT_MINOR_VERSION <= 10)
    tool.start();
    return 0;
#else
    return tool.start();
#endif
}

