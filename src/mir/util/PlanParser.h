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
