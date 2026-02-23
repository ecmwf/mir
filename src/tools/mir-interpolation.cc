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
#include <string>

#include "eckit/log/JSON.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/FactoryOption.h"
#include "eckit/option/SimpleOption.h"

#include "mir/input/MIRInput.h"
#include "mir/method/Method.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/ConfigurationWrapper.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir::tools {


struct MIRInterpolation : MIRTool {
    MIRInterpolation(int argc, char** argv) : MIRTool(argc, argv) {
        options_.push_back(new eckit::option::FactoryOption<method::MethodFactory>(
            "interpolation", "Grid to grid interpolation method"));
        options_.push_back(new eckit::option::SimpleOption<size_t>("nclosest", ""));
        options_.push_back(new eckit::option::SimpleOption<bool>("lookup-known-methods", ""));
    }

    int minimumPositionalArguments() const override { return 0; }

    void usage(const std::string& tool) const override {
        Log::info() << "\n"
                    << "Usage: " << tool << " ..." << std::endl;
    }

    void execute(const eckit::option::CmdArgs& args) override;
};


void MIRInterpolation::execute(const eckit::option::CmdArgs& args) {
    static const param::ConfigurationWrapper user(args);

    bool lookupKnownMethods = true;
    args.get("lookup-known-methods", lookupKnownMethods);

    auto make_method = [](const param::MIRParametrisation& param) {
        std::string interpolation;
        param.get("interpolation", interpolation);

        std::unique_ptr<method::Method> method{method::MethodFactory::build(interpolation, param)};
        ASSERT(method);

        return method;
    };

    if (args.count() == 0) {
        param::CombinedParametrisation param(user);

        eckit::JSON j(Log::info());
        make_method(param)->json(j, lookupKnownMethods);
        Log::info() << std::endl;

        return;
    }

    for (const auto& arg : args) {
        std::unique_ptr<input::MIRInput> input(input::MIRInputFactory::build(arg, user));
        ASSERT(input);

        while (input->next()) {
            param::CombinedParametrisation param(user, input->parametrisation());

            eckit::JSON j(Log::info());
            make_method(param)->json(j, lookupKnownMethods);
            Log::info() << std::endl;
        }
    }
}


}  // namespace mir::tools


int main(int argc, char** argv) {
    mir::tools::MIRInterpolation tool(argc, argv);
    return tool.start();
}
