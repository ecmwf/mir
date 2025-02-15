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

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/input/MIRInput.h"
#include "mir/output/MIROutput.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/ConfigurationWrapper.h"
#include "mir/param/DefaultParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/stats/Statistics.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/Exceptions.h"
#include "mir/util/GridBox.h"
#include "mir/util/Log.h"
#include "mir/util/MIRStatistics.h"


namespace mir::tools {


struct MIRGridBoxArea : MIRTool {
    MIRGridBoxArea(int argc, char** argv) : MIRTool(argc, argv) {}

    int numberOfPositionalArguments() const override { return 1; }

    void usage(const std::string& tool) const override {
        Log::info() << "\n"
                       "Calculate grid-box areas on the first field in file."
                    << std::endl;
    }

    void execute(const eckit::option::CmdArgs& args) override {
        const param::ConfigurationWrapper args_wrap(args);
        const param::DefaultParametrisation defaults;

        for (const auto& arg : args) {
            // read in first field to calculate grid-box areas
            std::unique_ptr<input::MIRInput> input(input::MIRInputFactory::build(arg, args_wrap));
            ASSERT(input->next());

            std::unique_ptr<param::MIRParametrisation> param(
                new param::CombinedParametrisation(args_wrap, input->parametrisation(), defaults));

            util::MIRStatistics statistics;
            context::Context ctx(*input, statistics);

            auto& field = ctx.field();
            ASSERT(field.dimensions() == 1);

            auto& area = field.direct(0);

            size_t i = 0;
            for (const auto& box : repres::RepresentationHandle(field.representation())->gridBoxes(false)) {
                area[i++] = box.area();
            }

            {
                std::unique_ptr<stats::Statistics> stats(stats::StatisticsFactory::build("scalar", *param));
                stats->execute(field);
                Log::info() << *stats << std::endl;
            }

            std::unique_ptr<output::MIROutput> out(output::MIROutputFactory::build(arg + ".area", *param));
            out->save(*param, ctx);
        }
    }
};


}  // namespace mir::tools


int main(int argc, char** argv) {
    mir::tools::MIRGridBoxArea tool(argc, argv);
    return tool.start();
}
