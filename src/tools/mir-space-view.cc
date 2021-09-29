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

#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir {
namespace tools {


struct MIRSpaceView : MIRTool {
    using MIRTool::MIRTool;

    int numberOfPositionalArguments() const override { return 1; }

    void usage(const std::string& tool) const override {
        Log::info() << "\nUsage: " << tool << " file.grib" << std::endl;
    }

    void execute(const eckit::option::CmdArgs&) override;
};


void MIRSpaceView::execute(const eckit::option::CmdArgs& args) {
    // References:
    // - LRIT/HRIT Global Specification (CGMS 03, Issue 2.6, 12.08.1999)
    // - MSG Ground Segment LRIT/HRIT Mission Specific Implementation, EUMETSAT Document, (EUM/MSG/SPE/057, Issue
    // 6, 21. June 2006).

    auto& log = Log::info();
    log.precision(32);

    ASSERT(args.count() == 1);

    std::unique_ptr<input::MIRInput> input(new input::GribFileInput(args(0)));
    ASSERT(input->next());

    auto field = input->field();
    // auto missingValue  = field.missingValue();
    // const auto& values = field.values(0);

    repres::RepresentationHandle rep(field.representation());
     for (std::unique_ptr<repres::Iterator> it(rep->iterator()); it->next();) {
         Log::info() << *it << std::endl;
     }
}


}  // namespace tools
}  // namespace mir


int main(int argc, char** argv) {
    mir::tools::MIRSpaceView tool(argc, argv);
    return tool.start();
}
