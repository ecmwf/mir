// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"

#include "mir/data/MIRField.h"
#include "mir/input/MIRInput.h"
#include "mir/param/ConfigurationWrapper.h"
#include "mir/repres/Representation.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Domain.h"
#include "mir/util/Log.h"


namespace mir::tools {


struct MIRBoundingBox : MIRTool {
    using MIRTool::MIRTool;

    MIRBoundingBox(int argc, char** argv) : MIRTool(argc, argv) {
        options_.push_back(new eckit::option::SimpleOption<bool>("domain", ""));
    }

    void usage(const std::string& tool) const override {
        Log::info() << "\n"
                       "Usage: "
                    << tool << "[FILE [FILE [FILE]]]" << std::endl;
    }

    void execute(const eckit::option::CmdArgs& args) override {
        const param::ConfigurationWrapper args_wrap(args);

        auto domain = false;
        args_wrap.get("domain", domain);

        for (const auto& arg : args) {
            std::unique_ptr<input::MIRInput> input(input::MIRInputFactory::build(arg, args_wrap));

            while (input->next()) {
                auto field = input->field();
                repres::RepresentationHandle representation(field.representation());

                if (domain) {
                    Log::info() << representation->domain() << std::endl;
                }
                else {
                    Log::info() << representation->boundingBox() << std::endl;
                }
            }
        }
    }
};


}  // namespace mir::tools


int main(int argc, char** argv) {
    mir::tools::MIRBoundingBox tool(argc, argv);
    return tool.start();
}
