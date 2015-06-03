





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


#include "eckit/runtime/Tool.h"

#include "atlas/Grid.h"
#include "eckit/io/StdFile.h"
#include "eckit/runtime/Context.h"
#include "mir/data/MIRField.h"
#include "mir/lsm/Mask.h"

#include "mir/input/GribFileInput.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/param/MIRArgs.h"
#include "mir/param/MIRCombinedParametrisation.h"
#include "mir/param/MIRDefaults.h"

using mir::param::option::Option;

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



    std::vector<const Option *> options;
    // options.push_back(new SimpleOption<eckit::PathName>("load", "Load file into shared memory. If file already loaded, does nothing."));
    // options.push_back(new SimpleOption<eckit::PathName>("unload", "Load file into shared memory. If file already loaded, does nothing."));

    mir::param::MIRArgs args(&usage, 2, options);
    args.set("lsm", true); // Force LSM

    mir::input::GribFileInput file(args.args(0));
    eckit::StdFile out(args.args(1), "w");

    while (file.next()) {
        mir::input::MIRInput &input = file;

        const mir::param::MIRParametrisation &defaults = mir::param::MIRDefaults::instance();
        const mir::param::MIRParametrisation &metadata = input.parametrisation();
        mir::param::MIRCombinedParametrisation combined(args, metadata, defaults);

        size_t Ni = 0;
        size_t Nj = 0;

        ASSERT(metadata.get("Ni", Ni));
        ASSERT(metadata.get("Nj", Nj));

        std::auto_ptr<mir::data::MIRField> field(input.field());
        std::auto_ptr<atlas::Grid> grid(field->representation()->atlasGrid());


        mir::lsm::Mask &mask = mir::lsm::Mask::lookupOutput(combined, *grid);

        eckit::Log::info() << "MASK IS => " << mask << std::endl;


        const std::vector<bool> &m = mask.mask();

        fprintf(out, "P5\n%zu %zu 255\n", Ni, Nj);

        for (std::vector<bool>::const_iterator j = m.begin(); j != m.end(); ++j) {
            unsigned char c = (*j) ? 0xff : 0;
            ASSERT(fwrite(&c, 1, 1, out));
        }

    }

}


int main( int argc, char **argv ) {
    MIRMakeLSM tool(argc, argv);
    tool.start();
    return 0;
}

