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

#include "eckit/option/CmdArgs.h"
#include "eckit/option/FactoryOption.h"
#include "eckit/option/SimpleOption.h"

#include "mir/key/grid/Grid.h"
#include "mir/key/grid/GridPattern.h"
#include "mir/method/Method.h"
#include "mir/param/ConfigurationWrapper.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/repres/proxy/HEALPix.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir::tools {


struct MIRHEALPixMatrix : MIRTool {
    MIRHEALPixMatrix(int argc, char** argv) : MIRTool(argc, argv) {
        using eckit::option::FactoryOption;
        using eckit::option::SimpleOption;

        options_.push_back(new FactoryOption<key::grid::GridPattern>(
            "input-grid", "Interpolate from given grid (following a recognizable regular expression)"));
        options_.push_back(new FactoryOption<key::grid::GridPattern>(
            "grid", "Interpolate to given grid (following a recognizable regular expression)"));
        options_.push_back(
            new FactoryOption<method::MethodFactory>("interpolation", "Grid to grid interpolation method", "linear"));
        options_.push_back(
            new SimpleOption<std::string>("ordering", "HEALPix ordering convention (ring or nested)", "ring"));
        options_.push_back(new SimpleOption<std::string>("output", "Interpolation matrix", "output.mat"));
    }

    int numberOfPositionalArguments() const override { return 1; }

    void usage(const std::string& tool) const override {
        Log::info() << "\n"
                    << "Usage: " << tool
                    << " [--interpolation=...]]"
                       " [--ordering=[ring|nested]]"
                    << std::endl;
    }

    void execute(const eckit::option::CmdArgs& args) override;
};


void MIRHEALPixMatrix::execute(const eckit::option::CmdArgs& args) {
    const param::ConfigurationWrapper param(args);

    auto output   = args.getString("output", "output.mat");
    auto ordering = args.getString("ordering", "ring") == "ring" ? repres::proxy::HEALPix::healpix_ring
                                                                 : repres::proxy::HEALPix::healpix_nested;

    repres::RepresentationHandle in(key::grid::Grid::lookup(args.getString("input-grid")).representation());
    repres::RepresentationHandle out(key::grid::Grid::lookup(args.getString("grid")).representation());

    std::string interpolation = args.getString("interpolation", "linear");
    std::unique_ptr<method::Method> method(method::MethodFactory::build(interpolation, param));

    NOTIMP;  // TODO
}


}  // namespace mir::tools


int main(int argc, char** argv) {
    mir::tools::MIRHEALPixMatrix tool(argc, argv);
    return tool.start();
}
