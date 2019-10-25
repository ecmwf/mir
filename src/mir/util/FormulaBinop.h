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


#ifndef mir_util_FormulaBinop_h
#define mir_util_FormulaBinop_h

#include <string>
#include <vector>

#include "mir/util/FormulaFunction.h"


namespace mir {
namespace util {


class FormulaBinop : public FormulaFunction {
public:

    FormulaBinop(const param::MIRParametrisation &parametrisation,
        const std::string& name, Formula* arg1, Formula *arg2);

    virtual ~FormulaBinop();

private:

    virtual void print(std::ostream& s) const;

};


}  // namespace util
}  // namespace mir


#endif
