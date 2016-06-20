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

#ifndef fdb5_SchemaParser_h
#define fdb5_SchemaParser_h

#include "eckit/parser/StreamParser.h"
#include "eckit/types/Types.h"

namespace mir {
namespace util {


//----------------------------------------------------------------------------------------------------------------------

class FormulaParser : public eckit::StreamParser {

public: // methods

    FormulaParser(std::istream &in);

    void parse();

private: // methods

    void parseAtom();
    void parseTest();
    void parsePower();
    void parseList();
    void parseFactor();
    void parseTerm();
    void parseIdent();
    void parseString();
    void parseNumber();

    // std::string parseIdent(bool emptyOK = false);

    // Rule *parseRule(const Schema &owner);

    // Predicate *parsePredicate(std::map<std::string, std::string> &types);
    // void parseTypes(std::map<std::string, std::string> &);

};

//----------------------------------------------------------------------------------------------------------------------

}
} // namespace eckit

#endif
