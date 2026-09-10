// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <map>
#include <vector>

#include "eckit/parser/StreamParser.h"


namespace mir {
namespace param {
class MIRParametrisation;
}
namespace action {
class ActionPlan;
}
}  // namespace mir


namespace mir::util {


class PlanParser : public eckit::StreamParser {

public:  // methods
    PlanParser(std::istream&);

    void parse(action::ActionPlan& p, const param::MIRParametrisation&);

private:  // methods
    void parseAction(action::ActionPlan&, const param::MIRParametrisation&);

    std::string parseToken();
    std::map<std::string, std::vector<std::string> > parseArguments(const std::string& action);
    std::vector<std::string> parseValues();
};


}  // namespace mir::util
