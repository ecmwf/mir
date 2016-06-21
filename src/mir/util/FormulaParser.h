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

#ifndef mir_FormulaParser_h
#define mir_FormulaParser_h

#include <vector>

#include "eckit/parser/StreamParser.h"
#include "eckit/types/Types.h"

namespace mir {
namespace util {

class Formula;
//----------------------------------------------------------------------------------------------------------------------

class FormulaParser : public eckit::StreamParser {

public: // methods

    FormulaParser(std::istream &in);

    Formula* parse();

private: // methods

    Formula* parseAtom();
    Formula* parseTest();
    Formula* parsePower();
    std::vector<Formula*> parseList();
    Formula* parseFactor();
    Formula* parseTerm();
    std::string parseIdent();
    Formula* parseString();
    Formula* parseNumber();

};

//----------------------------------------------------------------------------------------------------------------------

}
} // namespace eckit

#endif
