/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_data_dimension_Dimension2DVectorZonalMeridional_h
#define mir_data_dimension_Dimension2DVectorZonalMeridional_h

#include "mir/data/dimension/DimensionLinear.h"


namespace mir {
namespace data {
namespace dimension {


class Dimension2DVectorZonalMeridional : public DimensionLinear {

    // -- Overridden methods

    size_t dimensions() const;

};


}  // namespace dimension
}  // namespace data
}  // namespace mir


#endif

