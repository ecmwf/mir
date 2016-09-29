/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_method_decompose_DecomposeScalar_h
#define mir_method_decompose_DecomposeScalar_h

#include "mir/method/decompose/Decompose.h"


namespace mir {
namespace method {
namespace decompose {


class DecomposeScalar : public Decompose {
public:

    // -- Exceptions
    // None

    // -- Constructors

    DecomposeScalar();

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

    // -- Overridden methods

    void decompose(WeightMatrix::Vector&);

    void recompose(WeightMatrix::Vector&) const;

    // -- Class members
    // None

    // -- Class methods
    // None

private:

    // -- Types
    // None

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

