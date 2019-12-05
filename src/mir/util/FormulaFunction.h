/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @author Pedro Maciel
/// @date   April 2016


#ifndef mir_util_FormulaFunction_h
#define mir_util_FormulaFunction_h

#include <string>
#include <vector>

#include "mir/util/Formula.h"


namespace mir {
namespace util {
class Function;
}
}


namespace mir {
namespace util {

class FormulaFunction : public Formula {
public:

    FormulaFunction(const param::MIRParametrisation &parametrisation,
                    const std::string& name,
                    Formula* arg1);

    FormulaFunction(const param::MIRParametrisation &parametrisation,
                    const std::string& name,
                    Formula* arg1,
                    Formula *arg2);

    FormulaFunction(const param::MIRParametrisation &parametrisation,
                    const std::string& name,
                    std::vector<Formula*>& args);

    virtual ~FormulaFunction();

protected: // members

    const Function& function_;
    std::vector<Formula*> args_;

private:

    virtual void print(std::ostream& s) const;
    virtual void execute(context::Context & ctx) const;
    virtual bool sameAs(const Action& other) const;
    virtual const char* name() const;

};


}  // namespace util
}  // namespace mir


#endif
