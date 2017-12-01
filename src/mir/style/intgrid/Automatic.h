/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */


#ifndef mir_style_intgrid_Automatic_h
#define mir_style_intgrid_Automatic_h

#include "mir/style/Intgrid.h"


namespace mir {
namespace style {
namespace intgrid {


class Automatic : public Intgrid {
public:

    // -- Exceptions
    // None

    // -- Contructors

    Automatic(const param::MIRParametrisation& parametrisation, long targetGaussianN);

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

    // -- Overridden methods

    std::string gridname() const;
    void print(std::ostream& out) const;

    // -- Class members
    // None

    // -- Class methods
    // None

private:

    // -- Members

    std::string gridname_;

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


}  // namespace intgrid
}  // namespace style
}  // namespace mir


#endif

