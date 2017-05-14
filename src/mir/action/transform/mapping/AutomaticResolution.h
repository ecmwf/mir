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


#ifndef mir_action_transform_mapping_AutomaticResolution_h
#define mir_action_transform_mapping_AutomaticResolution_h

#include "mir/action/transform/mapping/Resol.h"


namespace mir {
namespace action {
namespace transform {
namespace mapping {


class AutomaticResolution : public Resol {
public:

    // -- Exceptions
    // None

    // -- Contructors

    AutomaticResolution(const param::MIRParametrisation&);

    // -- Destructor

    virtual ~AutomaticResolution();

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

    // -- Overridden methods
    // None

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

    size_t getTruncation() const;
    size_t getPointsPerLatitude() const;
    void print(std::ostream&) const;

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
