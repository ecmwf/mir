/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#include "eckit/runtime/Tool.h"
#include "mir/util/FormulaParser.h"


class MIRFormula : public eckit::Tool {

    virtual void run();


  public:
    MIRFormula(int argc, char **argv) :
        eckit::Tool(argc, argv) {
    }

};


void MIRFormula::run() {
    std::istringstream in("foo(f, 42) + bar() ^ 2");
    mir::util::FormulaParser p(in);

    p.parse();
}


int main( int argc, char **argv ) {
    MIRFormula tool(argc, argv);
    return tool.start();
}


