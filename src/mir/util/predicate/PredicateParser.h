/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_util_predicate_PredicateParser_h
#define mir_util_predicate_PredicateParser_h

#include <vector>
#include "eckit/parser/StreamParser.h"
#include "eckit/types/Types.h"


namespace mir {
namespace util {
namespace predicate {
class Predicate;
}
}
}


namespace mir {
namespace util {
namespace predicate {

class PredicateParser : public eckit::StreamParser {

public:

    PredicateParser(std::istream&);
    Predicate* parse();

private:

    Predicate* parseAtom();
    Predicate* parseTest();
    Predicate* parsePower();
    std::vector<Predicate*> parseList();
    Predicate* parseFactor();
    Predicate* parseTerm();
    std::string parseIdent();
    Predicate* parseString();
    Predicate* parseNumber();

};


}  // namespace predicate
}  // namespace util
}  // namespace mir


#endif
