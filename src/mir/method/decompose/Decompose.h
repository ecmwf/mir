/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_method_decompose_Decompose_h
#define mir_method_decompose_Decompose_h

#include "mir/method/WeightMatrix.h"


namespace mir {
namespace method {
namespace decompose {


class Decompose {
public:

    // -- Exceptions
    // None

    // -- Constructors

    Decompose();

    // -- Destructor

    virtual ~Decompose() {}

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    virtual void decompose(WeightMatrix::Vector&) = 0;

    virtual void recompose(WeightMatrix::Vector&) const = 0;

    WeightMatrix::Matrix& getMatrix() {
        return xy_;
    }

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:

    // -- Members

    WeightMatrix::Matrix xy_;

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


}  // namespace decompose
}  // namespace method
}  // namespace mir


#endif

