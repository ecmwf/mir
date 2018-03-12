/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_util_function_FunctionParser_h
#define mir_util_function_FunctionParser_h

#include <iosfwd>
#include <vector>
#include "eckit/parser/StreamParser.h"


namespace mir {
namespace util {
namespace function {
class Function;
}
}
}


namespace mir {
namespace util {
namespace function {

class FunctionParser : public eckit::StreamParser {

public:

    FunctionParser(std::istream&);
    Function* parse();

private:

    Function* parseAtom();
    Function* parseTest();
    Function* parsePower();
    std::vector<Function*> parseList();
    Function* parseFactor();
    Function* parseTerm();
    std::string parseIdent();
    Function* parseString();
    Function* parseNumber();

};


}  // namespace function
}  // namespace util
}  // namespace mir


#endif
