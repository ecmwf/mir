/*
* (C) Copyright 1996-2015 ECMWF.
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

#include "eckit/io/StdFile.h"
#include "eckit/runtime/Tool.h"
#include "eckit/memory/ScopedPtr.h"

#include "atlas/grids/LonLatGrid.h"

#include "mir/lsm/Mask.h"
#include "mir/param/MIRArgs.h"
#include "mir/param/MIRCombinedParametrisation.h"
#include "mir/param/MIRDefaults.h"
#include "mir/param/option/VectorOption.h"


using mir::param::option::Option;
using mir::param::option::VectorOption;

class MIRMakeLSM : public eckit::Tool {

    virtual void run();

    static void usage(const std::string &tool);

  public:
    MIRMakeLSM(int argc, char **argv) :
        eckit::Tool(argc, argv) {
    }

};

void MIRMakeLSM::usage(const std::string &tool) {

    eckit::Log::info()
            << std::endl << "Usage: " << tool << " file.grib file.lsm" << std::endl
            ;

    ::exit(1);
}

void MIRMakeLSM::run() {


    std::vector<Option *> options;
    options.push_back(new VectorOption<double>("grid", "Default 1/1", 2));
    options.push_back(new VectorOption<long>("ninj", "Default 360/181", 2));

    // options.push_back(new SimpleOption<eckit::PathName>("load", "Load file into shared memory. If file already loaded, does nothing."));
    // options.push_back(new SimpleOption<eckit::PathName>("unload", "Load file into shared memory. If file already loaded, does nothing."));

    mir::param::MIRArgs args(&usage, 1, options);
    args.set("lsm", true); // Force LSM

    size_t Ni = 360;
    size_t Nj = 181;

    std::vector<double> v;
    if (args.get("grid", v)) {
        Ni = size_t(360.0 / v[0] + 0.5);
        Nj = size_t(180.0 / v[1] + 0.5) + 1;
    }

    std::vector<long> n;
    if (args.get("ninj", n)) {
        Ni = n[0];
        Nj = n[1];
    }

    eckit::Log::info() << "Ni=" << Ni << ", Nj=" << Nj << std::endl;

    eckit::StdFile out(args.args(0), "w");



    const mir::param::MIRParametrisation &defaults = mir::param::MIRDefaults::instance();
    mir::param::MIRCombinedParametrisation combined(args, defaults, defaults);

    eckit::ScopedPtr<atlas::Grid> grid(new atlas::grids::LonLatGrid(Ni,Nj,
                                    atlas::grids::LonLatGrid::INCLUDES_POLES));


    mir::lsm::Mask &mask = mir::lsm::Mask::lookupOutput(combined, *grid);

    eckit::Log::info() << "MASK IS => " << mask << std::endl;

    const std::vector<bool> &m = mask.mask();

    fprintf(out, "P5\n%zu %zu 255\n", Ni, Nj);

    for (std::vector<bool>::const_iterator j = m.begin(); j != m.end(); ++j) {
        unsigned char c = (*j) ? 0xff : 0;
        ASSERT(fwrite(&c, 1, 1, out));
    }

}


int main( int argc, char **argv ) {
    MIRMakeLSM tool(argc, argv);
#if (ECKIT_MAJOR_VERSION == 0) && (ECKIT_MINOR_VERSION <= 10)
    return 0;
#else
    return tool.start();
#endif
}


