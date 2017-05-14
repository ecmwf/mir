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


#ifndef mir_action_transform_mapping_TMapping_h
#define mir_action_transform_mapping_TMapping_h

#include <cmath>
#include <string>
#include "eckit/exception/Exceptions.h"
#include "mir/action/transform/mapping/Mapping.h"


namespace mir {
namespace action {
namespace transform {
namespace mapping {


template< int ORDER >
class TMapping : public Mapping {
public:
    // -- Exceptions
    // None

    // -- Contructors
    TMapping() {
        ASSERT(ORDER);
    }

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

    // -- Overridden methods
    size_t getTruncationFromPointsPerLatitude(const size_t& N) const {
        ASSERT(N);
    
        size_t T = size_t(ceil( 4. / double(ORDER + 1) * N) - 1);
        ASSERT(T);
    
        return T;
    }

    size_t getPointsPerLatitudeFromTruncation(const size_t& T) const {
        ASSERT(T);

        size_t N = size_t(double(T + 1) * double(ORDER + 1) / 4.);
        ASSERT(N);

        return N;
    }

    void print(std::ostream& out) const {
        out << "TMapping<ORDER=" << ORDER << ">[]";
    }

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


}  // namespace mapping
}  // namespace transform
}  // namespace action
}  // namespace mir


#endif

