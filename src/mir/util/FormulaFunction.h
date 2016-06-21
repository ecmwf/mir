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

#ifndef mir_FormulaFunction_H
#define mir_FormulaFunction_H

#include <string>
#include <vector>

#include "mir/util/Formula.h"

namespace mir {
namespace util {


//----------------------------------------------------------------------------------------------------------------------

class FormulaFunction : public Formula {
public:

    FormulaFunction(const std::string& name, Formula* arg1);
    FormulaFunction(const std::string& name, Formula* arg1, Formula *arg2);

    FormulaFunction(const std::string& name, std::vector<Formula*>& args);
    virtual ~FormulaFunction();

protected: // members

    std::string name_;
    std::vector<Formula*> args_;

private:

    virtual void print(std::ostream& s) const;


};

//----------------------------------------------------------------------------------------------------------------------

}
} // namespace eckit

#endif
