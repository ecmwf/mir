/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_data_dimension_Space3DCartesian_h
#define mir_data_dimension_Space3DCartesian_h

#include "mir/data/space/SpaceLinear.h"


namespace mir {
namespace data {
namespace space {


class Space3DCartesian : public SpaceLinear {

    // -- Overridden methods

    size_t dimensions() const;
};


}  // namespace space
}  // namespace data
}  // namespace mir


#endif
