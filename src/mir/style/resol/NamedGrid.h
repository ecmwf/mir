/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

/// @date May 2017


#ifndef mir_style_resol_NamedGrid_h
#define mir_style_resol_NamedGrid_h

#include "mir/style/Resol.h"


namespace mir {
namespace style {
namespace resol {


class NamedGrid : public Resol {
public:

    // -- Exceptions
    // None

    // -- Contructors

    NamedGrid(const std::string& gridname, const param::MIRParametrisation& parametrisation);

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

    // -- Overridden methods

    void prepare(action::ActionPlan&) const;
    bool resultIsSpectral() const;
    void print(std::ostream& out) const;

    // -- Class members
    // None

    // -- Class methods
    // None

private:

    // -- Members

    const std::string gridname_;

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None

};


}  // namespace resol
}  // namespace style
}  // namespace mir


#endif

