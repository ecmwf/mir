// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <vector>

#include "eckit/runtime/Tool.h"


namespace eckit::option {
class CmdArgs;
class Option;
}  // namespace eckit::option


namespace mir::tools {


class MIRTool : public eckit::Tool {
protected:
    using options_t = std::vector<eckit::option::Option*>;

    void run() override;

    virtual void execute(const eckit::option::CmdArgs&) = 0;

    virtual int numberOfPositionalArguments() const { return -1; }

    virtual int minimumPositionalArguments() const { return numberOfPositionalArguments(); }

    virtual void init(const eckit::option::CmdArgs&);

    virtual void finish(const eckit::option::CmdArgs&);

    options_t options_;

public:
    virtual void usage(const std::string& tool) const = 0;

    MIRTool(int argc, char** argv);
};


}  // namespace mir::tools
