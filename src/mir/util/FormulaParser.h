// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <vector>

#include "eckit/parser/StreamParser.h"


namespace mir {
namespace param {
class MIRParametrisation;
}
namespace util {
class Formula;
}
}  // namespace mir


namespace mir::util {
class FormulaParser : public eckit::StreamParser {

public:  // methods
    FormulaParser(std::istream&);

    Formula* parse(const param::MIRParametrisation&);

private:  // methods
    Formula* parseAtom(const param::MIRParametrisation&);
    Formula* parseTest(const param::MIRParametrisation&);
    Formula* parsePower(const param::MIRParametrisation&);
    std::vector<Formula*> parseList(const param::MIRParametrisation&);
    Formula* parseFactor(const param::MIRParametrisation&);
    Formula* parseTerm(const param::MIRParametrisation&);
    std::string parseIdent(const param::MIRParametrisation&);
    Formula* parseString(const param::MIRParametrisation&);
    Formula* parseNumber(const param::MIRParametrisation&);
};


}  // namespace mir::util
