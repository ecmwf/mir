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


#ifndef mir_style_IntermediateNamedGrid_h
#define mir_style_IntermediateNamedGrid_h

#include "mir/style/IntermediateGrid.h"


namespace mir {
namespace style {


class IntermediateNamedGrid: public IntermediateGrid {
public:

    // -- Exceptions
    // None

    // -- Contructors

    IntermediateNamedGrid(const param::MIRParametrisation&);

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    size_t gaussianNumber() const { return 0; }
    std::string gaussianGridType() const { return ""; }

    // -- Overridden methods

    void print(std::ostream& out) const;

    // -- Class members
    // None

    // -- Class methods
    // None

protected:

    // -- Members
    // None

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:

    // -- Members
    // None

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


}  // namespace style
}  // namespace mir


#endif
