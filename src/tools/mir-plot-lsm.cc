/*
 * (C) Copyright 1996- ECMWF.
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


#include <memory>

#include "eckit/io/StdFile.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/VectorOption.h"
#include "eckit/runtime/Tool.h"

#include "mir/config/MIRConfiguration.h"
#include "mir/lsm/Mask.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/ConfigurationWrapper.h"
#include "mir/tools/MIRTool.h"


class MIRPlotLSM : public mir::tools::MIRTool {

    // -- Overridden methods

    void execute(const eckit::option::CmdArgs&);

    void usage(const std::string& tool) const;

    int minimumPositionalArguments() const { return 1; }

public:
    // -- Contructors

    MIRPlotLSM(int argc, char** argv) : mir::tools::MIRTool(argc, argv) {
        using namespace eckit::option;

        options_.push_back(new VectorOption<double>("grid", "Default 1/1", 2));
        options_.push_back(new VectorOption<long>("ninj", "Default 360/181", 2));

        // options_.push_back(new SimpleOption<eckit::PathName>("load", "Load file into shared memory. If file already loaded, does nothing."));
        // options_.push_back(new SimpleOption<eckit::PathName>("unload", "Load file into shared memory. If file already loaded, does nothing."));
    }
};


void MIRPlotLSM::usage(const std::string& tool) const {
    eckit::Log::info() << "\n"
                       << "Usage: " << tool << " file.grib file.lsm" << std::endl;
}


void MIRPlotLSM::execute(const eckit::option::CmdArgs& args) {

    const_cast<eckit::option::CmdArgs&>(args).set("lsm", true);  // Force LSM

    size_t Ni = 360;
    size_t Nj = 181;

    std::vector<double> v;
    if (args.get("grid", v)) {
        Ni = size_t(360.0 / v[0] + 0.5);
        Nj = size_t(180.0 / v[1] + 0.5) + 1;
    }

    std::vector<long> n;
    if (args.get("ninj", n)) {
        Ni = size_t(n[0]);
        Nj = size_t(n[1]);
    }

    eckit::Log::info() << "Ni=" << Ni << ", Nj=" << Nj << std::endl;

    eckit::StdFile out(args(0), "w");

    // Wrap the arguments, so that they behave as a MIRParameter
    mir::param::ConfigurationWrapper wrap(args);
    std::unique_ptr<const mir::param::MIRParametrisation> defaults(
        mir::config::MIRConfiguration::instance().defaults());

    mir::param::CombinedParametrisation parametrisation(wrap, *defaults, *defaults);

    std::string gridname("L" + std::to_string(Ni) + "x" + std::to_string(Nj));
    atlas::grid::RegularLonLatGrid grid(gridname);


    mir::lsm::Mask& mask = mir::lsm::Mask::lookupOutput(parametrisation, grid);

    eckit::Log::info() << "MASK IS => " << mask << std::endl;

    const std::vector<bool>& m = mask.mask();

    fprintf(out, "P5\n%zu %zu 255\n", Ni, Nj);

    for (std::vector<bool>::const_iterator j = m.begin(); j != m.end(); ++j) {
        unsigned char c = (*j) ? 0xff : 0;
        ASSERT(fwrite(&c, 1, 1, out));
    }
}


int main(int argc, char** argv) {
    MIRPlotLSM tool(argc, argv);
    return tool.start();
}
