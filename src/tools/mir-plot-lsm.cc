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


#include <memory>

#include "eckit/io/StdFile.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/FactoryOption.h"
#include "eckit/option/SimpleOption.h"
#include "eckit/runtime/Tool.h"

#include "mir/key/grid/Grid.h"
#include "mir/lsm/LSMSelection.h"
#include "mir/lsm/Mask.h"
#include "mir/lsm/NamedLSM.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/ConfigurationWrapper.h"
#include "mir/param/DefaultParametrisation.h"
#include "mir/repres/latlon/RegularLL.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/Log.h"


using namespace mir;


struct MIRPlotLSM : tools::MIRTool {
    MIRPlotLSM(int argc, char** argv) : MIRTool(argc, argv) {
        using eckit::option::FactoryOption;
        using eckit::option::SimpleOption;

        options_.push_back(new SimpleOption<std::string>("grid", "we/sn grid increments (default 1/1)"));

        options_.push_back(
            new FactoryOption<lsm::LSMSelection>("lsm-selection", "LSM selection method for both input and output"));
        options_.push_back(new FactoryOption<lsm::NamedMaskFactory>(
            "lsm-named", "If --lsm-selection=named, LSM name to use for both input and output"));
        options_.push_back(new SimpleOption<eckit::PathName>(
            "lsm-file", "If --lsm-selection=file, LSM GRIB file path to use for both input and output"));
    }

    int numberOfPositionalArguments() const override { return 1; }

    void usage(const std::string& tool) const override {
        Log::info() << "\n"
                    << "Usage: " << tool << " --grid=1/1 output.pbm" << std::endl;
    }

    void execute(const eckit::option::CmdArgs&) override;
};


void MIRPlotLSM::execute(const eckit::option::CmdArgs& args) {
    const_cast<eckit::option::CmdArgs&>(args).set("lsm", true);  // Force LSM


    // Create a regular_ll grid
    std::string grid = "1/1";
    args.get("grid", grid);

    repres::RepresentationHandle repres(key::grid::Grid::lookup(grid).representation());
    ASSERT(repres != nullptr);

    auto repres_ll = dynamic_cast<const repres::latlon::RegularLL*>(static_cast<const repres::Representation*>(repres));
    ASSERT(repres_ll != nullptr);


    // Build mask
    static const param::DefaultParametrisation defaults;
    static const param::ConfigurationWrapper args_wrap(args);
    param::CombinedParametrisation parametrisation(args_wrap, defaults, defaults);

    const auto& mask = lsm::Mask::lookupOutput(parametrisation, *repres);
    Log::info() << "MASK IS => " << mask << std::endl;


    // Write mask
    eckit::StdFile out(args(0), "w");
    fprintf(out, "P5\n%zu %zu 255\n", repres_ll->Ni(), repres_ll->Nj());

    const auto& m = mask.mask();
    for (auto j = m.begin(); j != m.end(); ++j) {
        unsigned char c = (*j) ? 0xff : 0;
        ASSERT(fwrite(&c, 1, 1, out));
    }

    out.close();
}


int main(int argc, char** argv) {
    MIRPlotLSM tool(argc, argv);
    return tool.start();
}
