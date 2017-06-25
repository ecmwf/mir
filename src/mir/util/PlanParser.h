/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @date Jun 2012

#ifndef mir_PlanParser_h
#define mir_PlanParser_h

#include <vector>

#include "eckit/parser/StreamParser.h"
#include "eckit/types/Types.h"

namespace mir {

namespace param {
class MIRParametrisation;
}

namespace action {
class ActionPlan;
}

namespace util {

//----------------------------------------------------------------------------------------------------------------------

class PlanParser : public eckit::StreamParser {

public: // methods

    PlanParser(std::istream &in);

    void parse(action::ActionPlan &plan,
               const param::MIRParametrisation &parametrisation);

private: // methods

    void parseAction(action::ActionPlan &plan,
                     const param::MIRParametrisation &parametrisation);

    std::string parseToken();
    std::map<std::string, std::vector<std::string> > parseArguments(const std::string& action);
    std::vector<std::string> parseValues();

};

//----------------------------------------------------------------------------------------------------------------------

}
} // namespace eckit

#endif
