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

#ifndef mir_FormulaString_H
#define mir_FormulaString_H

#include <string>
#include "mir/util/Formula.h"

namespace mir {
namespace util {


//----------------------------------------------------------------------------------------------------------------------

class FormulaString : public Formula {
public:

    FormulaString(const std::string& value);
    virtual ~FormulaString();

private:

    virtual void print(std::ostream& s) const;

private: // members

    std::string value_;

};

//----------------------------------------------------------------------------------------------------------------------

}
} // namespace eckit

#endif
