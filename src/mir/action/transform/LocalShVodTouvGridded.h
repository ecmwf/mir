/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_action_transform_LocalShVodTouvGridded_h
#define mir_action_transform_LocalShVodTouvGridded_h

#include "mir/action/transform/LocalShToGridded.h"


namespace mir {
namespace action {
namespace transform {


class LocalShVodTouvGridded : public LocalShToGridded {
public:

    // -- Exceptions
    // None

    // -- Contructors

    LocalShVodTouvGridded(const param::MIRParametrisation&);

    // -- Destructor

    virtual ~LocalShVodTouvGridded();

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

    void sh2grid(data::MIRField&, atlas::trans::Trans&, const atlas::Grid&) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None

};


}  // namespace transform
}  // namespace action
}  // namespace mir


#endif
