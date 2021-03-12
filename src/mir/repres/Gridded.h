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


#ifndef mir_repres_Gridded_h
#define mir_repres_Gridded_h

#include "mir/repres/Representation.h"
#include "mir/util/BoundingBox.h"


namespace mir {
namespace repres {


class Gridded : public Representation {
public:
    // -- Exceptions
    // None

    // -- Constructors

    Gridded(const param::MIRParametrisation&);
    Gridded(const util::BoundingBox&);

    // -- Destructor

    ~Gridded() override;

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
    // -- Constructors

    Gridded();

    // -- Members

    util::BoundingBox bbox_;

    // -- Methods
    // None

    // -- Overridden methods

    util::Domain domain() const override;
    const util::BoundingBox& boundingBox() const override;
    bool getLongestElementDiagonal(double&) const override;
    void estimate(api::MIREstimation&) const override;

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


}  // namespace repres
}  // namespace mir


#endif
