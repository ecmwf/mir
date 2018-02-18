/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_data_dimension_DimensionLinear_h
#define mir_data_dimension_DimensionLinear_h

#include "mir/data/Dimension.h"


namespace mir {
namespace data {
namespace dimension {


class DimensionLinear : public Dimension {
public:

    // -- Exceptions
    // None

    // -- Constructors

    DimensionLinear();

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

    // -- Overridden methods

    void linearise(const Dimension::Matrix&, Dimension::Matrix&, double missingValue) const;

    void unlinearise(const Dimension::Matrix&, Dimension::Matrix&, double missingValue) const;

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


}  // namespace dimension
}  // namespace data
}  // namespace mir


#endif

